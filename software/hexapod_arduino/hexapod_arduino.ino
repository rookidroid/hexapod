#include <Adafruit_PWMServoDriver.h>
//#include <ESP32Servo.h>
#include <WiFi.h>
#include <Wire.h>

#include "AsyncUDP.h"
#include "motion.h"
#include "posture.h"

#ifndef APSSID
#define APSSID "hexapod"
#define APPSK "hexapod_1234"
#endif
const char *ssid = APSSID;
const char *password = APPSK;

char *packet_ptr;               // buffer to hold incoming packet
unsigned int localPort = 1234;  // local port to listen on

AsyncUDP Udp;

Adafruit_PWMServoDriver left_pwm = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver right_pwm = Adafruit_PWMServoDriver(0x41);

int str_idx;
String inputString = "";

#define SERVOMIN 125  // Minimum value, 0 deg
#define SERVOMID 350  // Middle value, 90 deg
#define SERVOMAX 575  // Maximum value, 180 deg

int left_legs[3][3] = {{0, 3, 1}, {7, 5, 10}, {15, 12, 14}};
int right_legs[3][3] = {{15, 12, 14}, {8, 9, 5}, {0, 2, 1}};

// Offset to correct the installation error. Offset value is the number of ticks
int left_offset_ticks[3][3] = {{ -5, -18, 6}, { -15, 10, 14}, {20, 6, 0}};
int right_offset_ticks[3][3] = {{20, 16, 0}, { -15, 20, -15}, { -10, -8, 10}};

enum MotionMode {
  Mode_Standby,
  Mode_Laydown,
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

MotionMode motion_mode = MotionMode::Mode_Standby;

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);

  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  //  Udp.begin(localPort);

  // Initialize the PCA9685 library
  left_pwm.begin();
  left_pwm.setPWMFreq(60);  // Set the PWM frequency of the PCA9685

  right_pwm.begin();
  right_pwm.setPWMFreq(60);  // Set the PWM frequency of the PCA9685

  posture_standby();

  if (Udp.listen(localPort)) {
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
      packet_ptr = (char *)packet.data();
      for (str_idx = 0; str_idx < packet.length(); str_idx++) {
        char inChar = packet_ptr[str_idx];
        //      Serial.print(inChar);
        //      Serial.print("\n");

        if (inChar != '\n' && inChar != ':') {
          // add it to the inputString:
          inputString += inChar;
        } else if (inChar == ':') {
          if (inputString == String("standby")) {
            motion_mode = MotionMode::Mode_Standby;
          } else if (inputString == String("walk0")) {
            motion_mode = MotionMode::Mode_Walk_0;
          } else if (inputString == String("walk180")) {
            motion_mode = MotionMode::Mode_Walk_180;
          } else if (inputString == String("walkr45")) {
            motion_mode = MotionMode::Mode_Walk_R45;
          } else if (inputString == String("walkr90")) {
            motion_mode = MotionMode::Mode_Walk_R90;
          } else if (inputString == String("walkr135")) {
            motion_mode = MotionMode::Mode_Walk_R135;
          } else if (inputString == String("walkl45")) {
            motion_mode = MotionMode::Mode_Walk_L45;
          } else if (inputString == String("walkl90")) {
            motion_mode = MotionMode::Mode_Walk_L90;
          } else if (inputString == String("walkl135")) {
            motion_mode = MotionMode::Mode_Walk_L135;
          } else if (inputString == String("fastforward")) {
            motion_mode = MotionMode::Mode_Fast_Forward;
          } else if (inputString == String("fastbackward")) {
            motion_mode = MotionMode::Mode_Fast_Backward;
          } else if (inputString == String("turnleft")) {
            motion_mode = MotionMode::Mode_Turn_Left;
          } else if (inputString == String("turnright")) {
            motion_mode = MotionMode::Mode_Turn_Right;
          } else if (inputString == String("climbforward")) {
            motion_mode = MotionMode::Mode_Climb_Forward;
          } else if (inputString == String("climbbackward")) {
            motion_mode = MotionMode::Mode_Climb_Backward;
          } else if (inputString == String("rotatex")) {
            motion_mode = MotionMode::Mode_Rotate_X;
          } else if (inputString == String("rotatey")) {
            motion_mode = MotionMode::Mode_Rotate_Y;
          } else if (inputString == String("rotatez")) {
            motion_mode = MotionMode::Mode_Rotate_Z;
          } else if (inputString == String("twist")) {
            motion_mode = MotionMode::Mode_Twist;
          }

          inputString = "";
        }
      }
    });
  }
}

void loop() {
  if (motion_mode == MotionMode::Mode_Walk_0) {
    exec_motion(lut_walk_0_length, lut_walk_0);
  } else if (motion_mode == MotionMode::Mode_Walk_180) {
    exec_motion(lut_walk_180_length, lut_walk_180);
  } else if (motion_mode == MotionMode::Mode_Walk_R45) {
    exec_motion(lut_walk_r45_length, lut_walk_r45);
  } else if (motion_mode == MotionMode::Mode_Walk_R90) {
    exec_motion(lut_walk_r90_length, lut_walk_r90);
  } else if (motion_mode == MotionMode::Mode_Walk_R135) {
    exec_motion(lut_walk_r135_length, lut_walk_r135);
  } else if (motion_mode == MotionMode::Mode_Walk_L45) {
    exec_motion(lut_walk_l45_length, lut_walk_l45);
  } else if (motion_mode == MotionMode::Mode_Walk_L90) {
    exec_motion(lut_walk_l90_length, lut_walk_l90);
  } else if (motion_mode == MotionMode::Mode_Walk_L135) {
    exec_motion(lut_walk_l135_length, lut_walk_l135);
  } else if (motion_mode == MotionMode::Mode_Fast_Forward) {
    exec_motion(lut_fast_forward_length, lut_fast_forward);
  } else if (motion_mode == MotionMode::Mode_Fast_Backward) {
    exec_motion(lut_fast_backward_length, lut_fast_backward);
  } else if (motion_mode == MotionMode::Mode_Turn_Left) {
    exec_motion(lut_turn_left_length, lut_turn_left);
  } else if (motion_mode == MotionMode::Mode_Turn_Right) {
    exec_motion(lut_turn_right_length, lut_turn_right);
  } else if (motion_mode == MotionMode::Mode_Climb_Forward) {
    exec_motion(lut_climb_forward_length, lut_climb_forward);
  } else if (motion_mode == MotionMode::Mode_Climb_Backward) {
    exec_motion(lut_climb_backward_length, lut_climb_backward);
  } else if (motion_mode == MotionMode::Mode_Rotate_X) {
    exec_motion(lut_rotate_x_length, lut_rotate_x);
  } else if (motion_mode == MotionMode::Mode_Rotate_Y) {
    exec_motion(lut_rotate_y_length, lut_rotate_y);
  } else if (motion_mode == MotionMode::Mode_Rotate_Z) {
    exec_motion(lut_rotate_z_length, lut_rotate_z);
  } else if (motion_mode == MotionMode::Mode_Twist) {
    exec_motion(lut_twist_length, lut_twist);
  } else {
    posture_standby();
  }
  //  exec_motion(path_walk_length, path_walk);
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

void posture_standby() {
  for (int leg_idx = 0; leg_idx < 3; leg_idx++) {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++) {
      right_pwm.setPWM(right_legs[leg_idx][joint_idx], 0,
                       pos_standby[leg_idx][joint_idx] +
                       right_offset_ticks[leg_idx][joint_idx]);
      left_pwm.setPWM(left_legs[leg_idx][joint_idx], 0,
                      pos_standby[leg_idx + 3][joint_idx] +
                      left_offset_ticks[leg_idx][joint_idx]);
    }
  }
}

void exec_motion(int lut_size, int lut[][6][3]) {
  MotionMode current_mode = motion_mode;
  for (int lut_idx = 0; lut_idx < lut_size; lut_idx++) {
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
    if (current_mode != motion_mode)
    {
      posture_standby();
      delay(10);
      break;
    }
    delay(10);
  }
}
