/**

  Hexapod -- A 3D Printed Hexapod Robot

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

                        **
                       ****
                        **
                        **
                        **
                        **

        **********************************
      **************************************
     ****************************************
     ********      ************      ********
     *******        **********        *******
     *******        **********        *******
     ********      ************      ********
     ****************************************
     ****************************************
     ****************************************
     ****************************************


            **************************

                ******************

*/

/** OTA */
#include <ArduinoOTA.h>

/** Shared state and module interfaces */
#include "hexapod.h"

/** Motion Path LUT */
#include "motion.h"

// ============================================================================
// System state
// ============================================================================
// Declared in hexapod.h; the modules read and write these through that header.

bool ota_mode = true;                // OTA updates enabled until first command
bool wifi_connected = false;         // WiFi AP connection status
bool boot_sequence_executed = false; // Boot sequence completion flag
volatile bool trigger_boot_sequence =
    false;                     // Boot trigger from WiFi event (volatile for ISR)
bool calibration_mode = false; // Calibration mode flag

// Current motion index in motion_config table
int current_motion_idx = 0; // Starts at standby
int next_motion_idx = 0;    // Next motion to execute

unsigned long last_udp_packet_time = 0; // Tracks last UDP packet for failsafe

/**
   @brief Initialize system: WiFi AP, OTA, calibration, PWM drivers, UDP and
   the web interface.
*/
void setup()
{
  Serial.begin(115200);
  while (!Serial && millis() < 3000)
  {
    ; // Wait for serial port to connect, timeout after 3s
  }
  Serial.println("\n=== Hexapod Robot Initializing ===");

  setupWiFi();
  setupOta();

  // Offsets first: every servo position written afterwards is shifted by them.
  setupCalibration();
  setupServos();

  setupUdp();
  setupWebServer();

  Serial.println("=== Initialization Complete ===");

  // posture_calibration();
}

/**
   @brief Main loop: execute boot sequence, motion commands, and OTA updates.
*/
void loop()
{
  // Handle boot sequence trigger from WiFi event
  if (trigger_boot_sequence && !boot_sequence_executed)
  {
    boot_sequence_executed = true;
    trigger_boot_sequence = false;
    boot_up_motion(lut_standup_length, lut_standup);
    return;
  }

  // Don't execute motion commands until boot sequence is complete
  if (!boot_sequence_executed)
  {
    web_server.handleClient(); // Handle web requests
    if (ota_mode)
    {
      ArduinoOTA.handle();
      delay(10);
    }
    return;
  }

  // Real-time streaming takes priority over the LUT engine. exec_motion()
  // blocks for a whole gait cycle, which would add seconds of latency, so the
  // streaming path returns early and never reaches it.
  if (realtime_mode && !calibration_mode)
  {
    if (!realtime_returning &&
        (millis() - realtime_last_packet_time > REALTIME_TIMEOUT_MS))
    {
      // Stream dropped. Ease back to standby rather than freezing mid-pose or
      // snapping, then hand control back to the motion LUT engine.
      Serial.println("Real-time stream lost, returning to standby");
      realtime_returning = true;
      portENTER_CRITICAL(&realtime_mux);
      copyPose(lut_standby[0], realtime_target);
      realtime_snap = false;
      portEXIT_CRITICAL(&realtime_mux);
    }

    serviceRealtimePose();

    if (realtime_returning && posesEqual(realtime_current, lut_standby[0]))
    {
      exitRealtimeMode();
    }

    web_server.handleClient();
    delay(REALTIME_PERIOD_MS);
    return;
  }

  if (!calibration_mode)
  {
    // Safety failsafe: if no UDP packet received for >500ms, go to standby
    if (last_udp_packet_time > 0 && (millis() - last_udp_packet_time > 500)) {
      next_motion_idx = CMD_STANDBY;
    }

    // Execute motion based on next_motion_idx
    exec_motion(motion_config[next_motion_idx].length,
                motion_config[next_motion_idx].lut);
  }

  web_server.handleClient(); // Handle web requests

  if (ota_mode)
  {
    ArduinoOTA.handle();
    delay(10); // Small delay to prevent watchdog issues
  }
}
