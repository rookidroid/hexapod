# <img src="./imgs/hexapod-logo.svg" alt="logo" width="64"/> Hexapod

A 3D Printed Hexapod Robot

![banner](imgs/banner.jpg)

[<img alt="google_play" width="200" src="./imgs/google_play.svg" />](https://play.google.com/store/apps/details?id=com.rookiedev.hexapod)
[<img alt="windows" width="200" src="./imgs/windows_app.svg" />](https://github.com/rookiepeng/hexapod/releases)
[<img alt="linux" width="200" src="./imgs/linux_app.svg" />](https://github.com/rookiepeng/hexapod/releases)

## Introduction

This is a hexapod robot based on [Smallp Tsai](https://github.com/SmallpTsai)'s [hexapod-v2-7697](https://github.com/SmallpTsai/hexapod-v2-7697) project.
This project reused most of the mechanical design in the original project, but with a full redesign of the circuits and software.
The table below shows the difference between this project and the original one.

|                 | Original hexapod-v2-7697 | This project                                    |
| --------------- | ------------------------ | ----------------------------------------------- |
| Controller      | Linkit 7697              | Raspberry Pi Zero W or Raspberry Pi Zero W 2    |
| PWM control     | Custom circuit board     | 2 x PCA9685 motor driver boards                 |
| DC-DC           | 7 x mini360 modules      | 2 x XL4005 5A Max DC-DC modules and 1 x mini360 |
| Power           | 2S Lipo battery          | 2 x 18650 batteries                             |
| Remote          | BLE                      | WiFi, Bluetooth                                 |
| Remote software | Android and iOS          | PC, Android                                     |

https://user-images.githubusercontent.com/471808/148647807-02e3e901-6181-4473-8ac4-3b5998fa8d17.mp4

https://user-images.githubusercontent.com/471808/158099495-1caeb903-a2e6-4e8d-9f13-54728d7d255a.mp4

## Software

### Raspberry Pi

#### Requirements

Raspberry Pi is running [`Raspberry Pi OS`](https://www.raspberrypi.com/software/).

- Enable `I2C` through `raspi-config`

``` bash
sudo raspi-config
```

- Install required packages

``` bash
sudo apt-get install python3-numpy python3-pip
```

- Install required Python modules

``` bash
pip3 install adafruit-circuitpython-servokit
```

> Tips: It is convenient to debug code on Raspberry Pi through VSCode's remote feature.

#### Running

The hexapod program, located under `./software/raspberry pi/`, is written in Python. Use the following command to start it manually.

``` bash
python3 hexapod.py
```

### Android

<img src="./imgs/screenshot_wifi.png" alt="screenshot_wifi" width="300"/> <img src="./imgs/screenshot_bluetooth.png" alt="screenshot_bluetooth" width="300"/>

<img src="./imgs/screenshot_control1.png" alt="screenshot_control1" width="600"/>

<img src="./imgs/screenshot_control2.png" alt="screenshot_control2" width="600"/>

### PC

PC program source files are located under `./software/pc/`. The compiled program can be downloaded under the [release](https://github.com/rookiepeng/hexapod/releases) page.

<img src="./imgs/pc_interface.jpg" alt="pc_interface" width="600"/>

- Find the correct IP address of the Raspberry Pi on the hexapod
- Connect to the hexapod using the IP address
- Use the buttons on the panel to control the hexapod

## Mechanism

### Bill of Materials (BOM)

#### Servos

18 TowerPro [MG92B](http://www.towerpro.com.tw/product/mg92b/) are required.

> The dimension of 3d printed part is highly dependent on the servo's size. Modification is required if you want to use alternative servos.

#### 3D-Printed Parts

##### Body x 1

| Filename       | Thumbnail                                                               | Required |
| -------------- | ----------------------------------------------------------------------- | -------- |
| body_top       | <img src="./imgs/body_top.jpg" alt="body_top" width="300"/>             | 1        |
| body_bottom    | <img src="./imgs/body_bottom.jpg" alt="body_bottom" width="300"/>       | 1        |
| body_side_wall | <img src="./imgs/body_side_wall.jpg" alt="body_side_wall" width="300"/> | 2        |
| body_back_wall | <img src="./imgs/body_back_wall.jpg" alt="body_back_wall" width="300"/> | 2        |
| battery_box    | <img src="./imgs/battery_box.jpg" alt="battery_box" width="300"/>       | 1        |
| battery_cover  | <img src="./imgs/battery_cover.jpg" alt="battery_cover" width="300"/>   | 1        |

##### Leg x 6

| Filename        | Thumbnail                                                                 | Required |
| --------------- | ------------------------------------------------------------------------- | -------- |
| thigh_top       | <img src="./imgs/thigh_top.jpg" alt="thigh_top" width="300"/>             | 1 (x6)   |
| joint_cross     | <img src="./imgs/joint_cross.jpg" alt="joint_cross" width="300"/>         | 1 (x6)   |
| joint_top       | <img src="./imgs/joint_top.jpg" alt="joint_top" width="300"/>             | 2 (x6)   |
| joint_bottom    | <img src="./imgs/joint_bottom.jpg" alt="joint_bottom" width="300"/>       | 2 (x6)   |
| leg_top         | <img src="./imgs/leg_top.jpg" alt="leg_top" width="300"/>                 | 1 (x6)   |
| let_bottom      | <img src="./imgs/leg_bottom.jpg" alt="leg_bottom" width="300"/>           | 1 (x6)   |
| foot_top        | <img src="./imgs/foot_top.jpg" alt="foot_top" width="300"/>               | 1 (x6)   |
| foot_bottom     | <img src="./imgs/foot_bottom.jpg" alt="foot_bottom" width="300"/>         | 1 (x6)   |
| foot_ground     | <img src="./imgs/foot_ground.jpg" alt="foot_ground" width="300"/>         | 1 (x6)   |
| foot_tip        | <img src="./imgs/foot_tip.jpg" alt="foot_tip" width="300"/>               | 1 (x6)   |

#### Others

| Name      | Spec    | Thumbnail                    | Required | Note                                              |
| --------- | ------- | ---------------------------- | -------- | ------------------------------------------------- |
| Screw     | M2 6mm  | ![6mm](imgs/M2_6mm.JPG)     | 54       | Servo Arm: 1 x 18<br>Joint: 4 x 6<br>Thigh: 2 x 6 |
| Screw     | M2 10mm | ![10mm](imgs/M2_10mm.JPG)   | 24       | Thigh: 1 x 6<br>Pin lock: 1 x 18                  |
| Screw     | M2 30mm | ![30mm](imgs/M2_30mm.JPG)   | 36       | Servo: 2 x 18                                     |
| Nuts      | M2      | ![6mm](imgs/M2_nut.JPG)     | 36       | Servo: 2 x 18                                     |
| Pin (304) | M4 6mm  | ![pin](imgs/pin_M4_6mm.JPG) | 18       | Servo: 1 x 18                                     |

### Leg Assemble

#### Assembly video

<a href='https://youtu.be/oaAE5fC09KQ'><img src='http://img.youtube.com/vi/oaAE5fC09KQ/mqdefault.jpg'/></a>

#### Exploded view

<img src="./imgs/mech_leg_exploded.jpg" alt="LegAssemble" width="500"/>

#### Detailed steps

- **Step 1** - Combine `thigh_top`, `MG92B` and `thigh_bottom`, use M2x30mm screw and nut to secure them together.

<img src="./imgs/mech_leg_1.jpg" alt="LegAssemble1" width="500"/>

- **Step 2** - Grab one `joint_cross`, `joint_top` and `joint_bottom` and combine them

<img src="./imgs/mech_leg_2.jpg" alt="LegAssemble2" width="500"/>

- **Step 3** - First put 2 x `MG92B`, `leg_top` and `leg_bottom` together with M2x30mm screw and nut. Then attach `joint_top` and `joint_bottom` altogether

<img src="./imgs/mech_leg_3.jpg" alt="LegAssemble3" width="500"/>

- **Step 4** - Grab `foot_top`, `foot_bottom` and `foot_ground` (and `foot_top`) and assemble them.

<img src="./imgs/mech_leg_4.jpg" alt="LegAssemble4" width="500"/>

- **Finish** - Great, you got 1 leg done, 5 more to go :)

<img src="./imgs/mech_leg_5.jpg" alt="LegAssemble5" width="500"/>

## Electronics

### Bill of Materials (BOM)

| Name                       | Thumbnail                                                              | Required |
| -------------------------- | ---------------------------------------------------------------------- | -------- |
| Raspberry Pi Zero W or W 2 | <img src="./imgs/zero2-close-up.webp" alt="raspberry pi" width="300"/> | 1        |
| PCA9685 motor driver       | <img src="./imgs/PCA9685.jpg" alt="PCA9685" width="300"/>              | 2        |
| XL4005 DC-DC step down     | <img src="./imgs/XL4005.jpg" alt="XL4005" width="300"/>                | 2        |
| Mini 360 DC-DC step down   | <img src="./imgs/mini360.jpg" alt="mini360" width="300"/>              | 1        |
| Toggle switch              | <img src="./imgs/toggle_switch.jpg" alt="toggle_switch" width="300"/>  | 1        |

### Connection Diagram

<img src="./imgs/diagram.svg" alt="diagram" width="800"/>

