/**

  Web interface -- HTTP routes backing web_page.h (drive, calibration, WiFi)

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

*/

#include <WebServer.h>

#include "hexapod.h"
#include "robot_geometry.h"
#include "web_page.h"

// Web server for the drive and calibration page
WebServer web_server(80);

/**
   @brief Append a 3x3 offset table to `out` as nested JSON arrays.
*/
static void appendOffsetArray(String &out, const int offsets[3][3])
{
  out += "[";
  for (int i = 0; i < 3; i++)
  {
    out += "[";
    for (int j = 0; j < 3; j++)
    {
      out += String(offsets[i][j]);
      if (j < 2)
        out += ",";
    }
    out += "]";
    if (i < 2)
      out += ",";
  }
  out += "]";
}

/**
   @brief Append `text` to `out` as a quoted JSON string. SSIDs are arbitrary
   bytes, so quotes, backslashes and control characters are escaped.
*/
static void appendJsonString(String &out, const String &text)
{
  out += '"';
  for (size_t i = 0; i < text.length(); i++)
  {
    const char c = text[i];
    if (c == '"' || c == '\\')
    {
      out += '\\';
      out += c;
    }
    else if ((uint8_t)c < 0x20)
    {
      char escaped[7];
      snprintf(escaped, sizeof(escaped), "\\u%04x", (uint8_t)c);
      out += escaped;
    }
    else
    {
      out += c;
    }
  }
  out += '"';
}

/**
   @brief The network the robot is on, and the one saved for the next boot.
   The saved password is never sent back.
*/
static String wifiStatusJson()
{
  String saved_ssid;
  String saved_pass;
  loadWifiCredentials(saved_ssid, saved_pass);

  String json = "{\"mode\":\"";
  json += wifi_sta_mode ? "sta" : "ap";
  json += "\",\"ssid\":";
  appendJsonString(json, wifi_sta_mode ? WiFi.SSID() : String(APSSID));
  json += ",\"ip\":\"" + wifiIp().toString() + "\"";
  if (wifi_sta_mode)
  {
    json += ",\"rssi\":" + String(WiFi.RSSI());
  }
  json += ",\"saved\":";
  appendJsonString(json, saved_ssid);
  json += ",\"ap_ssid\":\"" APSSID "\"";
  json += ",\"hostname\":\"" WIFI_HOSTNAME "\"";
  json += ",\"timeout_ms\":" + String(WIFI_STA_TIMEOUT_MS);
  json += "}";
  return json;
}

/**
   @brief Scan for networks to join, as [{"ssid","rssi","secure"}], one entry
   per SSID (its strongest access point), strongest first.

   Blocks for a couple of seconds while the radio scans.
*/
static String wifiScanJson()
{
  // A pure access point cannot scan; add the station side. The AP stays up.
  if (WiFi.getMode() == WIFI_AP)
  {
    WiFi.mode(WIFI_AP_STA);
  }

  const int count = WiFi.scanNetworks();
  String json = "[";
  bool first = true;
  if (count > 0)
  {
    // Results come back strongest first, so the first sighting of an SSID is
    // its best access point.
    for (int i = 0; i < count; i++)
    {
      const String name = WiFi.SSID(i);
      if (name.length() == 0)
        continue; // Hidden network
      bool seen = false;
      for (int j = 0; j < i && !seen; j++)
      {
        seen = WiFi.SSID(j) == name;
      }
      if (seen)
        continue;

      if (!first)
        json += ",";
      first = false;
      json += "{\"ssid\":";
      appendJsonString(json, name);
      json += ",\"rssi\":" + String(WiFi.RSSI(i));
      json += ",\"secure\":";
      json += WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "false" : "true";
      json += "}";
    }
  }
  WiFi.scanDelete();
  json += "]";
  return json;
}

/**
   @brief Current offsets as {"left":[[...]],"right":[[...]]}.
*/
static String offsetsJson()
{
  String json = "{\"left\":";
  appendOffsetArray(json, left_offset_ticks);
  json += ",\"right\":";
  appendOffsetArray(json, right_offset_ticks);
  json += "}";
  return json;
}

/**
   @brief Everything a client needs to model and drive this robot, so it does
   not have to carry its own copy of each robot's config.

   `commands` lists the motion names in RobotCommand order, so a name's index
   is the command ID to send. `geometry` is the robot's path_tool config.
*/
static String robotConfigJson()
{
  String json = "{\"protocol\":" + String(PROTOCOL_VERSION);
  json += ",\"firmware\":{\"version\":\"" FIRMWARE_VERSION
          "\",\"build\":\"" FIRMWARE_BUILD "\"}";
  json += ",\"name\":\"" ROBOT_NAME "\"";
  json += ",\"ssid\":\"" APSSID "\"";
  json += ",\"wifi_mode\":\"";
  json += wifi_sta_mode ? "sta" : "ap";
  json += "\"";
  json += ",\"delay_ms\":" + String(DELAY_MS);
  json += ",\"servo\":{\"min\":" + String(SERVOMIN) +
          ",\"mid\":" + String(SERVOMID) +
          ",\"max\":" + String(SERVOMAX) + "}";
  json += ",\"speed\":{\"min\":" + String(MOTION_SPEED_MIN_PCT) +
          ",\"max\":100,\"default\":" + String(MOTION_SPEED_DEFAULT_PCT) +
          ",\"current\":" + String(motion_speed_pct) + "}";
  json += ",\"commands\":[";
  for (size_t i = 0; i < motion_config_count; i++)
  {
    if (i > 0)
      json += ",";
    json += "\"";
    json += motion_config[i].cmd;
    json += "\"";
  }
  json += "],\"geometry\":";
  json += ROBOT_GEOMETRY_JSON;
  json += "}";
  return json;
}

/**
   @brief Parse the 3x3 array that follows `key` in `body`. `key` includes the
   array's opening bracket, e.g. "\"left\":[".

   A minimal parser for the fixed shape the calibration page sends. Every
   delimiter is checked, so a truncated or malformed body is rejected instead of
   turning into garbage offsets.
   @return true if all nine values were found
*/
static bool parseOffsetArray(const String &body, const char *key, int out[3][3])
{
  int pos = body.indexOf(key);
  if (pos < 0)
    return false;
  pos += strlen(key);

  for (int i = 0; i < 3; i++)
  {
    pos = body.indexOf('[', pos);
    if (pos < 0)
      return false;
    pos++;
    for (int j = 0; j < 3; j++)
    {
      const int end = body.indexOf(j < 2 ? ',' : ']', pos);
      if (end < 0)
        return false;
      out[i][j] = clampOffset(body.substring(pos, end).toInt());
      pos = end + 1;
    }
  }
  return true;
}

/**
   @brief Print the offsets as config.h declarations for a manual backup.
*/
static void printOffsetsForConfig()
{
  Serial.println("\n=== Updated Offset Values ===");
  Serial.println("Copy these to config.h:");
  Serial.println();

  const char *names[2] = {"left_offset_ticks", "right_offset_ticks"};
  const int(*tables[2])[3] = {left_offset_ticks, right_offset_ticks};
  for (int side = 0; side < 2; side++)
  {
    Serial.print("static int ");
    Serial.print(names[side]);
    Serial.print("[3][3] = { ");
    for (int i = 0; i < 3; i++)
    {
      Serial.print("{ ");
      for (int j = 0; j < 3; j++)
      {
        Serial.print(tables[side][i][j]);
        if (j < 2)
          Serial.print(", ");
      }
      Serial.print(" }");
      if (i < 2)
        Serial.print(", ");
    }
    Serial.println(" };");
  }
  Serial.println("==============================\n");
}

/**
   @brief Setup web server routes for the drive and calibration page.
*/
void setupWebServer()
{
  // Serve the page. send_P streams it from flash rather than
  // copying it into a String on the heap first.
  web_server.on("/", HTTP_GET, []()
                {
    noteClientSeen();
    web_server.send_P(200, "text/html", index_html); });
  // Enter calibration mode and get current offset values
  web_server.on("/enter_calibration", HTTP_GET, []()
                {
    calibration_mode = true;
    Serial.println("Entered calibration mode");
    web_server.send(200, "application/json", offsetsJson()); });

  // Exit calibration mode
  web_server.on("/exit_calibration", HTTP_GET, []()
                {
    calibration_mode = false;
    Serial.println("Exited calibration mode");
    web_server.send(200, "text/plain", "Exited calibration mode"); });

  // Get current offset values
  web_server.on("/get_offsets", HTTP_GET, []()
                { web_server.send(200, "application/json", offsetsJson()); });

  // Set offset values and apply them
  web_server.on("/set_offsets", HTTP_POST, []()
                {
    // Outside calibration mode the robot may be mid-gait, where shifting the
    // offsets would jolt every leg.
    if (!calibration_mode) {
      web_server.send(409, "text/plain", "Enter calibration mode first");
      return;
    }
    if (!web_server.hasArg("plain")) {
      web_server.send(400, "text/plain", "No data received");
      return;
    }

    // Parse into scratch tables so a bad body leaves the live offsets intact.
    const String body = web_server.arg("plain");
    int left[3][3];
    int right[3][3];
    if (!parseOffsetArray(body, "\"left\":[", left) ||
        !parseOffsetArray(body, "\"right\":[", right)) {
      web_server.send(400, "text/plain", "Malformed offsets");
      return;
    }
    memcpy(left_offset_ticks, left, sizeof(left));
    memcpy(right_offset_ticks, right, sizeof(right));

    // The motion engine rewrites the calibration posture every tick, so the new
    // offsets show up on their own. Before boot it is not running yet.
    if (!boot_sequence_executed) {
      posture_calibration();
    }

    Serial.println("Offsets updated and applied");
    web_server.send(200, "text/plain", "Offsets applied!"); });

  // Save offsets to flash and serial
  web_server.on("/save_offsets", HTTP_POST, []()
                {
    const bool saved = saveOffsets();

    // Also print to serial for manual backup
    printOffsetsForConfig();

    if (saved) {
      web_server.send(200, "text/plain", "Offsets saved to flash!");
    } else {
      web_server.send(500, "text/plain", "Failed to save offsets");
    } });

  // Describe this robot: identity, geometry, servo range, speed and commands
  web_server.on("/robot_config", HTTP_GET, []()
                {
    noteClientSeen();
    web_server.send(200, "application/json", robotConfigJson()); });

  // Get the LUT playback speed
  web_server.on("/get_speed", HTTP_GET, []()
                { web_server.send(200, "application/json",
                                  "{\"speed\":" + String(motion_speed_pct) + "}"); });

  // Set the LUT playback speed (not saved; a connected remote overrides it)
  web_server.on("/set_speed", HTTP_POST, []()
                {
    if (!web_server.hasArg("pct")) {
      web_server.send(400, "text/plain", "Missing pct");
      return;
    }
    setMotionSpeed(web_server.arg("pct").toInt());
    web_server.send(200, "application/json",
                    "{\"speed\":" + String(motion_speed_pct) + "}"); });

  // Play a motion from the browser: the HTTP twin of the 0xA5 UDP packet. The
  // same failsafe applies, so the page resends it while a button is held.
  web_server.on("/motion", HTTP_POST, []()
                {
    // Calibration mode outranks motion in the engine; say so instead of
    // silently ignoring the command.
    if (calibration_mode) {
      web_server.send(409, "text/plain", "Exit calibration first");
      return;
    }
    if (!web_server.hasArg("id")) {
      web_server.send(400, "text/plain", "Missing id");
      return;
    }
    const long id = web_server.arg("id").toInt();
    if (id < 0 || (size_t)id >= motion_config_count) {
      web_server.send(400, "text/plain", "Unknown motion");
      return;
    }
    if (web_server.hasArg("pct")) {
      setMotionSpeed(web_server.arg("pct").toInt());
    }
    last_udp_packet_time = millis();
    selectMotion(id);
    web_server.send(200, "application/json",
                    "{\"motion\":" + String(id) +
                    ",\"speed\":" + String(motion_speed_pct) + "}"); });

  // Cut PWM drive so the servos go limp, like the RT_RELAX session packet. The
  // next motion command wakes them.
  web_server.on("/relax", HTTP_POST, []()
                {
    if (calibration_mode) {
      web_server.send(409, "text/plain", "Exit calibration first");
      return;
    }
    exitRealtimeMode();
    relax_requested = true;
    web_server.send(200, "text/plain", "Servos relaxed"); });

  // Which network the robot is on, and the one saved for the next boot
  web_server.on("/wifi_status", HTTP_GET, []()
                { web_server.send(200, "application/json", wifiStatusJson()); });

  // Networks in range, for the WiFi tab's picker
  web_server.on("/wifi_scan", HTTP_GET, []()
                { web_server.send(200, "application/json", wifiScanJson()); });

  // Save a network to join, then reboot to join it. If it cannot be reached
  // the robot falls back to its own AP, so a typo cannot lock anyone out.
  web_server.on("/wifi_config", HTTP_POST, []()
                {
    const String new_ssid = web_server.arg("ssid");
    String new_pass = web_server.arg("password");
    if (new_ssid.length() == 0 || new_ssid.length() > 32) {
      web_server.send(400, "text/plain", "SSID must be 1-32 characters");
      return;
    }

    // A blank password re-saving the same network keeps the stored one, so
    // the page never needs to know it.
    String saved_ssid;
    String saved_pass;
    if (new_pass.length() == 0 && loadWifiCredentials(saved_ssid, saved_pass) &&
        saved_ssid == new_ssid) {
      new_pass = saved_pass;
    }
    if (new_pass.length() != 0 && (new_pass.length() < 8 || new_pass.length() > 63)) {
      web_server.send(400, "text/plain", "Password must be 8-63 characters, or blank for an open network");
      return;
    }

    if (!saveWifiCredentials(new_ssid, new_pass)) {
      web_server.send(500, "text/plain", "Failed to save WiFi settings");
      return;
    }
    web_server.send(200, "text/plain", "Saved. Rebooting to join " + new_ssid + "...");
    restart_requested = true; });

  // Forget the saved network and reboot into AP mode
  web_server.on("/wifi_forget", HTTP_POST, []()
                {
    if (!clearWifiCredentials()) {
      web_server.send(500, "text/plain", "Failed to forget the WiFi network");
      return;
    }
    web_server.send(200, "text/plain", "Network forgotten. Rebooting into access point mode...");
    restart_requested = true; });

  // Start server
  web_server.begin();
  Serial.println("Web server started on port 80");
}
