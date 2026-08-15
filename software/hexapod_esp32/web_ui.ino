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
    
    String json = "{\"left\":[";
    for (int i = 0; i < 3; i++) {
      json += "[";
      for (int j = 0; j < 3; j++) {
        json += String(left_offset_ticks[i][j]);
        if (j < 2) json += ",";
      }
      json += "]";
      if (i < 2) json += ",";
    }
    json += "],\"right\":[";
    for (int i = 0; i < 3; i++) {
      json += "[";
      for (int j = 0; j < 3; j++) {
        json += String(right_offset_ticks[i][j]);
        if (j < 2) json += ",";
      }
      json += "]";
      if (i < 2) json += ",";
    }
    json += "]}";
    web_server.send(200, "application/json", json); });

  // Exit calibration mode
  web_server.on("/exit_calibration", HTTP_GET, []()
                {
    calibration_mode = false;
    Serial.println("Exited calibration mode");
    web_server.send(200, "text/plain", "Exited calibration mode"); });

  // Get current offset values
  web_server.on("/get_offsets", HTTP_GET, []()
                {
    String json = "{\"left\":[";
    for (int i = 0; i < 3; i++) {
      json += "[";
      for (int j = 0; j < 3; j++) {
        json += String(left_offset_ticks[i][j]);
        if (j < 2) json += ",";
      }
      json += "]";
      if (i < 2) json += ",";
    }
    json += "],\"right\":[";
    for (int i = 0; i < 3; i++) {
      json += "[";
      for (int j = 0; j < 3; j++) {
        json += String(right_offset_ticks[i][j]);
        if (j < 2) json += ",";
      }
      json += "]";
      if (i < 2) json += ",";
    }
    json += "]}";
    web_server.send(200, "application/json", json); });

  // Set offset values and apply them
  web_server.on("/set_offsets", HTTP_POST, []()
                {
    if (web_server.hasArg("plain")) {
      String body = web_server.arg("plain");
      
      // Simple JSON parsing for offset values
      int leftStart = body.indexOf("\"left\":[") + 8;
      int rightStart = body.indexOf("\"right\":[") + 9;
      
      // Parse left side offsets
      int pos = leftStart;
      for (int i = 0; i < 3; i++) {
        pos = body.indexOf('[', pos) + 1;
        for (int j = 0; j < 3; j++) {
          int endPos = body.indexOf(j < 2 ? ',' : ']', pos);
          left_offset_ticks[i][j] = body.substring(pos, endPos).toInt();
          pos = endPos + 1;
        }
      }
      
      // Parse right side offsets
      pos = rightStart;
      for (int i = 0; i < 3; i++) {
        pos = body.indexOf('[', pos) + 1;
        for (int j = 0; j < 3; j++) {
          int endPos = body.indexOf(j < 2 ? ',' : ']', pos);
          right_offset_ticks[i][j] = body.substring(pos, endPos).toInt();
          pos = endPos + 1;
        }
      }
      
      // Apply calibration with new offsets
      posture_calibration();
      
      Serial.println("Offsets updated and applied");
      web_server.send(200, "text/plain", "Offsets applied!");
    } else {
      web_server.send(400, "text/plain", "No data received");
    } });

  // Save offsets to EEPROM and serial
  web_server.on("/save_offsets", HTTP_POST, []()
                {
    // Save to EEPROM
    saveOffsetsToEEPROM();
    
    // Also print to serial for manual backup
    Serial.println("\n=== Updated Offset Values ===");
    Serial.println("Copy these to config.h:");
    Serial.println();
    
    Serial.print("static int left_offset_ticks[3][3] = { ");
    for (int i = 0; i < 3; i++) {
      Serial.print("{ ");
      for (int j = 0; j < 3; j++) {
        Serial.print(left_offset_ticks[i][j]);
        if (j < 2) Serial.print(", ");
      }
      Serial.print(" }");
      if (i < 2) Serial.print(", ");
    }
    Serial.println(" };");
    
    Serial.print("static int right_offset_ticks[3][3] = { ");
    for (int i = 0; i < 3; i++) {
      Serial.print("{ ");
      for (int j = 0; j < 3; j++) {
        Serial.print(right_offset_ticks[i][j]);
        if (j < 2) Serial.print(", ");
      }
      Serial.print(" }");
      if (i < 2) Serial.print(", ");
    }
    Serial.println(" };");
    Serial.println("==============================\n");
    
    web_server.send(200, "text/plain", "Offsets saved to EEPROM!"); });

  // Start server
  web_server.begin();
  Serial.println("Web server started on port 80");
}
