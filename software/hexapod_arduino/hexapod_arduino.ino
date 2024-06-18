#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver left_pwm = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver right_pwm = Adafruit_PWMServoDriver(0x41);

#define SERVOMIN  125  // Minimum value - previously 80
#define SERVOMAX  575  // Maximum value - previously 1024

int pulselength;


void setup()
{

  Serial.begin(115200);

  // Initialize the PCA9685 library
  left_pwm.begin();
  left_pwm.setPWMFreq(60); // Set the PWM frequency of the PCA9685 (you may need to adjust this)

  right_pwm.begin();
  right_pwm.setPWMFreq(60); // Set the PWM frequency of the PCA9685 (you may need to adjust this)

//  pulselength = map(90, 0, 180, SERVOMIN, SERVOMAX);
//
//  left_pwm.setPWM(0, 0, pulselength);

  // Initialize all servos
//  for (int i = 0; i < 4; i++)
//  {
//    for (int j = 0; j < 3; j++)
//    {
//      // Attach servo to PCA9685 channel
//      pwm.setPWM(servo_pins[i][j],0,0); // Initialize servos at their center position
//    }
//  }
//  center_servos();
//  Serial.println("center");
//  delay(2000);
}

void loop()
{
  pulselength = map(90, 0, 180, SERVOMIN, SERVOMAX);
  left_pwm.setPWM(0, 0, pulselength);
  left_pwm.setPWM(1, 0, pulselength);
  left_pwm.setPWM(3, 0, pulselength);

  left_pwm.setPWM(5, 0, pulselength);
  left_pwm.setPWM(7, 0, pulselength);
  left_pwm.setPWM(10, 0, pulselength);

  left_pwm.setPWM(12, 0, pulselength);
  left_pwm.setPWM(14, 0, pulselength);
  left_pwm.setPWM(15, 0, pulselength);

  right_pwm.setPWM(0, 0, pulselength);
  right_pwm.setPWM(1, 0, pulselength);
  right_pwm.setPWM(2, 0, pulselength);

  right_pwm.setPWM(5, 0, pulselength);
  right_pwm.setPWM(8, 0, pulselength);
  right_pwm.setPWM(9, 0, pulselength);

  right_pwm.setPWM(12, 0, pulselength);
  right_pwm.setPWM(14, 0, pulselength);
  right_pwm.setPWM(15, 0, pulselength);
}
