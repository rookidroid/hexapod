# Hexapod ESP32

This folder contains the Arduino code for the ESP32 microcontroller.

## Dependencies

The following Arduino libraries are required:

- [arduino-esp32](https://github.com/espressif/arduino-esp32)
- [Adafruit PWM Servo Driver Library](https://docs.arduino.cc/libraries/adafruit-pwm-servo-driver-library/#Releases)

## Network Configuration

The ESP32 creates a WiFi access point with these default settings in `config.h`:

- SSID: "hexapod"
- Password: "hexapod_1234"
- UDP Port: 1234

## Programming Steps

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Install [arduino-esp32](https://github.com/espressif/arduino-esp32) following the [Instruction](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
3. Install Dependencies:
   - Install all required libraries through Arduino Library Manager
4. Build and Upload the Code:
   - Select "ESP32 Dev Module" board in Arduino IDE
   - Upload the code to ESP32
   - The robot will perform a boot-up motion sequence

## OTA Updates

The code supports Over-The-Air (OTA) updates:

1. Power on the robot
2. Connect to the hexapod's WiFi network
3. Use Arduino IDE's network port to upload new firmware

## File Structure

- `hexapod_esp32.ino`: Main Arduino sketch
- `config.h`: Configuration parameters and pin mappings
- `motion.h`: Motion path lookup tables for different movements
