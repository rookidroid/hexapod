/**

  UDP wire protocol -- packet layouts shared with the host-side clients

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

*/

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>

enum RobotCommand : uint8_t {
  CMD_STANDBY = 0,
  CMD_WALK_0 = 1,
  CMD_WALK_180 = 2,
  CMD_WALK_R45 = 3,
  CMD_WALK_R90 = 4,
  CMD_WALK_R135 = 5,
  CMD_WALK_L45 = 6,
  CMD_WALK_L90 = 7,
  CMD_WALK_L135 = 8,
  CMD_FAST_FORWARD = 9,
  CMD_FAST_BACKWARD = 10,
  CMD_TURN_LEFT = 11,
  CMD_TURN_RIGHT = 12,
  CMD_CLIMB_FORWARD = 13,
  CMD_CLIMB_BACKWARD = 14,
  CMD_ROTATE_X = 15,
  CMD_ROTATE_Y = 16,
  CMD_ROTATE_Z = 17,
  CMD_TWIST = 18
};

// Packet discriminators. The first byte of every binary packet selects the
// protocol, so new packet types can be added without disturbing old clients.
const uint8_t MAGIC_MOTION = 0xA5;  // Pre-programmed motion command (legacy)
const uint8_t MAGIC_POSE = 0xA6;    // Real-time 18-servo pose
const uint8_t MAGIC_SESSION = 0xA7; // Real-time session control

// Actions carried by a session packet
enum RealtimeAction : uint8_t {
  RT_EXIT = 0,  // Leave real-time mode, hand back to the motion LUT engine
  RT_ENTER = 1, // Enter real-time mode, holding the standby posture
  RT_RELAX = 2, // Cut PWM drive so the servos go limp
  RT_PING = 3   // No-op; keeps the failsafe timer alive while idle
};

#pragma pack(push, 1)
struct UdpControlPacket {
  uint8_t magic;      // 0xA5
  RobotCommand cmd;
  uint32_t seq_num;
};

// Real-time pose: raw servo ticks for all 18 joints.
// Leg order matches the motion LUTs: right front/middle/back, then left
// front/middle/back. Joint order is coxa, femur, tibia.
struct UdpPosePacket {
  uint8_t magic;      // 0xA6
  uint8_t flags;      // bit0: 1 = snap immediately, 0 = obey the slew limit
  uint16_t max_step;  // Per-joint slew limit in ticks/cycle (0 = use default)
  uint32_t seq_num;
  int16_t ticks[6][3];
};

struct UdpSessionPacket {
  uint8_t magic;       // 0xA7
  RealtimeAction action;
  uint32_t seq_num;
};
#pragma pack(pop)

// Bit values for UdpPosePacket::flags
const uint8_t POSE_FLAG_SNAP = 0x01;

// Wire-format guarantees the host-side client depends on. Note that the motion
// and session packets are the same length, so parseCommand() must dispatch on
// the magic byte before it looks at the length.
static_assert(sizeof(UdpControlPacket) == 6, "control packet must be 6 bytes");
static_assert(sizeof(UdpPosePacket) == 44, "pose packet must be 44 bytes");
static_assert(sizeof(UdpSessionPacket) == 6, "session packet must be 6 bytes");

#endif  // PROTOCOL_H
