/**

  Shared state and module interfaces

  The sketch is split across several .ino tabs, which the Arduino build
  concatenates into one translation unit. This header is the seam between them:
  each module defines the globals listed under its own heading, and every other
  module reaches them through the declarations here.

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

*/

#ifndef HEXAPOD_H
#define HEXAPOD_H

#include <Adafruit_PWMServoDriver.h>
#include <Arduino.h>
#include <AsyncUDP.h>
#include <WebServer.h>
#include <WiFi.h>

#include "config.h"
#include "protocol.h"

// A whole-body pose: servo ticks for [leg][joint], before calibration offsets.
// Leg order is right front/middle/back, then left front/middle/back; joint order
// is coxa, femur, tibia. The motion LUTs are arrays of these.
typedef int16_t Pose[6][3];

/**
 * @brief Unified motion configuration structure
 *
 * Maps UDP command strings directly to their corresponding look-up tables.
 * This centralizes all motion definitions in one place.
 *
 * @param cmd Command string received via UDP
 * @param length Number of steps in the motion sequence
 * @param lut Pointer to the look-up table with servo positions
 */
struct MotionConfig
{
  const char *cmd; // UDP command string
  int length;      // Number of steps in sequence
  const Pose *lut; // LUT: [step][leg][joint]
};

// ============================================================================
// System state (hexapod_esp32.ino)
// ============================================================================
// Fields marked volatile are written by the AsyncUDP task and read by the main
// loop. They are single words, so each access is atomic on the ESP32.

extern volatile bool ota_mode;            // OTA updates enabled until first command
extern bool wifi_connected;               // WiFi AP connection status
extern bool boot_sequence_executed;       // Boot sequence completion flag
extern volatile bool trigger_boot_sequence; // Boot trigger from WiFi event
extern bool calibration_mode;             // Calibration mode flag (main loop only)

extern volatile int next_motion_idx;      // Motion requested over UDP
extern volatile unsigned long last_udp_packet_time; // Last UDP packet, for failsafe
extern volatile uint8_t motion_speed_pct; // LUT playback speed (percent)

// Clamp `pct` to [MOTION_SPEED_MIN_PCT, 100] and make it the playback speed.
void setMotionSpeed(int pct);

// ============================================================================
// Servos and motion playback (motion_control.ino)
// ============================================================================
// Only the main loop drives the servos. The UDP task never writes to them; it
// leaves requests in the flags above and in the real-time state below, and
// serviceMotionEngine() acts on them at its next tick.

extern Adafruit_PWMServoDriver left_pwm;
extern Adafruit_PWMServoDriver right_pwm;

// The motion table and its length. sizeof() cannot size an extern array, so
// the count travels with it.
extern const MotionConfig motion_config[];
extern const size_t motion_config_count;

// Motion engine modes. Declared here rather than in motion_control.ino because
// the Arduino build hoists function prototypes above a tab's own types.
enum EngineMode : uint8_t
{
  ENGINE_LUT,         // Playing the motion LUTs
  ENGINE_REALTIME,    // Chasing streamed poses
  ENGINE_CALIBRATION, // Holding still for the web calibration page
  ENGINE_RELAXED      // PWM drive cut, servos limp
};

// LUT playback phases. Gait changes go through standby, the one posture every
// gait is safe to enter and leave from.
enum LutPhase : uint8_t
{
  LUT_PLAYING,    // Stepping through the active motion's frames
  LUT_TO_STANDBY, // Slewing from wherever the legs are to standby
  LUT_TO_START    // Slewing from standby to the next motion's first frame
};

// The pose last written to the servos. Every transition starts from here, so
// switching modes never assumes where the legs are.
extern Pose pose_current;

void setupServos();
void setPwmEnabled(bool enabled);
void writeServo(int leg_idx, int joint_idx, int ticks);
void writePose(const Pose pose);
void posture_calibration();
void boot_up_motion(int lut_size, const Pose lut[]);
void serviceMotionEngine();
bool slewToward(Pose pose, const Pose target, int step);
void copyPose(const Pose src, Pose dst);

// ============================================================================
// Real-time pose streaming (realtime.ino)
// ============================================================================
// Pose packets are parsed inside the AsyncUDP task while the main loop drives
// the servos, so every shared field below is written under `realtime_mux`.
// Without it a pose could be torn across two packets, which would show up as a
// violent servo jump.
extern portMUX_TYPE realtime_mux;

extern volatile bool realtime_mode;    // Streaming requested instead of LUTs
extern volatile bool relax_requested;  // PWM drive cut until the next command
extern Pose realtime_target;           // Latest commanded pose (ticks)
extern bool realtime_target_valid;     // A pose has arrived since entering
extern uint16_t realtime_max_step;     // Slew limit
extern bool realtime_snap;             // Skip the slew limit for one cycle
extern volatile unsigned long realtime_last_packet_time;

// Called from the UDP task: they only record the request.
void enterRealtimeMode();
void exitRealtimeMode();
// Called from the motion engine, once per real-time control cycle.
void serviceRealtimePose();

// ============================================================================
// Calibration storage (calibration.ino)
// ============================================================================
// The offsets themselves live in config.h so their defaults sit beside the
// rest of the hardware description.

void setupCalibration();
void loadOffsets();
bool saveOffsets();
int clampOffset(int offset);

// ============================================================================
// Calibration web interface (web_ui.ino)
// ============================================================================

extern WebServer web_server;

void setupWebServer();

// ============================================================================
// WiFi, OTA and UDP (network.ino)
// ============================================================================

extern AsyncUDP udp_socket;

void setupWiFi();
void setupOta();
void setupUdp();
void parseCommand(char *data, size_t length);
void selectMotion(int motion_idx);
void WiFiEvent(arduino_event_id_t event);

#endif // HEXAPOD_H
