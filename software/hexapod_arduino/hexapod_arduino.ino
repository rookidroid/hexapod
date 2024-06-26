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

/** WiFi */
#include <AsyncUDP.h>
#include <WiFi.h>

/** PWM */
#include <Adafruit_PWMServoDriver.h>

/** I2C */
#include <Wire.h>

/** OTA */
#include <ArduinoOTA.h>

/** Motion Path LUT */
#include "motion.h"

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

/** WiFi Configurations */
#ifndef APSSID
#define APSSID "hexapod"
#define APPSK "hexapod_1234"
#endif

#define UDP_PORT 1234  // local port to listen on

#define SERVOMIN 125  // Minimum value, 0 deg
#define SERVOMID 350  // Middle value, 90 deg
#define SERVOMAX 575  // Maximum value, 180 deg

const char *ssid = APSSID;
const char *password = APPSK;

AsyncUDP Udp;

Adafruit_PWMServoDriver left_pwm = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver right_pwm = Adafruit_PWMServoDriver(0x41);

int left_legs[3][3] = {{0, 3, 1}, {7, 5, 10}, {15, 12, 14}};
int right_legs[3][3] = {{15, 12, 14}, {8, 9, 5}, {0, 2, 1}};

int start_new_motion = 1;

// Offset to correct the installation error. Offset value is the number of ticks
int left_offset_ticks[3][3] = {{ -5, -20, 6}, { -15, 0, 14}, {20, 6, 0}};
int right_offset_ticks[3][3] = {{20, 15, 0}, { -15, 20, -20}, { -10, -8, 10}};

MotionMode next_motion = MotionMode::Mode_Standby;

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS
    // using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
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
    }
  });

  ArduinoOTA.begin();

  // Initialize the PCA9685 library
  left_pwm.begin();
  left_pwm.setPWMFreq(60);  // Set the PWM frequency of the PCA9685

  right_pwm.begin();
  right_pwm.setPWMFreq(60);  // Set the PWM frequency of the PCA9685

  if (Udp.listen(UDP_PORT)) {
    Serial.print("UDP Listening on IP: ");
    Serial.println(myIP);
    Udp.onPacket([](AsyncUDPPacket packet) {
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
      Serial.print(", Data: ");
      Serial.write(packet.data(), packet.length());
      Serial.println();
      // reply to the client
      packet.printf("Got %u bytes of data", packet.length());
      char *packet_ptr = (char *)packet.data();
      String inputString = "";
      for (int str_idx = 0; str_idx < packet.length(); str_idx++) {
        char inChar = packet_ptr[str_idx];

        if (inChar != '\n' && inChar != ':') {
          // add it to the inputString:
          inputString += inChar;
        } else if (inChar == ':') {
          MotionMode current_mode = next_motion;
          if (inputString == String("standby")) {
            next_motion = MotionMode::Mode_Standby;
          } else if (inputString == String("walk0")) {
            next_motion = MotionMode::Mode_Walk_0;
          } else if (inputString == String("walk180")) {
            next_motion = MotionMode::Mode_Walk_180;
          } else if (inputString == String("walkr45")) {
            next_motion = MotionMode::Mode_Walk_R45;
          } else if (inputString == String("walkr90")) {
            next_motion = MotionMode::Mode_Walk_R90;
          } else if (inputString == String("walkr135")) {
            next_motion = MotionMode::Mode_Walk_R135;
          } else if (inputString == String("walkl45")) {
            next_motion = MotionMode::Mode_Walk_L45;
          } else if (inputString == String("walkl90")) {
            next_motion = MotionMode::Mode_Walk_L90;
          } else if (inputString == String("walkl135")) {
            next_motion = MotionMode::Mode_Walk_L135;
          } else if (inputString == String("fastforward")) {
            next_motion = MotionMode::Mode_Fast_Forward;
          } else if (inputString == String("fastbackward")) {
            next_motion = MotionMode::Mode_Fast_Backward;
          } else if (inputString == String("turnleft")) {
            next_motion = MotionMode::Mode_Turn_Left;
          } else if (inputString == String("turnright")) {
            next_motion = MotionMode::Mode_Turn_Right;
          } else if (inputString == String("climbforward")) {
            next_motion = MotionMode::Mode_Climb_Forward;
          } else if (inputString == String("climbbackward")) {
            next_motion = MotionMode::Mode_Climb_Backward;
          } else if (inputString == String("rotatex")) {
            next_motion = MotionMode::Mode_Rotate_X;
          } else if (inputString == String("rotatey")) {
            next_motion = MotionMode::Mode_Rotate_Y;
          } else if (inputString == String("rotatez")) {
            next_motion = MotionMode::Mode_Rotate_Z;
          } else if (inputString == String("twist")) {
            next_motion = MotionMode::Mode_Twist;
          }

          if (current_mode != next_motion) {
            start_new_motion = 1;
          }
          inputString = "";
        }
      }
    });
  }

  exec_motion(lut_standup_length, lut_standup);
}

void loop() {
  if (next_motion == MotionMode::Mode_Walk_0) {
    exec_motion(lut_walk_0_length, lut_walk_0);
  } else if (next_motion == MotionMode::Mode_Walk_180) {
    exec_motion(lut_walk_180_length, lut_walk_180);
  } else if (next_motion == MotionMode::Mode_Walk_R45) {
    exec_motion(lut_walk_r45_length, lut_walk_r45);
  } else if (next_motion == MotionMode::Mode_Walk_R90) {
    exec_motion(lut_walk_r90_length, lut_walk_r90);
  } else if (next_motion == MotionMode::Mode_Walk_R135) {
    exec_motion(lut_walk_r135_length, lut_walk_r135);
  } else if (next_motion == MotionMode::Mode_Walk_L45) {
    exec_motion(lut_walk_l45_length, lut_walk_l45);
  } else if (next_motion == MotionMode::Mode_Walk_L90) {
    exec_motion(lut_walk_l90_length, lut_walk_l90);
  } else if (next_motion == MotionMode::Mode_Walk_L135) {
    exec_motion(lut_walk_l135_length, lut_walk_l135);
  } else if (next_motion == MotionMode::Mode_Fast_Forward) {
    exec_motion(lut_fast_forward_length, lut_fast_forward);
  } else if (next_motion == MotionMode::Mode_Fast_Backward) {
    exec_motion(lut_fast_backward_length, lut_fast_backward);
  } else if (next_motion == MotionMode::Mode_Turn_Left) {
    exec_motion(lut_turn_left_length, lut_turn_left);
  } else if (next_motion == MotionMode::Mode_Turn_Right) {
    exec_motion(lut_turn_right_length, lut_turn_right);
  } else if (next_motion == MotionMode::Mode_Climb_Forward) {
    exec_motion(lut_climb_forward_length, lut_climb_forward);
  } else if (next_motion == MotionMode::Mode_Climb_Backward) {
    exec_motion(lut_climb_backward_length, lut_climb_backward);
  } else if (next_motion == MotionMode::Mode_Rotate_X) {
    exec_motion(lut_rotate_x_length, lut_rotate_x);
  } else if (next_motion == MotionMode::Mode_Rotate_Y) {
    exec_motion(lut_rotate_y_length, lut_rotate_y);
  } else if (next_motion == MotionMode::Mode_Rotate_Z) {
    exec_motion(lut_rotate_z_length, lut_rotate_z);
  } else if (next_motion == MotionMode::Mode_Twist) {
    exec_motion(lut_twist_length, lut_twist);
  } else {
    exec_motion(lut_standby_length, lut_standby);
    start_new_motion = 1;
  }
  ArduinoOTA.handle();
}

void posture_calibration() {
  for (int leg_idx = 0; leg_idx < 3; leg_idx++) {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++) {
      right_pwm.setPWM(right_legs[leg_idx][joint_idx], 0,
                       SERVOMID + right_offset_ticks[leg_idx][joint_idx]);
      left_pwm.setPWM(left_legs[leg_idx][joint_idx], 0,
                      SERVOMID + left_offset_ticks[leg_idx][joint_idx]);
    }
  }
}

void exec_motion(int lut_size, int lut[][6][3]) {
  MotionMode current_mode = next_motion;
  int lut_idx;
  if (start_new_motion) {
    start_new_motion = 0;
    exec_transition(lut_standby, 0, lut, 0);
  }

  for (lut_idx = 0; lut_idx < lut_size; lut_idx++) {
    for (int leg_idx = 0; leg_idx < 3; leg_idx++) {
      for (int joint_idx = 0; joint_idx < 3; joint_idx++) {
        right_pwm.setPWM(right_legs[leg_idx][joint_idx], 0,
                         lut[lut_idx][leg_idx][joint_idx] +
                         right_offset_ticks[leg_idx][joint_idx]);
        left_pwm.setPWM(left_legs[leg_idx][joint_idx], 0,
                        lut[lut_idx][leg_idx + 3][joint_idx] +
                        left_offset_ticks[leg_idx][joint_idx]);
      }
    }

    if (current_mode == MotionMode::Mode_Fast_Forward ||
        current_mode == MotionMode::Mode_Fast_Backward) {
      if (lut_idx % 28 == 0 && current_mode != next_motion) {
        exec_transition(lut, lut_idx, lut_standby, 0);
        delay(15);
        break;
      }
    } else {
      if (lut_idx % 14 == 0 && current_mode != next_motion) {
        exec_transition(lut, lut_idx, lut_standby, 0);
        delay(15);
        break;
      }
    }
    delay(15);
  }
}

void exec_transition(int start_pos[][6][3], int start_pos_idx,
                     int end_pos[][6][3], int end_pos_idx) {
  int max_step = 0;
  int sign[6][3];

  int temp_current[6][3];
  int diff;

  int p_count = 6;

  for (int l_idx = 0; l_idx < 6; l_idx++) {
    for (int j_idx = 0; j_idx < 3; j_idx++) {
      diff = end_pos[end_pos_idx][l_idx][j_idx] -
             start_pos[start_pos_idx][l_idx][j_idx];
      temp_current[l_idx][j_idx] = start_pos[start_pos_idx][l_idx][j_idx];
      if (diff < 0) {
        sign[l_idx][j_idx] = -p_count;
      } else {
        sign[l_idx][j_idx] = p_count;
      }
      max_step = max(max_step, abs(diff));
    }
  }
  max_step = ceil(max_step / p_count);
  for (int step_idx = 0; step_idx < max_step; step_idx++) {
    for (int leg_idx = 0; leg_idx < 3; leg_idx++) {
      for (int joint_idx = 0; joint_idx < 3; joint_idx++) {
        if (abs(temp_current[leg_idx][joint_idx] -
                end_pos[end_pos_idx][leg_idx][joint_idx]) > p_count) {
          temp_current[leg_idx][joint_idx] =
            temp_current[leg_idx][joint_idx] + sign[leg_idx][joint_idx];
        } else {
          temp_current[leg_idx][joint_idx] =
            end_pos[end_pos_idx][leg_idx][joint_idx];
        }

        if (abs(temp_current[leg_idx + 3][joint_idx] -
                end_pos[end_pos_idx][leg_idx + 3][joint_idx]) > p_count) {
          temp_current[leg_idx + 3][joint_idx] =
            temp_current[leg_idx + 3][joint_idx] +
            sign[leg_idx + 3][joint_idx];
        } else {
          temp_current[leg_idx + 3][joint_idx] =
            end_pos[end_pos_idx][leg_idx + 3][joint_idx];
        }

        right_pwm.setPWM(right_legs[leg_idx][joint_idx], 0,
                         temp_current[leg_idx][joint_idx] +
                         right_offset_ticks[leg_idx][joint_idx]);
        left_pwm.setPWM(left_legs[leg_idx][joint_idx], 0,
                        temp_current[leg_idx + 3][joint_idx] +
                        left_offset_ticks[leg_idx][joint_idx]);
      }
    }
  }
}
