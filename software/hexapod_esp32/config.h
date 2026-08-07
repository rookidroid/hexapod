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

#define SERVOMIN 102  // Minimum value, 0 deg
#define SERVOMID 307  // Middle value, 90 deg
#define SERVOMAX 512  // Maximum value, 180 deg

#define DELAY_MS 25  // Servo delay

/** Real-time pose streaming */
// Control cycle period while streaming poses (ms). 20 ms -> 50 Hz.
#define REALTIME_PERIOD_MS 20
// Drop out of real-time mode if no packet arrives within this window (ms).
#define REALTIME_TIMEOUT_MS 1000
// Default per-joint slew limit, in ticks per control cycle. Caps how fast a
// streamed pose can be chased so a large jump cannot slam the servos.
#define REALTIME_DEFAULT_MAX_STEP 8

// Servo connections to the PCA9685 driver
// {{leg1_join1, leg1_join2, leg1_join3},
//  {leg2_join1, leg2_join2, leg2_join3},
//  {leg3_join1, leg3_join2, leg3_join3}}
static int left_legs[3][3] = { { 1, 2, 3 }, { 5, 6, 7 }, { 9, 8, 10 } };
static int right_legs[3][3] = { { 10, 9, 8 }, { 13, 14, 15 }, { 7, 6, 5 } };

// Offset to correct the installation error. Offset value is the number of ticks
static int left_offset_ticks[3][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };
static int right_offset_ticks[3][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };

/** WiFi Configurations */
#ifndef APSSID
#define APSSID "hexapod_macaroon"
#define APPSK "hexapod_1234"
#endif

#define UDP_PORT 1234  // local port to listen on

#endif  // CONFIG_H
