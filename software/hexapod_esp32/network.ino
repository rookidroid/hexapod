/**

  Networking -- WiFi AP, OTA, and the UDP command endpoint

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

*/

#include <ArduinoOTA.h>
#include <AsyncUDP.h>
#include <WiFi.h>

#include "hexapod.h"

// UDP socket for receiving commands
AsyncUDP udp_socket;

// WiFi credentials (defined in config.h)
static const char *ssid = APSSID;
static const char *password = APPSK;

/**
   @brief Start the WiFi access point clients connect to.
*/
void setupWiFi()
{
  // Register WiFi event handler
  WiFi.onEvent(WiFiEvent);

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
  ArduinoOTA
      .onStart([]()
               {
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
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
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

  ArduinoOTA.begin();
  Serial.println("OTA update enabled");
}

/**
   @brief Start listening for command packets.
*/
void setupUdp()
{
  if (udp_socket.listen(UDP_PORT))
  {
    Serial.print("UDP Listening on IP: ");
    Serial.println(WiFi.softAPIP());
    // Register callback for incoming UDP packets
    udp_socket.onPacket([](AsyncUDPPacket packet)
                        {
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

      // Update failsafe timestamp
      last_udp_packet_time = millis();
      // Parse command from packet
      parseCommand((char*)packet.data(), packet.length()); });
  }
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

  // Pre-programmed motion command
  if (magic == MAGIC_MOTION && length == sizeof(UdpControlPacket)) {
    UdpControlPacket* packet = (UdpControlPacket*)data;
    if ((size_t)packet->cmd < motion_config_count) {
      // A motion command implies the operator wants LUT playback, not streaming.
      exitRealtimeMode();
      next_motion_idx = packet->cmd;
    }
    return;
  }

  // Real-time pose
  if (magic == MAGIC_POSE && length == sizeof(UdpPosePacket)) {
    UdpPosePacket* packet = (UdpPosePacket*)data;

    // A pose arriving while idle implicitly opens a streaming session.
    if (!realtime_mode) {
      enterRealtimeMode();
    }

    portENTER_CRITICAL(&realtime_mux);
    for (int leg_idx = 0; leg_idx < 3; leg_idx++) {
      for (int joint_idx = 0; joint_idx < 3; joint_idx++) {
        // Streamed poses are uncalibrated, so bound each joint to the window
        // that lands inside [SERVOMIN, SERVOMAX] once setAllServos() adds its
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
    // A fresh pose cancels an in-progress return to standby.
    realtime_returning = false;
    portEXIT_CRITICAL(&realtime_mux);

    realtime_last_packet_time = millis();
    return;
  }

  // Real-time session control
  if (magic == MAGIC_SESSION && length == sizeof(UdpSessionPacket)) {
    UdpSessionPacket* packet = (UdpSessionPacket*)data;

    switch (packet->action) {
    case RT_ENTER:
      enterRealtimeMode();
      break;
    case RT_EXIT:
      exitRealtimeMode();
      break;
    case RT_RELAX:
      // Drop PWM drive so the servos go limp. Recovering requires an explicit
      // re-entry, which re-enables the drivers from the standby posture.
      exitRealtimeMode();
      setPwmEnabled(false);
      Serial.println("Servos relaxed");
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
  bool found = false;
  for (size_t i = 0; i < motion_config_count; i++)
  {
    if (strcmp(command, motion_config[i].cmd) == 0)
    {
      next_motion_idx = i;
      found = true;
      break;
    }
  }

  if (!found)
  {
    Serial.print("Unknown command: ");
    Serial.println(command);
    return;
  }

  // Disable OTA after first command for better performance
  ota_mode = false;
  Serial.print("Command received: ");
  Serial.println(command);
}

/**
   @brief Handle WiFi events: trigger boot on connect, disable servos on
   disconnect.
   @param event WiFi event ID
*/
void WiFiEvent(arduino_event_id_t event)
{
  switch (event)
  {
  case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
    // Client connected - trigger boot sequence if not already done
    Serial.println("Station connected to AP");
    if (!boot_sequence_executed)
    {
      trigger_boot_sequence = true; // Flag checked in main loop
    }
    break;
  case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
    // Client disconnected - servos remain in last position
    Serial.println("Station disconnected from AP");
    break;
  case ARDUINO_EVENT_WIFI_AP_START:
    // Access point initialized and ready
    Serial.println("AP Started");
    break;
  default:
    break;
  }
}
