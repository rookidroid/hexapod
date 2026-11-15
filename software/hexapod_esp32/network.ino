/**

  Networking -- WiFi (station with AP fallback), OTA, and the UDP command endpoint

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

*/

#include <ArduinoOTA.h>
#include <AsyncUDP.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>

#include "hexapod.h"

// UDP socket for receiving commands
AsyncUDP udp_socket;

// Access point credentials (defined in config.h)
static const char *ssid = APSSID;
static const char *password = APPSK;

// Pose packets whose seq_num falls at most this far behind the newest one are
// stale (UDP reordered them) and dropped. A bigger step back means the client
// restarted its counter, so the packet is accepted.
static const int32_t POSE_REORDER_WINDOW = 64;

// Newest pose seq_num seen in the current streaming session (UDP task only)
static uint32_t last_pose_seq = 0;
static bool pose_seq_valid = false;

/**
   @brief Read the network saved from the web page's WiFi tab.
   @return true if an SSID is saved
*/
bool loadWifiCredentials(String &sta_ssid, String &sta_pass)
{
  Preferences prefs;
  if (!prefs.begin(WIFI_NVS_NAMESPACE, true))
  {
    // The namespace does not exist until the first save
    sta_ssid = "";
    sta_pass = "";
    return false;
  }
  sta_ssid = prefs.getString(WIFI_NVS_KEY_SSID, "");
  sta_pass = prefs.getString(WIFI_NVS_KEY_PASS, "");
  prefs.end();
  return sta_ssid.length() > 0;
}

/**
   @brief Save the network to join at the next boot.
   @return true on success
*/
bool saveWifiCredentials(const String &sta_ssid, const String &sta_pass)
{
  Preferences prefs;
  bool ok = prefs.begin(WIFI_NVS_NAMESPACE, false);
  // putString() returns 0 on failure. An open network stores no password.
  ok = ok && prefs.putString(WIFI_NVS_KEY_SSID, sta_ssid) > 0;
  if (sta_pass.length() == 0)
  {
    prefs.remove(WIFI_NVS_KEY_PASS); // Fails harmlessly if there was none
  }
  else
  {
    ok = ok && prefs.putString(WIFI_NVS_KEY_PASS, sta_pass) > 0;
  }
  prefs.end();

  Serial.println(ok ? "WiFi network saved" : "ERROR: Failed to save WiFi network");
  return ok;
}

/**
   @brief Forget the saved network, so the robot starts its own AP.
   @return true on success
*/
bool clearWifiCredentials()
{
  Preferences prefs;
  bool ok = prefs.begin(WIFI_NVS_NAMESPACE, false);
  ok = ok && prefs.clear();
  prefs.end();

  Serial.println(ok ? "WiFi network forgotten" : "ERROR: Failed to forget WiFi network");
  return ok;
}

/**
   @brief The robot's own address: on the joined network, or on its AP.
*/
IPAddress wifiIp()
{
  return wifi_sta_mode ? WiFi.localIP() : WiFi.softAPIP();
}

/**
   @brief A client talked to the robot (UDP packet or page load): stand up if
   that has not happened yet.

   On its own AP the robot stands up when a device joins. On a shared network
   devices join the router instead, so the first contact is the cue.
*/
void noteClientSeen()
{
  if (!boot_sequence_executed)
  {
    trigger_boot_sequence = true; // Flag checked in main loop
  }
}

/**
   @brief Try to join the saved network.
   @return true once connected, false if none is saved or it timed out
*/
static bool connectStation()
{
  String sta_ssid;
  String sta_pass;
  if (!loadWifiCredentials(sta_ssid, sta_pass))
  {
    Serial.println("No saved WiFi network");
    return false;
  }

  Serial.print("Joining WiFi network ");
  Serial.print(sta_ssid);

  // The credentials live in our own NVS namespace; stop the core keeping a
  // second copy that it would rewrite on every boot.
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(sta_ssid.c_str(), sta_pass.c_str());

  const unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_STA_TIMEOUT_MS)
  {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Could not join the network, falling back to AP");
    // Stop retrying, or the station would keep hopping channels under the AP
    WiFi.setAutoReconnect(false);
    WiFi.disconnect(true);
    return false;
  }

  // A router blip later on reconnects on its own
  WiFi.setAutoReconnect(true);
  Serial.print("Station IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

/**
   @brief Join the saved WiFi network, or start the robot's own access point
   if there is none or it cannot be reached.
*/
void setupWiFi()
{
  // Register WiFi event handler
  WiFi.onEvent(WiFiEvent);
  WiFi.setHostname(WIFI_HOSTNAME);

  if (connectStation())
  {
    wifi_sta_mode = true;
    wifi_connected = true;
    return;
  }
  wifi_sta_mode = false;

  // Initialize WiFi Access Point
  WiFi.mode(WIFI_AP);
  wifi_connected = WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();

  if (!wifi_connected)
  {
    Serial.println("ERROR: Failed to create WiFi AP");
  }
  else
  {
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  }
}

/**
   @brief Register the OTA update handlers and start the OTA service.
*/
void setupOta()
{
  ArduinoOTA.setHostname(WIFI_HOSTNAME);
#ifdef OTA_PASSWORD
  ArduinoOTA.setPassword(OTA_PASSWORD);
#endif

  ArduinoOTA
      .onStart([]()
               {
      // Go limp rather than hold a stale pose while the flash is rewritten.
      setPwmEnabled(false);

      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS
      // using SPIFFS.end()
      Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", total ? (unsigned)(progress * 100ULL / total) : 0u); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      } });

  // Also starts mDNS under the hostname
  ArduinoOTA.begin();
  // Advertise the web page, so http://<hostname>.local/ finds the robot on a
  // shared network
  MDNS.addService("http", "tcp", 80);
  Serial.println("OTA update enabled, hostname: " WIFI_HOSTNAME);
}

/**
   @brief Start listening for command packets.
*/
void setupUdp()
{
  if (udp_socket.listen(UDP_PORT))
  {
    Serial.print("UDP Listening on IP: ");
    Serial.println(wifiIp());
    // Register callback for incoming UDP packets
    udp_socket.onPacket([](AsyncUDPPacket packet)
                        {
      // Any packet means a client has found the robot
      noteClientSeen();

      // Queries are answered here, where the sender is known. They are not
      // control input, so they skip the failsafe refresh below: polling the
      // version must not keep a gait running.
      if (packet.length() == sizeof(UdpVersionRequest) &&
          packet.data()[0] == MAGIC_VERSION)
      {
        replyVersion(packet);
        return;
      }

#if HEXAPOD_DEBUG
      // Pose packets arrive at the control rate (50 Hz). Logging and echoing
      // each one would saturate the serial port and stall the UDP task, so
      // chatter is limited to the low-rate packet types.
      const bool is_stream = packet.length() > 0 && packet.data()[0] == MAGIC_POSE;

      if (!is_stream)
      {
        // Log packet details for debugging
        Serial.print("UDP Packet Type: ");
        Serial.print(packet.isBroadcast()   ? "Broadcast"
                     : packet.isMulticast() ? "Multicast"
                                            : "Unicast");
        Serial.print(", From: ");
        Serial.print(packet.remoteIP());
        Serial.print(":");
        Serial.print(packet.remotePort());
        Serial.print(", To: ");
        Serial.print(packet.localIP());
        Serial.print(":");
        Serial.print(packet.localPort());
        Serial.print(", Length: ");
        Serial.print(packet.length());
        Serial.println();
        // reply to the client
        packet.printf("Got %u bytes of data", packet.length());
      }
#endif

      // Update failsafe timestamp
      last_udp_packet_time = millis();
      // Parse command from packet
      parseCommand((char*)packet.data(), packet.length()); });
  }
}

/**
   @brief Answer a version query with a UdpVersionReply followed by
   FIRMWARE_BUILD (UDP task).
   @param packet The UdpVersionRequest; the reply goes back to its sender
*/
void replyVersion(AsyncUDPPacket &packet)
{
  const UdpVersionRequest *request = (const UdpVersionRequest *)packet.data();

  static const char build[] = FIRMWARE_BUILD;
  const size_t build_len = sizeof(build) - 1; // Drop the terminator
  uint8_t reply[sizeof(UdpVersionReply) + sizeof(build)];

  UdpVersionReply header;
  header.magic = MAGIC_VERSION;
  header.seq_num = request->seq_num;
  header.protocol = PROTOCOL_VERSION;
  header.major = FIRMWARE_VERSION_MAJOR;
  header.minor = FIRMWARE_VERSION_MINOR;
  header.patch = FIRMWARE_VERSION_PATCH;

  memcpy(reply, &header, sizeof(header));
  memcpy(reply + sizeof(header), build, build_len);
  packet.write(reply, sizeof(header) + build_len);
}

/**
   @brief Switch to LUT playback of a motion (UDP task).

   Shared by the binary and the legacy string commands, so both leave real-time
   mode, wake relaxed servos and disable OTA the same way.
   @param motion_idx Index into motion_config[]
*/
void selectMotion(int motion_idx)
{
  // A motion command implies the operator wants LUT playback, not streaming.
  exitRealtimeMode();
  relax_requested = false;
  next_motion_idx = motion_idx;

  // Disable OTA after first command for better performance
  ota_mode = false;
}

/**
   @brief Parse UDP command string and update motion mode.
   @param data Command data buffer
   @param length Length of command data
*/
void parseCommand(char *data, size_t length)
{
  if (length == 0)
    return;

  // Binary packets are selected by their first byte, then validated by length.
  const uint8_t magic = (uint8_t)data[0];

  // Pre-programmed motion command, optionally carrying a playback speed
  if (magic == MAGIC_MOTION && (length == sizeof(UdpControlPacket) ||
                                length == sizeof(UdpControlSpeedPacket))) {
    UdpControlPacket* packet = (UdpControlPacket*)data;
    if ((size_t)packet->cmd < motion_config_count) {
      if (length == sizeof(UdpControlSpeedPacket)) {
        const uint8_t speed_pct = ((UdpControlSpeedPacket*)data)->speed_pct;
        if (speed_pct != 0) {
          setMotionSpeed(speed_pct);
        }
      }
      selectMotion(packet->cmd);
    }
    return;
  }

  // Real-time pose
  if (magic == MAGIC_POSE && length == sizeof(UdpPosePacket)) {
    UdpPosePacket* packet = (UdpPosePacket*)data;

    // A pose arriving while idle implicitly opens a streaming session.
    if (!realtime_mode) {
      pose_seq_valid = false;
    }
    enterRealtimeMode();

    // Drop poses that UDP delivered out of order; applying them would make the
    // legs twitch back to where they just were.
    const int32_t behind = (int32_t)(last_pose_seq - packet->seq_num);
    if (pose_seq_valid && behind >= 0 && behind < POSE_REORDER_WINDOW) {
      return;
    }
    last_pose_seq = packet->seq_num;
    pose_seq_valid = true;

    portENTER_CRITICAL(&realtime_mux);
    for (int leg_idx = 0; leg_idx < 3; leg_idx++) {
      for (int joint_idx = 0; joint_idx < 3; joint_idx++) {
        // Streamed poses are uncalibrated, so bound each joint to the window
        // that lands inside [SERVOMIN, SERVOMAX] once writeServo() adds its
        // offset. Clamping against the raw range instead would cost the joint
        // |offset| ticks of travel at one end.
        const int right_offset = right_offset_ticks[leg_idx][joint_idx];
        const int left_offset = left_offset_ticks[leg_idx][joint_idx];
        realtime_target[leg_idx][joint_idx] =
            constrain((int)packet->ticks[leg_idx][joint_idx],
                      SERVOMIN - right_offset, SERVOMAX - right_offset);
        realtime_target[leg_idx + 3][joint_idx] =
            constrain((int)packet->ticks[leg_idx + 3][joint_idx],
                      SERVOMIN - left_offset, SERVOMAX - left_offset);
      }
    }
    if (packet->max_step > 0) {
      realtime_max_step = packet->max_step;
    }
    if (packet->flags & POSE_FLAG_SNAP) {
      realtime_snap = true;
    }
    realtime_target_valid = true;
    portEXIT_CRITICAL(&realtime_mux);

    realtime_last_packet_time = millis();
    return;
  }

  // Real-time session control
  if (magic == MAGIC_SESSION && length == sizeof(UdpSessionPacket)) {
    UdpSessionPacket* packet = (UdpSessionPacket*)data;

    switch (packet->action) {
    case RT_ENTER:
      pose_seq_valid = false;
      enterRealtimeMode();
      break;
    case RT_EXIT:
      exitRealtimeMode();
      break;
    case RT_RELAX:
      // Drop PWM drive so the servos go limp. Any motion command, pose or
      // RT_ENTER re-enables the drivers.
      exitRealtimeMode();
      relax_requested = true;
      break;
    case RT_PING:
      realtime_last_packet_time = millis();
      break;
    default:
      Serial.print("Unknown session action: ");
      Serial.println(packet->action);
      break;
    }
    return;
  }

  // Fallback to legacy string parsing
  // Command buffer with space for null terminator
  char command[32] = {0};
  size_t cmd_len = 0;

  // Parse command string from UDP packet
  // Stops at delimiters: ':', '\n', '\r', '\0'
  for (size_t i = 0; i < length && i < sizeof(command) - 1; i++)
  {
    char c = data[i];

    // Check for delimiters that end the command
    if (c == ':' || c == '\n' || c == '\r' || c == '\0')
    {
      if (cmd_len > 0)
      {
        command[cmd_len] = '\0';
        break;
      }
      continue; // Skip leading delimiters
    }

    // Add character to command buffer
    command[cmd_len++] = c;
  }

  // Null-terminate if we reached the end without a delimiter
  if (cmd_len > 0 && cmd_len < sizeof(command))
  {
    command[cmd_len] = '\0';
  }

  if (cmd_len == 0)
    return;

  // Find matching command and set motion index
  for (size_t i = 0; i < motion_config_count; i++)
  {
    if (strcmp(command, motion_config[i].cmd) == 0)
    {
      selectMotion(i);
#if HEXAPOD_DEBUG
      Serial.print("Command received: ");
      Serial.println(command);
#endif
      return;
    }
  }

  Serial.print("Unknown command: ");
  Serial.println(command);
}

/**
   @brief Handle WiFi events: trigger boot when a device joins the AP, log
   station connection changes.
   @param event WiFi event ID
*/
void WiFiEvent(arduino_event_id_t event)
{
  switch (event)
  {
  case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
    // Client connected - trigger boot sequence if not already done
    Serial.println("Station connected to AP");
    noteClientSeen();
    break;
  case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
    // Client disconnected - servos remain in last position
    Serial.println("Station disconnected from AP");
    break;
  case ARDUINO_EVENT_WIFI_AP_START:
    // Access point initialized and ready
    Serial.println("AP Started");
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    Serial.print("Station got IP: ");
    Serial.println(WiFi.localIP());
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    // Only worth noting once connected; auto-reconnect handles the rest
    if (wifi_sta_mode)
    {
      Serial.println("Lost the WiFi network, reconnecting");
    }
    break;
  default:
    break;
  }
}
