/**

  Configuration file

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

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Robot-specific settings (DELAY_MS, APSSID) come from the robot picked here
#include "robot.h"

#define SERVOMIN 102  // Minimum value, 0 deg
#define SERVOMID 307  // Middle value, 90 deg
#define SERVOMAX 512  // Maximum value, 180 deg

/** Real-time pose streaming */
// Control cycle period while streaming poses (ms). 20 ms -> 50 Hz.
#define REALTIME_PERIOD_MS 20
// Drop out of real-time mode if no packet arrives within this window (ms).
#define REALTIME_TIMEOUT_MS 1000
// Default per-joint slew limit, in ticks per control cycle. Caps how fast a
// streamed pose can be chased so a large jump cannot slam the servos.
#define REALTIME_DEFAULT_MAX_STEP 8

/** Motion LUT playback */
// Fall back to standby if no UDP packet arrives within this window (ms). Clients
// driving the LUT engine must repeat their motion command faster than this.
#define MOTION_TIMEOUT_MS 500
// LUT playback speed, as a percentage of the robot's tuned frame rate
// (DELAY_MS). Slower speeds blend between LUT frames, so the servos keep moving
// smoothly; faster is not allowed.
#define MOTION_SPEED_MIN_PCT 20
#define MOTION_SPEED_DEFAULT_PCT 60

/** Debugging */
// 1 = log and echo every low-rate UDP packet on the serial port
#ifndef HEXAPOD_DEBUG
#define HEXAPOD_DEBUG 0
#endif

/** PCA9685 PWM drivers */
// I2C addresses for the left and right servo banks
const uint8_t LEFT_PWM_ADDRESS = 0x40;   // Left side servos (3 legs)
const uint8_t RIGHT_PWM_ADDRESS = 0x41;  // Right side servos (3 legs)

// GPIO pins for PWM driver enable control (active LOW)
const uint8_t LEFT_PWM_ENABLE_PIN = 19;   // Enable left legs PWM driver
const uint8_t RIGHT_PWM_ENABLE_PIN = 26;  // Enable right legs PWM driver

// PWM frequency for servo control signals (Hz)
const uint16_t SERVO_PWM_FREQ = 50;

// I2C bus clock (Hz). The PCA9685 supports up to 1 MHz; at the 100 kHz default
// writing all 18 servos takes ~10 ms, which stretched every LUT frame.
const uint32_t I2C_CLOCK_HZ = 400000;

// PCA9685 internal oscillator (Hz). Nominally 25 MHz, but individual chips vary
// by a few percent; measure a pulse and adjust if servo angles are off.
const uint32_t PCA9685_OSC_HZ = 25000000;

// Delay between individual servo movements during boot (ms)
const uint16_t SERVO_INIT_DELAY_MS = 50;

// Step size for smooth transitions between positions
const uint8_t TRANSITION_TICK_STEP = 6;

// Servo connections to the PCA9685 driver
// {{leg1_join1, leg1_join2, leg1_join3},
//  {leg2_join1, leg2_join2, leg2_join3},
//  {leg3_join1, leg3_join2, leg3_join3}}
static const int left_legs[3][3] = { { 1, 2, 3 }, { 5, 6, 7 }, { 9, 8, 10 } };
static const int right_legs[3][3] = { { 10, 9, 8 }, { 13, 14, 15 }, { 7, 6, 5 } };

// Offset to correct the installation error. Offset value is the number of ticks.
// These are the live calibration values: the web interface edits them and
// calibration.ino restores them from flash at boot. They stay `static` because
// the sketch tabs compile as a single translation unit.
static int left_offset_ticks[3][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };
static int right_offset_ticks[3][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };

// Largest calibration offset accepted from the web interface or from flash
// (ticks, ~44 deg). Anything bigger is a typo or corrupt data, not a trim.
const int CALIBRATION_MAX_OFFSET = 100;

/** Calibration storage (NVS via Preferences) */
#define CALIBRATION_NVS_NAMESPACE "hexapod"
#define CALIBRATION_NVS_KEY "offsets"
const uint8_t CALIBRATION_VERSION = 1;  // Bump when the stored layout changes

/** Legacy EEPROM layout -- read once to migrate old calibrations to NVS */
const uint16_t EEPROM_SIZE = 64;        // Total EEPROM size to allocate
const uint16_t EEPROM_MAGIC = 0xABCD;   // Magic number to verify valid data
const uint16_t EEPROM_ADDR_MAGIC = 0;   // Address for magic number
const uint16_t EEPROM_ADDR_LEFT = 2;    // Address for left offsets
const uint16_t EEPROM_ADDR_RIGHT = 20;  // Address for right offsets (after 18 bytes)

/** WiFi Configurations */
// The access point name (APSSID) is set per robot in src/robots/<name>/robot_config.h
#ifndef APPSK
#define APPSK "hexapod_1234"
#endif

#define UDP_PORT 1234  // local port to listen on

/** OTA */
// Uncomment (or pass -DOTA_PASSWORD=...) to require a password for OTA uploads.
// Without it, anyone who joins the robot's access point can flash firmware.
// #define OTA_PASSWORD "change_me"

#endif  // CONFIG_H
