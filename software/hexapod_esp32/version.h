/**

  Firmware version

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

  Reported over UDP (version query, see protocol.h), by GET /robot_config and
  on the serial port at boot. Bump the numbers by hand when releasing:
  - MAJOR: a change that breaks existing clients or saved calibrations
  - MINOR: new features, packets or routes
  - PATCH: fixes and tuning

*/

#ifndef VERSION_H
#define VERSION_H

#define FIRMWARE_VERSION_MAJOR 3
#define FIRMWARE_VERSION_MINOR 2
#define FIRMWARE_VERSION_PATCH 0

// Free-form build tag, e.g. the git describe of the release. package_esp32.py
// stamps it into each package; it can also be passed with -DFIRMWARE_BUILD=...
#ifndef FIRMWARE_BUILD
#define FIRMWARE_BUILD "dev"
#endif

#define FIRMWARE_STRINGIFY_(x) #x
#define FIRMWARE_STRINGIFY(x) FIRMWARE_STRINGIFY_(x)

// "MAJOR.MINOR.PATCH"
#define FIRMWARE_VERSION                         \
  FIRMWARE_STRINGIFY(FIRMWARE_VERSION_MAJOR) "." \
  FIRMWARE_STRINGIFY(FIRMWARE_VERSION_MINOR) "." \
  FIRMWARE_STRINGIFY(FIRMWARE_VERSION_PATCH)

#endif  // VERSION_H
