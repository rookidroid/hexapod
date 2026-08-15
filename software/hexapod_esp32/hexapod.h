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
  const char *cmd;  // UDP command string
  int length;       // Number of steps in sequence
  int (*lut)[6][3]; // LUT: [step][leg][joint]
};

// ============================================================================
// System state (hexapod_esp32.ino)
// ============================================================================

extern bool ota_mode;                     // OTA updates enabled until first command
extern bool wifi_connected;               // WiFi AP connection status
extern bool boot_sequence_executed;       // Boot sequence completion flag
extern volatile bool trigger_boot_sequence; // Boot trigger from WiFi event
extern bool calibration_mode;             // Calibration mode flag

extern int current_motion_idx;            // Motion being played
extern int next_motion_idx;               // Motion to play next
extern unsigned long last_udp_packet_time; // Tracks last UDP packet for failsafe

// ============================================================================
// Servos and motion playback (motion_control.ino)
// ============================================================================

extern Adafruit_PWMServoDriver left_pwm;
extern Adafruit_PWMServoDriver right_pwm;

// The motion table and its length. sizeof() cannot size an extern array, so
// the count travels with it.
extern const MotionConfig motion_config[];
extern const size_t motion_config_count;

void setupServos();
void setPwmEnabled(bool enabled);
void posture_calibration();
void setAllServos(int positions[][3]);
void boot_up_motion(int lut_size, int lut[][6][3]);
void exec_motion(int lut_size, int lut[][6][3]);
void exec_transition(int start_pos[][6][3], int start_pos_idx,
                     int end_pos[][6][3], int end_pos_idx);
void copyPose(int src[6][3], int dst[6][3]);
bool posesEqual(int a[6][3], int b[6][3]);

// ============================================================================
// Real-time pose streaming (realtime.ino)
// ============================================================================
// Pose packets are parsed inside the AsyncUDP task while the main loop drives
// the servos, so every shared field below is written under `realtime_mux`.
// Without it a pose could be torn across two packets, which would show up as a
// violent servo jump.
extern portMUX_TYPE realtime_mux;

// These mirror the motion LUT element type (int) so poses can be copied to and
// from the LUTs directly. The wire format is int16_t and is widened on receipt.
extern bool realtime_mode;      // Streaming poses instead of LUTs
extern int realtime_target[6][3]; // Latest commanded pose (ticks)
extern int realtime_current[6][3]; // Pose actually written to the servos
extern uint16_t realtime_max_step; // Slew limit
extern bool realtime_snap;      // Skip the slew limit for one cycle
extern bool realtime_returning; // Easing back to standby after a dropout
extern unsigned long realtime_last_packet_time;

void enterRealtimeMode();
void exitRealtimeMode();
void serviceRealtimePose();

// ============================================================================
// Calibration storage (calibration.ino)
// ============================================================================
// The offsets themselves live in config.h so their defaults sit beside the
// rest of the hardware description.

void setupCalibration();
void loadOffsetsFromEEPROM();
void saveOffsetsToEEPROM();

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
void WiFiEvent(arduino_event_id_t event);

#endif // HEXAPOD_H
