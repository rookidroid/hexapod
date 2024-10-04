# <img src="./images/hexapod-logo.svg" alt="logo" width="64"/> Hexapod

A 3D Printed Hexapod Robot

![hexapod_model](./images/hexapod_model.jpg)

![booting_up](./images/booting_up.gif)

## Introduction

This agile, 3D-printed hexapod robot, built with ESP32 and Arduino, offers a range of advanced features, including:

- A robust, durable structure
- WiFi-enabled remote control
- Smooth, agile movement
- Over-the-air (OTA) firmware updates for easy maintenance

### Watch on Youtube

[![youtube_booting_up](https://img.youtube.com/vi/QFQOPC1Mbdo/0.jpg)](https://www.youtube.com/watch?v=QFQOPC1Mbdo)

[![youtube_remote_control](https://img.youtube.com/vi/YvXEto3tymQ/0.jpg)](https://www.youtube.com/watch?v=YvXEto3tymQ)

## Electronics

| Name                       | Thumbnail                                                                | Required          | Note      |
| -------------------------- | ------------------------------------------------------------------------ | ----------------- | --------- |
| ESP32 Dev Module           | <img src="./images/esp32.jpg" alt="esp32" width="300"/>                  | 1                 |           |
| MG92B or MG90S (metal gear)| <img src="./images/mg92b.jpg" alt="mg92b" width="300"/>                  | 18                | Can be mixed |
| PCA9685 servo driver       | <img src="./images/PCA9685.jpg" alt="PCA9685" width="300"/>              | 2                 | Need to be configured as different I2C addresss |
| XL4005 DC-DC step down     | <img src="./images/XL4005.jpg" alt="XL4005" width="300"/>                | 2                 | 6V output |
| Mini 360 DC-DC step down   | <img src="./images/mini360.jpg" alt="mini360" width="300"/>              | 1                 | 5V output |
| Switch                     | <img src="./images/switch.jpg" alt="switch" width="300"/>                | 1                 |           |
| 18650 battery              | <img src="./images/battery.jpg" alt="battery" width="300"/>              | 2                 |           |
| 18650 battery holder       | <img src="./images/battery_holder.jpg" alt="batter_holder" width="300"/>  | 1                 |           |

### Connection Diagram

![diagram](./images/diagram.svg)

## Mechanism

### Bill of Materials (BOM)

#### 3D-Printed Parts

![whole_assembly](./images/assembly_whole.gif)

##### Body x 1

![body_assembly](./images/assembly_body.gif)

| Filename       | Thumbnail                                                               | Required |
| -------------- | ----------------------------------------------------------------------- | -------- |
| body_top       | <img src="./images/body_top.jpg" alt="body_top" width="300"/>           | 1        |
| body_base      | <img src="./images/body_base.jpg" alt="body_base" width="300"/>         | 1        |
| body_side      | <img src="./images/body_side.jpg" alt="body_side" width="300"/>         | 2        |
| body_front_back| <img src="./images/body_front_back.jpg" alt="body_front_back" width="300"/>| 2        |
| body_battery   | <img src="./images/body_battery.jpg" alt="body_battery" width="300"/>     | 1        |
| body_servo_side1 | <img src="./images/body_servo_side1.jpg" alt="body_servo_side1" width="300"/> | 6        |
| body_servo_side2 | <img src="./images/body_servo_side2.jpg" alt="body_servo_side2" width="300"/> | 6        |
| body_servo_top   | <img src="./images/body_servo_top.jpg" alt="body_servo_top" width="300"/>     | 6        |

##### Joint x 6

![joint_assembly](./images/assembly_joint.gif)

| Filename        | Thumbnail                                                                   | Required | Note      |
| --------------- | --------------------------------------------------------------------------- | -------- | --------- |
| joint_bottom    | <img src="./images/joint_bottom.jpg" alt="joint_bottom" width="300"/>       | 12       |           |
| joint_cross     | <img src="./images/joint_cross.jpg" alt="joint_cross" width="300"/>         | 6        |           |
| joint_top_mg90s | <img src="./images/joint_top_mg90s.jpg" alt="joint_top_mg90s" width="300"/> | 12       | For MG90S |
| joint_top_mg92b | <img src="./images/joint_top_mg92b.jpg" alt="joint_top_mg92b" width="300"/> | 12       | For MG92B |

##### Leg x 6

![leg_assembly](./images/assembly_leg.gif)

| Filename        | Thumbnail                                                                   | Required |
| --------------- | --------------------------------------------------------------------------- | -------- |
| leg_bottom      | <img src="./images/leg_bottom.jpg" alt="leg_bottom" width="300"/>           | 6        |
| leg_side        | <img src="./images/leg_side.jpg" alt="leg_side" width="300"/>               | 12       |
| leg_top         | <img src="./images/leg_top.jpg" alt="leg_top" width="300"/>                 | 6        |

##### Foot x 6

![foot_assembly](./images/assembly_foot.gif)

| Filename        | Thumbnail                                                                   | Required | Note      |
| --------------- | --------------------------------------------------------------------------- | -------- | --------- |
| foot_bottom     | <img src="./images/foot_bottom.jpg" alt="foot_bottom" width="300"/>         | 6        |           |
| foot_ground     | <img src="./images/foot_ground.jpg" alt="foot_ground" width="300"/>         | 6        |           |
| foot_tip        | <img src="./images/foot_tip.jpg" alt="foot_tip" width="300"/>               | 6        |           |
| foot_top_mg90s  | <img src="./images/foot_top_mg90s.jpg" alt="foot_top_mg90s" width="300"/>   | 6        | For MG90S |
| foot_top_mg92b  | <img src="./images/foot_top_mg92b.jpg" alt="foot_top_mg92b" width="300"/>   | 6        | For MG92B |

##### Accessory

| Filename               | Thumbnail                                                                                 | Note             |
| ---------------------- | ----------------------------------------------------------------------------------------- | ---------------- |
| accessory_spacer_mg90s | <img src="./images/accessory_spacer_mg90s.jpg" alt="accessory_spacer_mg90s" width="300"/> | Spacer for MG90S |
| accessory_cable_holder | <img src="./images/accessory_cable_holder.jpg" alt="accessory_cable_holder" width="300"/> |                  |

#### Others

| Name      | Spec    | Required |
| --------- | ------- | -------- |
| Screw     | M2 6mm  | 36       |
| Screw     | M2 10mm | 198      |
| Nuts      | M2      | 234      |
| Pin (304) | M4 6mm  | 18       |
| Bearing   | MR74-2RS (4mm ID, 7mm OD, 2.5mm Bore) | 18 |

## Software

### ESP32-Arduino

Source code is under `./hexapod_arduino`.

- `hexapod_arduino.ino`: Main Arduino sketch
- `config.h`: Configuration header. Change the configurations based on your servo connections
- `motion.h`: Automatically generated motion look-up-table using `path_tool'

### Android

*Working in progress*

### PC

*Working in progress*
