/**
 * This is an automatically generated header, which includes the robot's
 * config from software/path_tool/robots/<name>.json. The firmware serves it
 * at GET /robot_config.
 * 
 * - Copyright (C) 2024 - PRESENT  rookidroid.com
 * - E-mail: info@rookidroid.com
 * - Website: https://rookidroid.com/
 */

#ifndef ROBOT_GEOMETRY_H
#define ROBOT_GEOMETRY_H

static const char ROBOT_GEOMETRY_JSON[] = R"json({"name":"mochi","label":"Mochi","legNames":["front_right","center_right","rear_right","front_left","center_left","rear_left"],"legMountX":[40.9,81.8,40.9,-40.9,-81.8,-40.9],"legMountY":[70.84,0,-70.84,70.84,0,-70.84],"legMountAngle":[60,0,-60,-240,-180,-120],"legScale":[[1,1,1],[1,1,1],[1,1,1],[1,-1,-1],[1,-1,-1],[1,-1,-1]],"legRootToJoint1":0,"legJoint1ToJoint2":36.0,"legJoint2ToJoint3":43.6,"legJoint3ToTip":85.22,"servoMin":102,"servoMax":512,"jointLimits":{"coxia":45,"femur":75,"tibia":75},"standbyPosture":[60,75],"laydownPosture":[25,25],"gait":{"walk_radius":30,"fastwalk_y_radius":40,"fastwalk_z_radius":30,"fastwalk_x_radius":15,"turn_radius":35}})json";

#endif // ROBOT_GEOMETRY_H
