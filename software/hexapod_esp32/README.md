# Hexapod ESP32 Firmware

Arduino-based firmware for the ESP32-powered hexapod robot controller. Supports WiFi control via UDP commands, OTA updates, and 18-servo coordination through dual PCA9685 PWM drivers.

## Features

- **WiFi Control**: Access Point mode with UDP command interface
- **18-Servo Control**: Dual PCA9685 PWM drivers (I2C) for coordinated leg movement
- **OTA Updates**: Wireless firmware updates over WiFi
- **Motion Library**: Pre-programmed gaits and movements
- **Event-Driven Boot**: Automatic boot sequence when client connects

## Hardware Requirements

- ESP32 Dev Module
- [Hexapod Controller Board (ESP32)](https://rookidroid.com/product/hexapod-controller-board-esp32/) - ESP32-based controller with dual PCA9685 PWM drivers
- 18× Servos (3 per leg × 6 legs)

## Dependencies

Install these libraries through Arduino Library Manager:

| Library | Purpose | Link |
|---------|---------|------|
| arduino-esp32 | ESP32 board support | [GitHub](https://github.com/espressif/arduino-esp32) |
| Adafruit_PWMServoDriver | PCA9685 control | [Arduino Library](https://docs.arduino.cc/libraries/adafruit-pwm-servo-driver-library/) |
| AsyncUDP | Non-blocking UDP | Included with arduino-esp32 |
| ArduinoOTA | OTA updates | Included with arduino-esp32 |

## Quick Start

### 1. Setup Arduino IDE

1. Install [Arduino IDE 2.x](https://www.arduino.cc/en/software)
2. Install ESP32 board support following [this guide](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
3. Install required libraries via Library Manager (Sketch → Include Library → Manage Libraries)

### 2. Configure Hardware

Edit `config.h` to match your hardware:

```cpp
// Servo pin mappings
static int left_legs[3][3] = {{1, 2, 3}, {5, 6, 7}, {9, 8, 10}};
static int right_legs[3][3] = {{10, 9, 8}, {13, 14, 15}, {7, 6, 5}};

// Calibration offsets (adjust to compensate for mechanical assembly)
static int left_offset_ticks[3][3] = {{-10, 0, 0}, {5, -5, 0}, {5, 15, 0}};
static int right_offset_ticks[3][3] = {{5, -10, 0}, {0, 0, 5}, {15, 0, 5}};

// WiFi credentials
#define APSSID "hexapod"
#define APPSK "hexapod_1234"
```

### 3. Upload Firmware

1. Connect ESP32 via USB
2. Select **Tools → Board → ESP32 Dev Module**
3. Select correct COM port
4. Click **Upload**
5. Monitor Serial output (115200 baud) for IP address

### 4. Control the Robot

1. Connect to WiFi network `hexapod` (password: `hexapod_1234`)
2. Robot will automatically perform boot sequence when client connects
3. Send UDP commands to `192.168.4.1:1234`

## UDP Command Reference

Send ASCII commands via UDP to control the robot. Commands should be wrapped with `:` delimiters (e.g., `:walk0:`):

| Command | Description |
|---------|-------------|
| `:standby:` | Stop and hold position |
| `:walk0:` | Walk forward |
| `:walk180:` | Walk backward |
| `:walkr45:` / `:walkr90:` / `:walkr135:` | Walk right (45°/90°/135°) |
| `:walkl45:` / `:walkl90:` / `:walkl135:` | Walk left (45°/90°/135°) |
| `:fastforward:` / `:fastbackward:` | Fast walk |
| `:turnleft:` / `:turnright:` | Rotate in place |
| `:climbforward:` / `:climbbackward:` | Climbing gait |
| `:rotatex:` / `:rotatey:` / `:rotatez:` | Body rotation (pitch/roll/yaw) |
| `:twist:` | Body twist motion |

### Example (Python)

```python
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(b":walk0:", ("192.168.4.1", 1234))
```

## OTA Updates

After initial USB upload, use OTA for wireless updates:

1. Power on robot and connect to its WiFi
2. In Arduino IDE: **Tools → Port → Network Ports → hexapod**
3. Upload as normal
4. Note: OTA is disabled after first motion command (reboot to re-enable)

## File Structure

```text
hexapod_esp32/
├── hexapod_esp32.ino    # Main firmware (WiFi, OTA, motion control)
├── config.h             # Hardware config, pin mappings, calibration
├── motion.h             # Motion lookup tables (generated from path_tool)
└── README.md            # This file
```

## Configuration Parameters

Key constants in `hexapod_esp32.ino`:

```cpp
// Servo timing
const uint16_t SERVO_INIT_DELAY_MS = 50;    // Boot sequence delay
const uint8_t TRANSITION_TICK_STEP = 6;      // Transition smoothness

// Hardware
const uint8_t LEFT_PWM_ADDRESS = 0x40;       // Left PCA9685 I2C address
const uint8_t RIGHT_PWM_ADDRESS = 0x41;      // Right PCA9685 I2C address
const uint8_t LEFT_PWM_ENABLE_PIN = 19;      // Left driver enable (active LOW)
const uint8_t RIGHT_PWM_ENABLE_PIN = 26;     // Right driver enable (active LOW)
```

## Troubleshooting

### WiFi connection issues

- Verify WiFi credentials in `config.h`
- Check serial monitor for IP address (192.168.4.1 default)
- Ensure client device supports 2.4GHz WiFi

### Servos not moving

- Check PWM driver I2C addresses match hardware (0x40, 0x41)
- Verify enable pins are connected correctly (active LOW)
- Confirm servo pin mappings in `config.h`
- Test individual servos with `posture_calibration()` function

### OTA not visible

- OTA only works before first motion command
- Reboot robot to re-enable OTA
- Ensure connected to robot's WiFi network
- Check firewall settings on upload computer

## Adding Custom Motions

1. Generate motion LUT using `path_tool` (see `../path_tool/`)
2. Add LUT arrays to `motion.h`
3. Add command mapping to `motion_config[]` in `hexapod_esp32.ino`:

```cpp
static const MotionConfig motion_config[] = {
  // ... existing motions ...
  { "mymotion", lut_mymotion_length, lut_mymotion }
};
```

## License

Copyright (C) 2024 - PRESENT rookidroid.com

## Support

- Website: [rookidroid.com](https://rookidroid.com/)
- Email: [info@rookidroid.com](mailto:info@rookidroid.com)
