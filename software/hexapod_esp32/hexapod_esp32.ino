/**

  Hexapod -- A 3D Printed Hexapod Robot

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

                        **
                       ****
                        **
                        **
                        **
                        **

        **********************************
      **************************************
     ****************************************
     ********      ************      ********
     *******        **********        *******
     *******        **********        *******
     ********      ************      ********
     ****************************************
     ****************************************
     ****************************************
     ****************************************


            **************************

                ******************

*/

/** WiFi */
#include <AsyncUDP.h>
#include <WiFi.h>

/** PWM */
#include <Adafruit_PWMServoDriver.h>

/** I2C */
#include <Wire.h>

/** OTA */
#include <ArduinoOTA.h>

/** Robot Configuration */
#include "config.h"

/** Motion Path LUT */
#include "motion.h"

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
struct MotionConfig {
  const char* cmd;   // UDP command string
  int length;        // Number of steps in sequence
  int (*lut)[6][3];  // LUT: [step][leg][joint]
};

// Motion configuration table - add new motions here
static const MotionConfig motion_config[] = {
  { "standby", lut_standby_length, lut_standby },
  { "walk0", lut_walk_0_length, lut_walk_0 },
  { "walk180", lut_walk_180_length, lut_walk_180 },
  { "walkr45", lut_walk_r45_length, lut_walk_r45 },
  { "walkr90", lut_walk_r90_length, lut_walk_r90 },
  { "walkr135", lut_walk_r135_length, lut_walk_r135 },
  { "walkl45", lut_walk_l45_length, lut_walk_l45 },
  { "walkl90", lut_walk_l90_length, lut_walk_l90 },
  { "walkl135", lut_walk_l135_length, lut_walk_l135 },
  { "fastforward", lut_fast_forward_length, lut_fast_forward },
  { "fastbackward", lut_fast_backward_length, lut_fast_backward },
  { "turnleft", lut_turn_left_length, lut_turn_left },
  { "turnright", lut_turn_right_length, lut_turn_right },
  { "climbforward", lut_climb_forward_length, lut_climb_forward },
  { "climbbackward", lut_climb_backward_length, lut_climb_backward },
  { "rotatex", lut_rotate_x_length, lut_rotate_x },
  { "rotatey", lut_rotate_y_length, lut_rotate_y },
  { "rotatez", lut_rotate_z_length, lut_rotate_z },
  { "twist", lut_twist_length, lut_twist }
};

// ============================================================================
// Hardware Configuration
// ============================================================================

// I2C addresses for PCA9685 PWM driver boards
const uint8_t LEFT_PWM_ADDRESS = 0x40;   // Left side servos (3 legs)
const uint8_t RIGHT_PWM_ADDRESS = 0x41;  // Right side servos (3 legs)

// GPIO pins for PWM driver enable control (active LOW)
const uint8_t LEFT_PWM_ENABLE_PIN = 19;   // Enable left legs PWM driver
const uint8_t RIGHT_PWM_ENABLE_PIN = 26;  // Enable right legs PWM driver

// ============================================================================
// Servo Control Parameters
// ============================================================================

// Delay between individual servo movements during boot (ms)
const uint16_t SERVO_INIT_DELAY_MS = 50;

// Step size for smooth transitions between positions
const uint8_t TRANSITION_TICK_STEP = 6;

// PWM frequency for servo control signals (Hz)
const uint16_t SERVO_PWM_FREQ = 50;

// ============================================================================
// Global Objects and State
// ============================================================================

// PWM driver instances for left and right servo banks
Adafruit_PWMServoDriver left_pwm = Adafruit_PWMServoDriver(LEFT_PWM_ADDRESS);
Adafruit_PWMServoDriver right_pwm = Adafruit_PWMServoDriver(RIGHT_PWM_ADDRESS);

// Current motion index in motion_config table
int current_motion_idx = 0;  // Starts at standby
int next_motion_idx = 0;     // Next motion to execute

// WiFi credentials (defined in config.h)
const char* ssid = APSSID;
const char* password = APPSK;

// UDP socket for receiving commands
AsyncUDP udp_socket;

// System state flags
bool ota_mode = true;                 // OTA updates enabled until first command
bool wifi_connected = false;          // WiFi AP connection status
bool boot_sequence_executed = false;  // Boot sequence completion flag
volatile bool trigger_boot_sequence =
  false;  // Boot trigger from WiFi event (volatile for ISR)

// Forward declarations
void parseCommand(char* data, size_t length);
void setAllServos(int positions[][3]);
void WiFiEvent(arduino_event_id_t event);

/**
   @brief Initialize system: WiFi AP, OTA, PWM drivers, and UDP server.
*/
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    ;  // Wait for serial port to connect, timeout after 3s
  }
  Serial.println("\n=== Hexapod Robot Initializing ===");

  // Register WiFi event handler
  WiFi.onEvent(WiFiEvent);

  // Initialize WiFi Access Point
  WiFi.mode(WIFI_AP);
  wifi_connected = WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();

  if (!wifi_connected) {
    Serial.println("ERROR: Failed to create WiFi AP");
  } else {
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  }

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS
      // using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });

  ArduinoOTA.begin();
  Serial.println("OTA update enabled");

  // Initialize the PCA9685 PWM drivers
  Serial.println("Initializing PWM drivers...");
  left_pwm.begin();
  left_pwm.setPWMFreq(SERVO_PWM_FREQ);

  right_pwm.begin();
  right_pwm.setPWMFreq(SERVO_PWM_FREQ);
  Serial.println("PWM drivers initialized");

  if (udp_socket.listen(UDP_PORT)) {
    Serial.print("UDP Listening on IP: ");
    Serial.println(myIP);
    // Register callback for incoming UDP packets
    udp_socket.onPacket([](AsyncUDPPacket packet) {
      // Log packet details for debugging
      Serial.print("UDP Packet Type: ");
      Serial.print(packet.isBroadcast()   ? "Broadcast"
                   : packet.isMulticast() ? "Multicast"
                                          : "Unicast");
      Serial.print(", From: ");
      Serial.print(packet.remoteIP());
      Serial.print(":");
      Serial.print(packet.remotePort());
      Serial.print(", To: ");
      Serial.print(packet.localIP());
      Serial.print(":");
      Serial.print(packet.localPort());
      Serial.print(", Length: ");
      Serial.print(packet.length());
      Serial.print(", Data: ");
      Serial.write(packet.data(), packet.length());
      Serial.println();
      // reply to the client
      packet.printf("Got %u bytes of data", packet.length());
      // Parse command from packet
      parseCommand((char*)packet.data(), packet.length());
    });
  }

  // Configure PWM driver enable pins (active LOW)
  pinMode(LEFT_PWM_ENABLE_PIN, OUTPUT);
  pinMode(RIGHT_PWM_ENABLE_PIN, OUTPUT);

  // Enable PWM drivers (active LOW)
  digitalWrite(LEFT_PWM_ENABLE_PIN, LOW);   // Enable left legs PWM driver
  digitalWrite(RIGHT_PWM_ENABLE_PIN, LOW);  // Enable right legs PWM driver

  if (!wifi_connected) {
    Serial.println("WARNING: WiFi AP not started");
  }

  Serial.println("=== Initialization Complete ===");

  // posture_calibration();
}

/**
   @brief Main loop: execute boot sequence, motion commands, and OTA updates.
*/
void loop() {
  // Handle boot sequence trigger from WiFi event
  if (trigger_boot_sequence && !boot_sequence_executed) {
    boot_sequence_executed = true;
    trigger_boot_sequence = false;
    boot_up_motion(lut_standup_length, lut_standup);
    return;
  }

  // Don't execute motion commands until boot sequence is complete
  if (!boot_sequence_executed) {
    if (ota_mode) {
      ArduinoOTA.handle();
      delay(10);
    }
    return;
  }

  // Execute motion based on next_motion_idx
  exec_motion(motion_config[next_motion_idx].length,
              motion_config[next_motion_idx].lut);

  if (ota_mode) {
    ArduinoOTA.handle();
    delay(10);  // Small delay to prevent watchdog issues
  }
}

/**
   @brief Set all servos to neutral position using calibration offsets.
*/
void posture_calibration() {
  for (int leg_idx = 0; leg_idx < 3; leg_idx++) {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++) {
      right_pwm.setPWM(right_legs[leg_idx][joint_idx], 0,
                       SERVOMID + right_offset_ticks[leg_idx][joint_idx]);
      left_pwm.setPWM(left_legs[leg_idx][joint_idx], 0,
                      SERVOMID + left_offset_ticks[leg_idx][joint_idx]);
    }
  }
}

/**
   @brief Execute boot sequence to stand up the robot.
   @param lut_size Number of steps in the motion sequence
   @param lut Look-up table with servo positions for each step
*/
void boot_up_motion(int lut_size, int lut[][6][3]) {
  Serial.println("Starting boot sequence...");

  // Phase 1: Initialize servos to starting position
  // Gradual activation prevents current spikes and sudden movements
  for (int leg_idx = 0; leg_idx < 3; leg_idx++) {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++) {
      right_pwm.setPWM(
        right_legs[leg_idx][joint_idx], 0,
        lut[0][leg_idx][joint_idx] + right_offset_ticks[leg_idx][joint_idx]);
      delay(SERVO_INIT_DELAY_MS);
      left_pwm.setPWM(left_legs[leg_idx][joint_idx], 0,
                      lut[0][leg_idx + 3][joint_idx] + left_offset_ticks[leg_idx][joint_idx]);
      delay(SERVO_INIT_DELAY_MS);
    }
  }

  // Phase 2: Execute stand-up motion sequence
  // Step through each position in the LUT to stand up robot
  for (int lut_idx = 0; lut_idx < lut_size; lut_idx++) {
    setAllServos(lut[lut_idx]);
    delay(DELAY_MS);
  }

  Serial.println("Boot sequence complete");
}

/**
   @brief Execute motion sequence with transition and interruption support.
   @param lut_size Number of steps in the motion sequence
   @param lut Look-up table with servo positions for each step
*/
void exec_motion(int lut_size, int lut[][6][3]) {
  const int mid_step = lut_size / 2;

  // Transition from standby to target motion
  if (current_motion_idx == 0) {  // 0 = standby
    exec_transition(lut_standby, 0, lut, 0);
  }
  current_motion_idx = next_motion_idx;

  // Execute motion loop with interruption check
  for (int lut_idx = 0; lut_idx < lut_size; lut_idx++) {
    setAllServos(lut[lut_idx]);

    // Check for motion change at mid-point for smooth transitions
    // Allows interruption at stable points in the gait cycle
    if (mid_step > 0 && lut_idx % mid_step == 0 && current_motion_idx != next_motion_idx) {
      exec_transition(lut, lut_idx, lut_standby, 0);
      delay(DELAY_MS);
      break;
    }
    delay(DELAY_MS);
  }
}

/**
   @brief Smoothly transition between two servo positions.
   @param start_pos Starting position LUT
   @param start_pos_idx Index in start_pos LUT
   @param end_pos Target position LUT
   @param end_pos_idx Index in end_pos LUT
*/
void exec_transition(int start_pos[][6][3], int start_pos_idx,
                     int end_pos[][6][3], int end_pos_idx) {
  const int tick_step = TRANSITION_TICK_STEP;
  int max_step = 0;
  int signed_ticks[6][3];

  int current_pos[6][3];
  int diff;

  for (int leg_idx = 0; leg_idx < 6; leg_idx++) {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++) {
      diff = end_pos[end_pos_idx][leg_idx][joint_idx] - start_pos[start_pos_idx][leg_idx][joint_idx];
      current_pos[leg_idx][joint_idx] =
        start_pos[start_pos_idx][leg_idx][joint_idx];
      if (diff < 0) {
        signed_ticks[leg_idx][joint_idx] = -tick_step;
      } else {
        signed_ticks[leg_idx][joint_idx] = tick_step;
      }
      max_step = max(max_step, abs(diff));
    }
  }
  // Calculate number of steps needed (ceiling division ensures we reach target)
  max_step = (max_step + tick_step - 1) / tick_step;

  // Interpolate positions in small steps for smooth motion
  for (int step_idx = 0; step_idx < max_step; step_idx++) {
    for (int leg_idx = 0; leg_idx < 6; leg_idx++) {
      for (int joint_idx = 0; joint_idx < 3; joint_idx++) {
        // Update position towards target
        int remaining = abs(current_pos[leg_idx][joint_idx] - end_pos[end_pos_idx][leg_idx][joint_idx]);
        if (remaining > tick_step) {
          current_pos[leg_idx][joint_idx] += signed_ticks[leg_idx][joint_idx];
        } else {
          current_pos[leg_idx][joint_idx] =
            end_pos[end_pos_idx][leg_idx][joint_idx];
        }
      }
    }

    // Update all servos with new positions
    for (int leg_idx = 0; leg_idx < 3; leg_idx++) {
      for (int joint_idx = 0; joint_idx < 3; joint_idx++) {
        right_pwm.setPWM(right_legs[leg_idx][joint_idx], 0,
                         current_pos[leg_idx][joint_idx] + right_offset_ticks[leg_idx][joint_idx]);
        left_pwm.setPWM(left_legs[leg_idx][joint_idx], 0,
                        current_pos[leg_idx + 3][joint_idx] + left_offset_ticks[leg_idx][joint_idx]);
      }
    }
    delay(DELAY_MS / 2);  // Smoother transition with shorter delays
  }
}

/**
   @brief Set all servo positions from a LUT entry.
   @param positions Array of positions for all 6 legs (3 joints each)
*/
void setAllServos(int positions[][3]) {
  for (int leg_idx = 0; leg_idx < 3; leg_idx++) {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++) {
      right_pwm.setPWM(right_legs[leg_idx][joint_idx], 0,
                       positions[leg_idx][joint_idx] + right_offset_ticks[leg_idx][joint_idx]);
      left_pwm.setPWM(left_legs[leg_idx][joint_idx], 0,
                      positions[leg_idx + 3][joint_idx] + left_offset_ticks[leg_idx][joint_idx]);
    }
  }
}

/**
   @brief Parse UDP command string and update motion mode.
   @param data Command data buffer
   @param length Length of command data
*/
void parseCommand(char* data, size_t length) {
  if (length == 0) return;

  // Command buffer with space for null terminator
  char command[32] = { 0 };
  size_t cmd_len = 0;

  // Parse command string from UDP packet
  // Stops at delimiters: ':', '\n', '\r', '\0'
  for (size_t i = 0; i < length && i < sizeof(command) - 1; i++) {
    char c = data[i];

    // Check for delimiters that end the command
    if (c == ':' || c == '\n' || c == '\r' || c == '\0') {
      if (cmd_len > 0) {
        command[cmd_len] = '\0';
        break;
      }
      continue;  // Skip leading delimiters
    }

    // Add character to command buffer
    command[cmd_len++] = c;
  }

  // Null-terminate if we reached the end without a delimiter
  if (cmd_len > 0 && cmd_len < sizeof(command)) {
    command[cmd_len] = '\0';
  }

  if (cmd_len == 0) return;

  // Find matching command and set motion index
  bool found = false;
  for (size_t i = 0; i < sizeof(motion_config) / sizeof(motion_config[0]);
       i++) {
    if (strcmp(command, motion_config[i].cmd) == 0) {
      next_motion_idx = i;
      found = true;
      break;
    }
  }

  if (!found) {
    Serial.print("Unknown command: ");
    Serial.println(command);
    return;
  }

  // Disable OTA after first command for better performance
  ota_mode = false;
  Serial.print("Command received: ");
  Serial.println(command);
}

/**
   @brief Handle WiFi events: trigger boot on connect, disable servos on
   disconnect.
   @param event WiFi event ID
*/
void WiFiEvent(arduino_event_id_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      // Client connected - trigger boot sequence if not already done
      Serial.println("Station connected to AP");
      if (!boot_sequence_executed) {
        trigger_boot_sequence = true;  // Flag checked in main loop
      }
      break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      // Client disconnected - servos remain in last position
      Serial.println("Station disconnected from AP");
      break;
    case ARDUINO_EVENT_WIFI_AP_START:
      // Access point initialized and ready
      Serial.println("AP Started");
      break;
    default:
      break;
  }
}
