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

#define SERVOMIN 102  // Minimum value, 0 deg
#define SERVOMID 307  // Middle value, 90 deg
#define SERVOMAX 512  // Maximum value, 180 deg

#define DELAY_MS 12  // Servo delay

/** Real-time pose streaming */
// Control cycle period while streaming poses (ms). 20 ms -> 50 Hz.
#define REALTIME_PERIOD_MS 20
// Drop out of real-time mode if no packet arrives within this window (ms).
#define REALTIME_TIMEOUT_MS 1000
// Default per-joint slew limit, in ticks per control cycle. Caps how fast a
// streamed pose can be chased so a large jump cannot slam the servos.
#define REALTIME_DEFAULT_MAX_STEP 8

/** PCA9685 PWM drivers */
// I2C addresses for the left and right servo banks
const uint8_t LEFT_PWM_ADDRESS = 0x40;   // Left side servos (3 legs)
const uint8_t RIGHT_PWM_ADDRESS = 0x41;  // Right side servos (3 legs)

// GPIO pins for PWM driver enable control (active LOW)
const uint8_t LEFT_PWM_ENABLE_PIN = 19;   // Enable left legs PWM driver
const uint8_t RIGHT_PWM_ENABLE_PIN = 26;  // Enable right legs PWM driver

// PWM frequency for servo control signals (Hz)
const uint16_t SERVO_PWM_FREQ = 50;

// Delay between individual servo movements during boot (ms)
const uint16_t SERVO_INIT_DELAY_MS = 50;

// Step size for smooth transitions between positions
const uint8_t TRANSITION_TICK_STEP = 6;

// Servo connections to the PCA9685 driver
// {{leg1_join1, leg1_join2, leg1_join3},
//  {leg2_join1, leg2_join2, leg2_join3},
//  {leg3_join1, leg3_join2, leg3_join3}}
static int left_legs[3][3] = { { 1, 2, 3 }, { 5, 6, 7 }, { 9, 8, 10 } };
static int right_legs[3][3] = { { 10, 9, 8 }, { 13, 14, 15 }, { 7, 6, 5 } };

// Offset to correct the installation error. Offset value is the number of ticks.
// These are the live calibration values: the web interface edits them and
// calibration.ino restores them from EEPROM at boot. They stay `static` because
// the sketch tabs compile as a single translation unit.
static int left_offset_ticks[3][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };
static int right_offset_ticks[3][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };

/** EEPROM layout for the saved calibration */
const uint16_t EEPROM_SIZE = 64;        // Total EEPROM size to allocate
const uint16_t EEPROM_MAGIC = 0xABCD;   // Magic number to verify valid data
const uint16_t EEPROM_ADDR_MAGIC = 0;   // Address for magic number
const uint16_t EEPROM_ADDR_LEFT = 2;    // Address for left offsets
const uint16_t EEPROM_ADDR_RIGHT = 20;  // Address for right offsets (after 18 bytes)

/** WiFi Configurations */
#ifndef APSSID
#define APSSID "hexapod"
#define APPSK "hexapod_1234"
#endif

#define UDP_PORT 1234  // local port to listen on

#endif  // CONFIG_H
