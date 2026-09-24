/**

  Nougat robot configuration

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

  Settings that differ between robots. Everything shared lives in config.h.
  Selected through robot.h.

*/

#ifndef ROBOT_CONFIG_H
#define ROBOT_CONFIG_H

#define ROBOT_NAME "nougat"

#define DELAY_MS 12  // Servo delay between LUT steps (ms)

/** WiFi access point name */
#ifndef APSSID
#define APSSID "hexapod_nougat"
#endif

#endif  // ROBOT_CONFIG_H
