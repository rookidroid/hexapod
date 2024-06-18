#include <Adafruit_PWMServoDriver.h>
#include <ESP32Servo.h>
#include <Wire.h>

Adafruit_PWMServoDriver left_pwm = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver right_pwm = Adafruit_PWMServoDriver(0x41);

#define SERVOMIN 125  // Minimum value, 0 deg
#define SERVOMID 350  // Middle value, 90 deg
#define SERVOMAX 575  // Maximum value, 180 deg

int left_legs[3][3] = {{0, 1, 3}, {5, 7, 10}, {12, 14, 15}};
int right_legs[3][3] = {{0, 1, 2}, {5, 8, 9}, {12, 14, 15}};

// Offset to correct the installation error. Offset value is the number of ticks
int left_offset_ticks[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
int right_offset_ticks[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

int left_flip[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
int right_flip[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};

int pulselength;

void setup() {
  Serial.begin(115200);

  // Initialize the PCA9685 library
  left_pwm.begin();
  left_pwm.setPWMFreq(60);  // Set the PWM frequency of the PCA9685

  right_pwm.begin();
  right_pwm.setPWMFreq(60);  // Set the PWM frequency of the PCA9685
}

void loop() {
  // pulselength = map(90, 0, 180, SERVOMIN, SERVOMAX);

  posture_calibration();
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
