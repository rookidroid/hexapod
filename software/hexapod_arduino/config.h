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

#define SERVOMIN 205  // Minimum value, 0 deg
#define SERVOMID 307  // Middle value, 90 deg
#define SERVOMAX 512  // Maximum value, 180 deg

#define DELAY_MS 12  // Servo delay

/** Motion Mode */
enum MotionMode {
  Mode_Standby,
  Mode_Walk_0,
  Mode_Walk_180,
  Mode_Walk_R45,
  Mode_Walk_R90,
  Mode_Walk_R135,
  Mode_Walk_L45,
  Mode_Walk_L90,
  Mode_Walk_L135,
  Mode_Fast_Forward,
  Mode_Fast_Backward,
  Mode_Turn_Left,
  Mode_Turn_Right,
  Mode_Climb_Forward,
  Mode_Climb_Backward,
  Mode_Rotate_X,
  Mode_Rotate_Y,
  Mode_Rotate_Z,
  Mode_Twist,
};

// Servo connections to the PICO
// {{leg1_join1, leg1_join2, leg1_join3},
//  {leg2_join1, leg2_join2, leg2_join3},
//  {leg3_join1, leg3_join2, leg3_join3}}
static int left_legs[3][3] = {{0, 1, 2}, {6, 7, 8}, {13, 14, 15}};
static int right_legs[3][3] = {{27, 26, 22}, {21, 20, 19}, {18, 17, 16}};

// Offset to correct the installation error. Offset value is the number of ticks
static int left_offset_ticks[3][3] = {
    {0, 10, -10}, {-12, 10, 15}, {-3, -10, 10}};
static int right_offset_ticks[3][3] = {
    {-10, 5, 0}, {3, -5, -10}, {15, -10, -10}};

/** WiFi Configurations */
#ifndef APSSID
#define APSSID "hexapod"
#define APPSK "hexapod_1234"
#endif

#define UDP_PORT 1234  // local port to listen on

#endif  // CONFIG_H
