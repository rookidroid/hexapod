# <img src="./images/hexapod-logo.svg" alt="logo" width="128"/> Hexapod

A 3D Printed Hexapod Robot

<img src="./images/hexapod_photo.jpg" alt="hexapod_photo" width="400"/><img src="./images/hexapod_photo_battery.jpg" alt="hexapod_photo" width="400"/>

## Introduction

This agile, 3D-printed hexapod robot is designed to work with an ESP32, providing flexibility and enhanced performance. Equipped with stronger 25kg servos, it offers a range of advanced features, including:

- **Robust, durable structure**: 3D-printed parts designed for strength and easy assembly
- **WiFi-enabled remote control**: Control your hexapod wirelessly from your smartphone or computer
- **Smooth, agile movement**: Advanced motion algorithms for natural walking patterns
- **Web-based calibration interface**: Easy servo calibration through your browser with real-time adjustment
- **Over-the-air (OTA) firmware updates**: Update firmware without cables for easy maintenance

### What You'll Build

This project will guide you through building a fully functional hexapod robot with 18 degrees of freedom (3 joints per leg × 6 legs). The complete build typically takes 8-12 hours, including 3D printing, assembly, and calibration.

### Skill Level

- **Beginner-Intermediate**: Basic soldering and mechanical assembly skills required
- **Tools needed**: Soldering iron, hex key set, wire cutters/strippers

## Bill of Materials (BOM)

### Electronics Components

| Name                 | Thumbnail                                                                                                                                                    | Required # | Specifications | Note                                                                                                                                                                          |
| -------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ | ---------- | -------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Controller Board     | <img src="./images/controller_esp32.png" alt="controller_esp32" width="300"/> | 1          | ESP32 | Purchase [ESP32 version](https://rookidroid.com/product/hexapod-controller-board-esp32/) |
| 25kg Servo            | <img src="./images/25kg_servo.jpg" alt="25kg_servo" width="200"/>                                                                                          | 18         | 180° rotation | Ensure all servos are from the same batch for consistency                                                                                                                                                  |
| Toggle Switch        | <img src="./images/switch.jpg" alt="switch" width="300"/>                                                                                                    | 1          | SPST, 6mm diameter |                                                                                                                                                                               |
| 18650 Battery        | <img src="./images/battery.jpg" alt="battery" width="300"/>                                                                                                  | 4          | 3.7V Li-ion, 2000mAh+ recommended | **Use protected batteries for safety**                                                                                                                                                                               |
| 18650 Battery Holder | <img src="./images/battery_holder.jpg" alt="batter_holder" width="300"/>                                                                                     | 1          | 4-cell holder with wire leads |                                                                                                                                                                               |

### Connection Diagram

#### ESP32

![diagram_esp32](./images/pcb_diagram_esp32.jpg)

## Assembly Instructions

### Overview

Follow these steps in order for the best results:

1. **3D Print all parts** (see parts list below)
2. **Prepare hardware** (organize screws, nuts, bearings, and pins)
3. **Assemble body** (install servos and controller)
4. **Build legs** (joint → leg → foot for each leg)
5. **Attach legs to body**
6. **Wire and test** (connect servos, upload firmware, calibrate)

**⚠️ Important:** Do NOT tighten the servo horn screws during assembly! Leave them loose to allow for rotation. You'll tighten them properly during the calibration process after adjusting each servo to its correct neutral position.

![whole_assembly](./images/assembly_whole.gif)

### Step 1: 3D-Printed Parts

**Print Settings Recommendations:**

- Layer height: 0.2mm
- Infill: 20-30%
- Material: PLA or PETG
- Supports: Required for some parts (check STL orientation)

#### Step 1.1: Body Components (x1 complete body)

**Assembly Tips:**

- Install servos before closing the body
- Route wires neatly to avoid pinching
- Test fit all parts before final assembly

![body_assembly](./images/assembly_body.gif)

| Filename         | Thumbnail                                                                     | Required # |
| ---------------- | ----------------------------------------------------------------------------- | ---------- |
| body_base        | <img src="./images/body_base.jpg" alt="body_base" width="400"/>               | 1          |
| body_side        | <img src="./images/body_side.jpg" alt="body_side" width="400"/>               | 6          |
| body_top         | <img src="./images/body_top.jpg" alt="body_top" width="400"/>                 | 1          |
| body_head   | <img src="./images/body_head.jpg" alt="body_head" width="400"/>     | 1          |
| body_battery_top     | <img src="./images/body_battery.jpg" alt="body_battery_top" width="400"/>         | 1          |
| body_servo_side | <img src="./images/body_servo_side.jpg" alt="body_servo_side" width="400"/> | 12          |

#### Step 1.2: Joint Components (x6 total: 3 standard + 3 mirrored)

**Important:** Pay attention to joint orientation! Three legs use standard joints, three use mirrored joints.

- **Left side legs**: Use mirrored joints
- **Right side legs**: Use standard joints

_Refer to the fully assembled robot images for correct joint orientations_

![joint_assembly](./images/assembly_joint.gif)

| Filename     | Thumbnail                                                             | Required # |
| ------------ | --------------------------------------------------------------------- | ---------- |
| joint_bottom | <img src="./images/joint_bottom.jpg" alt="joint_bottom" width="400"/> | 12         |
| joint_cross  | <img src="./images/joint_cross.jpg" alt="joint_cross" width="400"/>   | 6          |
| joint_top    | <img src="./images/joint_top.jpg" alt="joint_top" width="400"/>       | 12         |

#### Step 1.3: Leg Components (x6)

**Assembly Notes:**

- Ensure bearings are properly seated
- Pins should slide in smoothly without forcing
- Check for smooth joint rotation before proceeding

![leg_assembly](./images/assembly_leg.gif)

| Filename   | Thumbnail                                                         | Required # |
| ---------- | ----------------------------------------------------------------- | ---------- |
| leg_bottom | <img src="./images/leg_bottom.jpg" alt="leg_bottom" width="400"/> | 6          |
| leg_side   | <img src="./images/leg_side.jpg" alt="leg_side" width="400"/>     | 12         |
| leg_top    | <img src="./images/leg_top.jpg" alt="leg_top" width="400"/>       | 6          |

#### Step 1.4: Foot Components (x6 total: 3 standard + 3 mirrored)

**Important:** Match foot orientation with joint orientation!

- **Left side legs**: Use mirrored feet with mirrored joints
- **Right side legs**: Use standard feet with standard joints

_Refer to the fully assembled robot images for correct foot orientations_

![foot_assembly](./images/assembly_foot.gif)

| Filename    | Thumbnail                                                           | Required # | Note                        |
| ----------- | ------------------------------------------------------------------- | ---------- | --------------------------- |
| foot_bottom | <img src="./images/foot_bottom.jpg" alt="foot_bottom" width="400"/> | 6          |                             |
| foot_top    | <img src="./images/foot_top.jpg" alt="foot_top" width="400"/>       | 6          |                             |
| foot_ground | <img src="./images/foot_ground.jpg" alt="foot_ground" width="400"/> | 6          |                             |
| foot_tip    | <img src="./images/foot_tip.jpg" alt="foot_tip" width="400"/>       | 6          | Recommend to print with TPU |

### Step 2: Hardware Components

**Pro Tip:** Organize all hardware into labeled containers before assembly to save time!

| Name      | Spec                                  | Required # | Usage                          |
| --------- | ------------------------------------- | ---------- | ------------------------------ |
| Screw     | M2 × 6mm hex socket                   | 36         | Servo mounting                 |
| Screw     | M2 × 12mm countersunk                 | 180        | General assembly               |
| Nuts      | M2 hex nut                            | 216        | Securing screws                |
| Pin       | M4 × 6mm stainless steel (304)        | 18         | Joint pivots                   |
| Bearing   | MR74-2RS (4mm ID, 7mm OD, 2.5mm Bore) | 18         | Smooth joint rotation          |

**Where to Buy:** These are standard metric hardware available from Amazon, AliExpress, or local hardware stores.

## Software Setup

![hexapod_model_label](./images/hexapod_model_label.svg)

### Prerequisites

- **Arduino IDE** (version 2.x recommended)
- **USB cable** compatible with your controller board
- **Required Libraries** (install via Arduino Library Manager):
  - ESP32: `Adafruit_PWMServoDriver` (for PCA9685 control)
  - ESP32: `AsyncUDP` and `ArduinoOTA` (included with arduino-esp32)

### Step-by-Step Installation

#### ESP32 Setup

1. **Install Arduino IDE** from [arduino.cc](https://www.arduino.cc/en/software)

2. **Add ESP32 Board Support**:
   - Follow the official [ESP32 Arduino installation guide](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
   - Or add to Arduino IDE → File → Preferences → "Additional Board Manager URLs":

     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```

   - Then install via Tools → Board → Boards Manager → Search "ESP32"

3. **Install Required Library**:
   - Go to Sketch → Include Library → Manage Libraries
   - Search for "Adafruit PWM Servo Driver"
   - Click Install

4. **Configure Board Settings**:
   - Board: "ESP32 Dev Module"
   - Upload Speed: "115200"
   - Flash Frequency: "80MHz"
   - Partition Scheme: "Default 4MB with spiffs"

5. **Open and Configure Code**:
   - Open `./software/hexapod_esp32/hexapod_esp32.ino`
   - Edit `config.h` to set your WiFi credentials (default: SSID="hexapod", password="hexapod_1234"):

     ```cpp
     #define APSSID "hexapod"
     #define APPSK "hexapod_1234"
     ```

   - Configure servo pin mappings if using custom wiring:

     ```cpp
     static int left_legs[3][3] = {{1, 2, 3}, {5, 6, 7}, {9, 8, 10}};
     static int right_legs[3][3] = {{10, 9, 8}, {13, 14, 15}, {7, 6, 5}};
     ```

   - Adjust calibration offset values after assembly (see Calibration section)

6. **Upload Firmware**:
   - Connect ESP32 via USB
   - Select correct COM port in Tools → Port
   - Click Upload button
   - Open Serial Monitor (115200 baud) to see the WiFi AP IP address (default: 192.168.4.1)

### Project File Structure

**ESP32 Version** (`./software/hexapod_esp32/`):

- `hexapod_esp32.ino`: Main Arduino sketch with control logic
- `config.h`: WiFi credentials, servo pin mappings, and calibration offsets
- `motion.h`: Pre-generated motion look-up tables for smooth walking
- `README.md`: ESP32-specific documentation

**Path Tool** (`./software/path_tool/`):

- `path_tool.py`: Tool for generating custom walking patterns
- `lut_generator.ipynb`: Jupyter notebook for motion path visualization
- `path_lib.py`: Library for inverse kinematics calculations

### Control Interface

#### Connection

1. **Power on the hexapod** - Connect batteries and turn on
2. **Connect to WiFi** - Join the hexapod's WiFi network:
   - SSID: `hexapod` (default)
   - Password: `hexapod_1234` (default)
   - The hexapod creates its own Access Point
3. **Default IP**: `192.168.4.1`
4. **UDP Port**: `1234`

#### Sending UDP Commands

The hexapod accepts motion commands via UDP packets on port 1234. Commands must be wrapped with `:` delimiters (e.g., `:walk0:`).

**Available Commands:**

- `:standby:` - Stop and hold position
- `:walk0:` - Walk forward
- `:walk180:` - Walk backward
- `:walkr45:` / `:walkr90:` / `:walkr135:` - Walk right at 45°/90°/135°
- `:walkl45:` / `:walkl90:` / `:walkl135:` - Walk left at 45°/90°/135°
- `:turnleft:` / `:turnright:` - Rotate in place
- `:fastforward:` / `:fastbackward:` - Fast walking
- `:climbforward:` / `:climbbackward:` - Climbing gait
- `:rotatex:` / `:rotatey:` / `:rotatez:` - Body rotation (pitch/roll/yaw)
- `:twist:` - Body twist motion

**Example (Python):**

```python
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(b":walk0:", ("192.168.4.1", 1234))
```

**Boot Behavior:** The robot automatically performs a boot sequence (stands up) when a client connects to its WiFi network.

### Over-The-Air (OTA) Updates

ESP32 supports wireless firmware updates:

1. **Power on** the robot and **connect** to its WiFi network
2. In Arduino IDE, go to **Tools → Port → Network Ports**
3. Select the hexapod from network ports
4. Click **Upload** as normal

**Important Notes:**

- For ESP32: OTA is disabled after the first motion command. Reboot the robot to re-enable OTA.

### Troubleshooting

**Upload fails:**

- Check USB cable (must support data transfer)
- Try different USB port
- Ensure correct board and port selected
- For ESP32: Hold BOOT button during upload

**Servos not responding:**

- Verify battery voltage (should be 7.4V nominal for 2S Li-ion setup)
- Check all servo connections match the wiring diagram
- For ESP32: Verify PCA9685 I2C addresses (default: 0x40 left, 0x41 right)
- For ESP32: Check enable pins (GPIO 19 for left, GPIO 26 for right)
- Test individual servos using `posture_calibration()` function
- Confirm servo pin mappings in `config.h` match your wiring

**WiFi connection issues:**

- Verify SSID and password in `config.h` (default: hexapod/hexapod_1234)
- The hexapod creates an **Access Point** - connect to it, don't look for it on your router
- Ensure your device supports 2.4GHz WiFi (5GHz not supported)
- Default IP is always `192.168.4.1` when connected to the hexapod's AP

**OTA not working:**

- For ESP32: OTA only works before the first motion command - reboot to re-enable
- Ensure you're connected to the hexapod's WiFi network
- Check firewall settings on your computer

### Android App

Control your hexapod directly from your Android smartphone:

<a href="https://play.google.com/store/apps/details?id=com.rookiedev.hexapod">
  <img src="https://play.google.com/intl/en_us/badges/static/images/badges/en_badge_web_generic.png" alt="Get it on Google Play" height="80"/>
</a>

The app provides an intuitive interface to:

- Connect to your hexapod's WiFi network
- Control movement with on-screen buttons
- Execute all available motion commands
- Monitor connection status

**Requirements:** Android device with WiFi capability

### Desktop Control Software

_Work in progress - Check repository for updates_

## Calibration Guide

Proper calibration is **critical** for smooth walking. Take your time with this step!

### Understanding Servo Positions

All servos should be at 90° (neutral position) when the legs are in the reference positions shown below.

### Calibration Procedure

The hexapod now includes a **web-based calibration interface** that makes the calibration process much easier - no need to repeatedly edit code and re-upload firmware!

#### Step 1: Access the Calibration Interface

1. **Upload Firmware**: Flash the code to your controller with default offset values
2. **Power On**: Connect batteries and turn on the hexapod
3. **Connect to WiFi**: Join the hexapod's WiFi network (default SSID: `hexapod`)
4. **Open Browser**: Navigate to `http://192.168.4.1`

![Calibration Web Interface](./images/calibration_page.jpg)

#### Step 2: Enter Calibration Mode

1. Click the **"Enter Calibration Mode"** button
   - The hexapod will stop motion execution
   - All current offset values will be loaded
   - A grid showing all 6 legs (18 servos) will appear

2. **Check Leg Positions**: Compare each leg with the reference images below
   - **Coxa (hip) joint**: Should be perpendicular to body
   - **Femur (thigh) joint**: Should be horizontal
   - **Tibia (shin) joint**: Should form 90° angle with femur

#### Step 3: Adjust Offsets

For each misaligned servo:

1. **Locate the servo** in the web interface:
   - Left column: Left Leg 1, 2, 3
   - Right column: Right Leg 1, 2, 3
   - Each leg has 3 joints: Joint 1 (coxa), Joint 2 (femur), Joint 3 (tibia)

2. **Adjust the offset value**:
   - Use **+** and **-** buttons for fine adjustment (±1 tick per click)
   - Or type a value directly in the input field
   - Changes apply immediately to the servo
   - 1 tick ≈ 0.44° for most servos

3. **Observe the movement** and continue adjusting until the servo reaches the correct position

#### Step 4: Save Your Calibration

1. Once all servos are properly aligned, click **"Save Offsets"**
   - Offset values are saved to **EEPROM** (persistent storage)
   - Values are also printed to Serial Monitor as backup
   - The hexapod will remember these values even after power cycles

2. **Now tighten the servo horn screws** - With each servo in its correct neutral position, firmly tighten the small screws on each servo horn to secure them in place

3. Click **"Exit Calibration Mode"** to resume normal operation

#### Step 5: Test Walking

Send a walking command to verify smooth motion. If adjustments are needed, simply re-enter calibration mode and fine-tune.

### Reference Images

**Front View**

![calibration_1](./images/calibration_1.jpg)

**Top View**

![calibration_2](./images/calibration_2.jpg)

### Calibration Tips

- **Use the web interface**: Much faster than editing code and re-uploading
- **Start with one leg**: Calibrate completely before moving to the next
- **Make small adjustments**: Use the +/- buttons for precise control (±1 tick per click)
- **Real-time feedback**: Servos respond immediately as you adjust values
- **Save your work**: Don't forget to click "Save Offsets" when done!
- **Check screw tightness**: Loose servo horns will affect calibration
- **Servo horn position**: If offsets exceed ±25 ticks, consider repositioning the servo horn physically
