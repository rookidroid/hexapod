/**

  Robot selection

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

  This firmware drives every hexapod in the family. Pick the robot you built
  by leaving exactly one of the defines below uncommented. The choice selects
  the robot's settings (src/robots/<name>/robot_config.h) and its motion LUTs
  (src/robots/<name>/motion.h).

  The robot can also be picked from the command line without editing this
  file, e.g. arduino-cli compile --build-property "build.extra_flags=-DROBOT_MOCHI"

*/

#ifndef ROBOT_H
#define ROBOT_H

#if !defined(ROBOT_NOUGAT) && !defined(ROBOT_MOCHI) && !defined(ROBOT_MACAROON)
// #define ROBOT_NOUGAT
// #define ROBOT_MOCHI
#define ROBOT_MACAROON
#endif

#if (defined(ROBOT_NOUGAT) + defined(ROBOT_MOCHI) + defined(ROBOT_MACAROON)) > 1
#error "Select only one robot in robot.h"
#endif

#if defined(ROBOT_NOUGAT)
#include "src/robots/nougat/robot_config.h"
#elif defined(ROBOT_MOCHI)
#include "src/robots/mochi/robot_config.h"
#elif defined(ROBOT_MACAROON)
#include "src/robots/macaroon/robot_config.h"
#else
#error "No robot selected: uncomment one ROBOT_* define in robot.h"
#endif

#endif  // ROBOT_H
