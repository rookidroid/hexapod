# Hexapod Pico

This folder contains the Arduino code for Raspberry Pi Pico W or 2W.

## Dependencies

The following Arduino libraries are required:

- [arduino-pico](https://github.com/earlephilhower/arduino-pico)

## Network Configuration

The Pico creates a WiFi access point with these default settings in `config.h`:

- SSID: "hexapod"
- Password: "hexapod_1234"
- UDP Port: 1234

## Programming Steps

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Install [arduino-pico](https://github.com/earlephilhower/arduino-pico)
3. Build and Upload the Code:
   - Select "Raspberry Pi Pico W" or "Raspberry Pi Pico 2W" board in Arduino IDE
   - Upload the code to Pico
   - The robot will perform a boot-up motion sequence

## OTA Updates

The code supports Over-The-Air (OTA) updates:

1. Power on the robot
2. Connect to the hexapod's WiFi network
3. Use Arduino IDE's network port to upload new firmware

**On Arduino IDE under `Tools`, make sure `Flash Size ... FS: 1MB`**

## File Structure

- `hexapod_esp32.ino`: Main Arduino sketch
- `config.h`: Configuration parameters and pin mappings
- `motion.h`: Motion path lookup tables for different movements
- `PicoPWM.h`, `PicoPWM.cpp`: Pico's PWM class
