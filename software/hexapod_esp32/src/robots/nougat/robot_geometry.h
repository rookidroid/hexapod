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

static const char ROBOT_GEOMETRY_JSON[] = R"json({"name":"nougat","label":"Nougat","legNames":["front_right","center_right","rear_right","front_left","center_left","rear_left"],"legMountX":[44.82,61.03,44.82,-44.82,-61.03,-44.82],"legMountY":[74.82,0,-74.82,74.82,0,-74.82],"legMountAngle":[45,0,-45,-225,-180,-135],"legScale":[[1,-1,-1],[1,1,1],[1,1,1],[1,1,1],[1,-1,-1],[1,-1,-1]],"legRootToJoint1":0,"legJoint1ToJoint2":38.0,"legJoint2ToJoint3":54.06,"legJoint3ToTip":93.53,"servoMin":102,"servoMax":512,"jointLimits":{"coxia":45,"femur":75,"tibia":75},"standbyPosture":[60,75],"laydownPosture":[25,25],"gait":{"walk_radius":30,"fastwalk_y_radius":50,"fastwalk_z_radius":40,"fastwalk_x_radius":15,"turn_radius":35}})json";

#endif // ROBOT_GEOMETRY_H
