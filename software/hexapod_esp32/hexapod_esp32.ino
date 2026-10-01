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

volatile bool ota_mode = true;       // OTA updates enabled until first command
bool wifi_connected = false;         // WiFi AP connection status
bool boot_sequence_executed = false; // Boot sequence completion flag
volatile bool trigger_boot_sequence =
    false;                     // Boot trigger from WiFi event (volatile for ISR)
bool calibration_mode = false; // Calibration mode flag

volatile int next_motion_idx = CMD_STANDBY; // Motion requested over UDP

volatile unsigned long last_udp_packet_time = 0; // Tracks last UDP packet for failsafe

volatile uint8_t motion_speed_pct = MOTION_SPEED_DEFAULT_PCT; // LUT playback speed

/**
   @brief Set the LUT playback speed. Called from the UDP task and the web
   interface; a single byte, so the store is atomic.
   @param pct Speed in percent of the tuned frame rate
*/
void setMotionSpeed(int pct)
{
  motion_speed_pct = constrain(pct, MOTION_SPEED_MIN_PCT, 100);
}

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
  Serial.println("Robot: " ROBOT_NAME ", firmware " FIRMWARE_VERSION " (" FIRMWARE_BUILD ")");

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
   @brief Main loop: run the boot sequence once, then service the web
   interface, OTA and the motion engine without blocking.
*/
void loop()
{
  // Handle boot sequence trigger from WiFi event
  if (trigger_boot_sequence && !boot_sequence_executed)
  {
    boot_sequence_executed = true;
    trigger_boot_sequence = false;
    boot_up_motion(lut_standup_length, lut_standup);
  }

  web_server.handleClient();

  if (ota_mode)
  {
    ArduinoOTA.handle();
  }

  // Motion commands wait until the robot has stood up.
  if (boot_sequence_executed)
  {
    serviceMotionEngine();
  }

  delay(1); // Yield to the WiFi and UDP tasks
}
