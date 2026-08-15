# Hexapod ESP32 Firmware

Arduino-based firmware for the ESP32-powered hexapod robot controller. Supports WiFi control via UDP commands, OTA updates, and 18-servo coordination through dual PCA9685 PWM drivers.

## Features

- **WiFi Control**: Access Point mode with UDP command interface
- **Web Calibration Interface**: Browser-based UI to adjust and save servo offsets to EEPROM
- **Binary UDP Protocol**: Fast and efficient binary packet structure for motion control
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
| EEPROM | Calibration storage | Included with arduino-esp32 |
| WebServer | Calibration UI | Included with arduino-esp32 |

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

## How to Calibrate

The firmware includes a web-based calibration interface to easily adjust servo offsets without recompiling code. These offsets are saved directly to the ESP32's EEPROM.

![Calibration Interface](https://raw.githubusercontent.com/rookidroid/hexapod/refs/heads/mochi/images/calibration_page.jpg)

### Calibration Steps:

1. Power on the hexapod and connect your device to its WiFi network (`hexapod`).
2. Open a web browser and navigate to `http://192.168.4.1/`.
3. Click the **Enter Calibration Mode** button. The robot will move to its neutral calibration posture.
4. Use the `+` and `-` buttons for each joint on the web interface to fine-tune the positions.
   - The goal is to align the legs such that the coxa (shoulder) is parallel to the body, the femur (thigh) is horizontal, and the tibia (calf) is vertical.
   - Adjust the offsets until all legs are perfectly aligned and the robot stands evenly.
5. Once satisfied with the posture, click **Save Offsets**. This will permanently save the calibration values to the EEPROM.

*Note: You no longer need to manually edit offsets in `config.h`. If you wish to backup your offsets, the web interface will print the configured arrays to the Serial Monitor when you click save.*

## UDP Command Reference

Three binary packet types share port 1234. The **first byte selects the
protocol**, so the firmware dispatches on the magic number before it checks the
length (the motion and session packets are both 6 bytes).

| Magic | Packet | Size | Purpose |
|-------|--------|------|---------|
| `0xA5` | Motion command | 6 B | Play one of the built-in gait LUTs |
| `0xA6` | Real-time pose | 44 B | Stream raw servo positions for all 18 joints |
| `0xA7` | Session control | 6 B | Enter/leave real-time mode, relax, keep-alive |

All packets are little-endian and unpadded (`#pragma pack(1)`).

### Motion command (`0xA5`)

- **Byte 0**: Magic number (`0xA5`)
- **Byte 1**: Command ID (see enum below)
- **Bytes 2-5**: Sequence number (32-bit unsigned integer)

| Command ID | Action |
|------------|--------|
| 0 | Standby |
| 1 | Walk forward |
| 2 | Walk backward |
| 3-5 | Walk right (45°/90°/135°) |
| 6-8 | Walk left (45°/90°/135°) |
| 9-10 | Fast walk (forward/backward) |
| 11-12 | Rotate in place (left/right) |
| 13-14 | Climbing gait (forward/backward) |
| 15-17 | Body rotation (pitch/roll/yaw) |
| 18 | Body twist motion |

#### Example (Python)

```python
import socket
import struct

# 0xA5 (magic), 1 (Walk forward), 0 (sequence)
packet = struct.pack("<BB I", 0xA5, 1, 0)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(packet, ("192.168.4.1", 1234))
```

### Real-time pose (`0xA6`)

Streams servo positions directly, bypassing the motion LUTs. Used by the
[hexapod-robot-simulator](https://github.com/rookidroid/hexapod-robot-simulator)
to drive the robot live, from a single joint up to a whole-body pose.

| Offset | Type | Field | Notes |
|--------|------|-------|-------|
| 0 | `uint8` | magic | `0xA6` |
| 1 | `uint8` | flags | bit 0: snap immediately instead of obeying the slew limit |
| 2 | `uint16` | max_step | Per-joint slew limit in ticks per 20 ms cycle (0 = default, 8) |
| 4 | `uint32` | seq_num | Sequence number |
| 8 | `int16[6][3]` | ticks | Servo ticks, leg-major |

Leg order matches the motion LUTs: right front, right middle, right back, left
front, left middle, left back. Joint order is coxa, femur, tibia. Ticks use the
same scale as the LUTs (`SERVOMIN` 102 … `SERVOMAX` 512, mid 307), before
calibration: the saved per-joint offset is added on the robot, exactly as it is
for LUT playback. Ticks are clamped on arrival to the window that stays within
`SERVOMIN`…`SERVOMAX` after that offset, so calibration never costs a joint any
travel.

Receiving a pose packet implicitly enters real-time mode from the standby
posture. Each control cycle every joint moves toward its target by at most
`max_step` ticks, so a large jump becomes a controlled slew rather than a step
input to 18 servos at once.

**Failsafe:** if no packet arrives for 1 s the robot eases back to standby at
the slew limit and returns to LUT control. Send a keep-alive (below) while idle.

### Session control (`0xA7`)

| Offset | Type | Field |
|--------|------|-------|
| 0 | `uint8` | magic (`0xA7`) |
| 1 | `uint8` | action |
| 2 | `uint32` | seq_num |

| Action | Name | Effect |
|--------|------|--------|
| 0 | Exit | Leave real-time mode, resume LUT playback from standby |
| 1 | Enter | Enter real-time mode, holding the standby posture |
| 2 | Relax | Disable both PWM drivers so the servos go limp |
| 3 | Ping | Keep-alive; resets the failsafe timer |

Sending a motion command (`0xA5`) also leaves real-time mode, so the two control
styles cannot fight over the servos.

#### Example (Python)

```python
import socket
import struct

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
addr = ("192.168.4.1", 1234)

# Enter real-time mode
sock.sendto(struct.pack("<BBI", 0xA7, 1, 0), addr)

# Hold the standby posture (matches lut_standby)
ticks = [307, 239, 273] * 3 + [307, 375, 341] * 3
sock.sendto(struct.pack("<BBHI" + "h" * 18, 0xA6, 0, 8, 1, *ticks), addr)
```

> **Note:** unlike motion commands, pose packets are not echoed or logged to
> serial. At 50 Hz the logging would saturate the serial port and stall the UDP
> task.

## OTA Updates

After initial USB upload, use OTA for wireless updates:

1. Power on robot and connect to its WiFi
2. In Arduino IDE: **Tools → Port → Network Ports → hexapod**
3. Upload as normal
4. Note: OTA is disabled after first motion command (reboot to re-enable)

## File Structure

```text
hexapod_esp32/
├── hexapod_esp32.ino    # setup() / loop() and the shared system state
├── motion_control.ino   # PWM drivers, LUT playback, every write to a servo
├── realtime.ino         # Real-time pose streaming and its slew limiter
├── network.ino          # WiFi AP, OTA, UDP endpoint and packet parsing
├── calibration.ino      # Servo offsets loaded from / saved to EEPROM
├── web_ui.ino           # HTTP routes for the calibration interface
├── hexapod.h            # Shared state and module interfaces
├── protocol.h           # UDP packet layouts and magic numbers
├── web_page.h           # Calibration page served from flash
├── config.h             # Hardware config, pin mappings, calibration
├── motion.h             # Motion lookup tables (generated from path_tool)
└── README.md            # This file
```

The `.ino` files are Arduino sketch tabs: the build concatenates them into one
translation unit, so the `static` tables in `config.h` and `motion.h` exist once
and every module sees the same calibration offsets.

## Configuration Parameters

Key constants in `config.h`:

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
