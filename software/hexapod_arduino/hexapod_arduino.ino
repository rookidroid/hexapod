#include <Adafruit_PWMServoDriver.h>
#include <ESP32Servo.h>
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
  MOde_Twist,
};

MotionMode motion_mode;

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
          }

          inputString = "";
        }
      }
    });
  }
}

void loop() {
  //  posture_calibration();
  //  posture_standby();
  motion_walk();
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

void motion_walk() {
  for (int p_idx = 0; p_idx < path_walk_length; p_idx++) {
    for (int leg_idx = 0; leg_idx < 3; leg_idx++) {
      for (int joint_idx = 0; joint_idx < 3; joint_idx++) {
        right_pwm.setPWM(right_legs[leg_idx][joint_idx], 0,
                         path_walk[p_idx][leg_idx][joint_idx] +
                         right_offset_ticks[leg_idx][joint_idx]);
        left_pwm.setPWM(left_legs[leg_idx][joint_idx], 0,
                        path_walk[p_idx][leg_idx + 3][joint_idx] +
                        left_offset_ticks[leg_idx][joint_idx]);
      }
    }
    delay(10);
  }
}
