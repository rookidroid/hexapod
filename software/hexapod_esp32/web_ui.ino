/**

  Calibration web interface -- HTTP routes backing web_page.h

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

*/

#include <WebServer.h>

#include "hexapod.h"
#include "web_page.h"

// Web server for calibration interface
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
   @brief Setup web server routes for calibration interface.
*/
void setupWebServer()
{
  // Serve main page with calibration button
  web_server.on("/", HTTP_GET, []()
                { web_server.send(200, "text/html", index_html); });

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

  // Start server
  web_server.begin();
  Serial.println("Web server started on port 80");
}
