#include <Adafruit_PWMServoDriver.h>
#include <ESP32Servo.h>
#include <Wire.h>

#include <WiFi.h>
#include <WiFiUdp.h>

#include "posture.h"
#include "motion.h"

#ifndef APSSID
#define APSSID "hexapod"
#define APPSK "hexapod_1234"
#endif
const char *ssid = APSSID;
const char *password = APPSK;

char packetBuffer[4096 + 1];   // buffer to hold incoming packet
unsigned int localPort = 1234; // local port to listen on

WiFiUDP Udp;

Adafruit_PWMServoDriver left_pwm = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver right_pwm = Adafruit_PWMServoDriver(0x41);

#define SERVOMIN 125  // Minimum value, 0 deg
#define SERVOMID 350  // Middle value, 90 deg
#define SERVOMAX 575  // Maximum value, 180 deg

int left_legs[3][3] = {{0, 3, 1}, {7, 5, 10}, {15, 12, 14}};
int right_legs[3][3] = {{15, 12, 14}, {8, 9, 5}, {0, 2, 1}};

// Offset to correct the installation error. Offset value is the number of ticks
int left_offset_ticks[3][3] = {{-5, -18, 6}, {-15, 10, 14}, {20, 6, 0}};
int right_offset_ticks[3][3] = {{20, 16, 0}, {-15, 20, -15}, {-10, -8, 10}};

int left_flip[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
int right_flip[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};

int pulselength;

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);

  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  Udp.begin(localPort);

  // Initialize the PCA9685 library
  left_pwm.begin();
  left_pwm.setPWMFreq(60);  // Set the PWM frequency of the PCA9685

  right_pwm.begin();
  right_pwm.setPWMFreq(60);  // Set the PWM frequency of the PCA9685

  posture_standby();
}

void loop() {
//  int packetSize = Udp.parsePacket();
//  if (packetSize)
//  {
//    rover = false;
//    // read the packet into packetBufffer
//    int n = Udp.read(packetBuffer, 4096);
//    packetBuffer[n] = 0;
//
//    for (str_idx = 0; str_idx < n; str_idx++)
//    {
//      char inChar = packetBuffer[str_idx];
//      //      Serial.print(inChar);
//      //      Serial.print("\n");
//
//      if (inChar != '\n' && inChar != ':')
//      {
//        // add it to the inputString:
//        inputString += inChar;
//      }
//      else if (inChar == ':')
//      {
//        if (inputString[0] == 'X')
//        {
//          int sLength = inputString.length();
//          String tempStr = inputString.substring(1, sLength);
//          x_val = tempStr.toInt();
//          Serial.print('X');
//          Serial.print(x_val);
//          Serial.print("\n");
//          rover = true;
//        }
//        else if (inputString[0] == 'Y')
//        {
//          int sLength = inputString.length();
//          String tempStr = inputString.substring(1, sLength);
//          y_val = tempStr.toInt();
//          Serial.print('Y');
//          Serial.print(y_val);
//          Serial.print("\n");
//          rover = true;
//        }
//        else if (inputString[0] == 'L')
//        {
//          int sLength = inputString.length();
//          String tempStr = inputString.substring(1, sLength);
//          left_speed = tempStr.toInt();
//          set_left_motor(left_speed);
//          // rover = true;
//        }
//        else if (inputString[0] == 'R')
//        {
//          int sLength = inputString.length();
//          String tempStr = inputString.substring(1, sLength);
//          right_speed = tempStr.toInt();
//          set_right_motor(right_speed);
//          // rover = true;
//        }
//
//        inputString = "";
//      }
//    }
//  }
  // pulselength = map(90, 0, 180, SERVOMIN, SERVOMAX);

//  posture_calibration();
//  posture_standby();
//  motion_walk();
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

void posture_standby(){
  for (int leg_idx = 0; leg_idx < 3; leg_idx++) {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++) {
      right_pwm.setPWM(right_legs[leg_idx][joint_idx], 0,
                       pos_standby[leg_idx][joint_idx] + right_offset_ticks[leg_idx][joint_idx]);
      left_pwm.setPWM(left_legs[leg_idx][joint_idx], 0,
                      pos_standby[leg_idx+3][joint_idx] + left_offset_ticks[leg_idx][joint_idx]);
    }
  }
}

void motion_walk(){
  for (int p_idx=0; p_idx< path_walk_length; p_idx++){
    for (int leg_idx = 0; leg_idx < 3; leg_idx++) {
      for (int joint_idx = 0; joint_idx < 3; joint_idx++) {
        right_pwm.setPWM(right_legs[leg_idx][joint_idx], 0,
                         path_walk[p_idx][leg_idx][joint_idx] + right_offset_ticks[leg_idx][joint_idx]);
        left_pwm.setPWM(left_legs[leg_idx][joint_idx], 0,
                        path_walk[p_idx][leg_idx+3][joint_idx] + left_offset_ticks[leg_idx][joint_idx]);
      }
    }
    delay(10);
  }
}
