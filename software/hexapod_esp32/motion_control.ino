/**

  Servo output and motion playback -- the only path to the PCA9685 drivers

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

*/

#include <Wire.h>

#include "hexapod.h"
#include "motion.h"

// PWM driver instances for left and right servo banks
Adafruit_PWMServoDriver left_pwm = Adafruit_PWMServoDriver(LEFT_PWM_ADDRESS);
Adafruit_PWMServoDriver right_pwm = Adafruit_PWMServoDriver(RIGHT_PWM_ADDRESS);

// Motion configuration table - add new motions here
const MotionConfig motion_config[] = {
    {"standby", lut_standby_length, lut_standby},
    {"walk0", lut_walk_0_length, lut_walk_0},
    {"walk180", lut_walk_180_length, lut_walk_180},
    {"walkr45", lut_walk_r45_length, lut_walk_r45},
    {"walkr90", lut_walk_r90_length, lut_walk_r90},
    {"walkr135", lut_walk_r135_length, lut_walk_r135},
    {"walkl45", lut_walk_l45_length, lut_walk_l45},
    {"walkl90", lut_walk_l90_length, lut_walk_l90},
    {"walkl135", lut_walk_l135_length, lut_walk_l135},
    {"fastforward", lut_fast_forward_length, lut_fast_forward},
    {"fastbackward", lut_fast_backward_length, lut_fast_backward},
    {"turnleft", lut_turn_left_length, lut_turn_left},
    {"turnright", lut_turn_right_length, lut_turn_right},
    {"climbforward", lut_climb_forward_length, lut_climb_forward},
    {"climbbackward", lut_climb_backward_length, lut_climb_backward},
    {"rotatex", lut_rotate_x_length, lut_rotate_x},
    {"rotatey", lut_rotate_y_length, lut_rotate_y},
    {"rotatez", lut_rotate_z_length, lut_rotate_z},
    {"twist", lut_twist_length, lut_twist}};

const size_t motion_config_count =
    sizeof(motion_config) / sizeof(motion_config[0]);

/**
   @brief Bring up the PWM drivers and their enable pins.

   The enable pins are configured before the drivers are switched on so a pose
   packet arriving mid-boot cannot drive an unconfigured pin.
*/
void setupServos()
{
  // Initialize the PCA9685 PWM drivers
  Serial.println("Initializing PWM drivers...");
  left_pwm.begin();
  left_pwm.setPWMFreq(SERVO_PWM_FREQ);

  right_pwm.begin();
  right_pwm.setPWMFreq(SERVO_PWM_FREQ);
  Serial.println("PWM drivers initialized");

  // Configure PWM driver enable pins (active LOW)
  pinMode(LEFT_PWM_ENABLE_PIN, OUTPUT);
  pinMode(RIGHT_PWM_ENABLE_PIN, OUTPUT);

  setPwmEnabled(true);
}

/**
   @brief Set all servos to neutral position using calibration offsets.
*/
void posture_calibration()
{
  for (int leg_idx = 0; leg_idx < 3; leg_idx++)
  {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++)
    {
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
void boot_up_motion(int lut_size, int lut[][6][3])
{
  Serial.println("Starting boot sequence...");

  // Phase 1: Initialize servos to starting position
  // Gradual activation prevents current spikes and sudden movements
  for (int leg_idx = 0; leg_idx < 3; leg_idx++)
  {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++)
    {
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
  for (int lut_idx = 0; lut_idx < lut_size; lut_idx++)
  {
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
void exec_motion(int lut_size, int lut[][6][3])
{
  const int mid_step = lut_size / 2;

  // Transition from standby to target motion
  if (current_motion_idx == 0)
  { // 0 = standby
    exec_transition(lut_standby, 0, lut, 0);
  }
  current_motion_idx = next_motion_idx;

  // Execute motion loop with interruption check
  for (int lut_idx = 0; lut_idx < lut_size; lut_idx++)
  {
    setAllServos(lut[lut_idx]);

    // Check for motion change at mid-point for smooth transitions
    // Allows interruption at stable points in the gait cycle
    if (mid_step > 0 && lut_idx % mid_step == 0 && current_motion_idx != next_motion_idx)
    {
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
                     int end_pos[][6][3], int end_pos_idx)
{
  const int tick_step = TRANSITION_TICK_STEP;
  int max_step = 0;
  int signed_ticks[6][3];

  int current_pos[6][3];
  int diff;

  for (int leg_idx = 0; leg_idx < 6; leg_idx++)
  {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++)
    {
      diff = end_pos[end_pos_idx][leg_idx][joint_idx] - start_pos[start_pos_idx][leg_idx][joint_idx];
      current_pos[leg_idx][joint_idx] =
          start_pos[start_pos_idx][leg_idx][joint_idx];
      if (diff < 0)
      {
        signed_ticks[leg_idx][joint_idx] = -tick_step;
      }
      else
      {
        signed_ticks[leg_idx][joint_idx] = tick_step;
      }
      max_step = max(max_step, abs(diff));
    }
  }
  // Calculate number of steps needed (ceiling division ensures we reach target)
  max_step = (max_step + tick_step - 1) / tick_step;

  // Interpolate positions in small steps for smooth motion
  for (int step_idx = 0; step_idx < max_step; step_idx++)
  {
    for (int leg_idx = 0; leg_idx < 6; leg_idx++)
    {
      for (int joint_idx = 0; joint_idx < 3; joint_idx++)
      {
        // Update position towards target
        int remaining = abs(current_pos[leg_idx][joint_idx] - end_pos[end_pos_idx][leg_idx][joint_idx]);
        if (remaining > tick_step)
        {
          current_pos[leg_idx][joint_idx] += signed_ticks[leg_idx][joint_idx];
        }
        else
        {
          current_pos[leg_idx][joint_idx] =
              end_pos[end_pos_idx][leg_idx][joint_idx];
        }
      }
    }

    // Update all servos with new positions
    for (int leg_idx = 0; leg_idx < 3; leg_idx++)
    {
      for (int joint_idx = 0; joint_idx < 3; joint_idx++)
      {
        right_pwm.setPWM(right_legs[leg_idx][joint_idx], 0,
                         current_pos[leg_idx][joint_idx] + right_offset_ticks[leg_idx][joint_idx]);
        left_pwm.setPWM(left_legs[leg_idx][joint_idx], 0,
                        current_pos[leg_idx + 3][joint_idx] + left_offset_ticks[leg_idx][joint_idx]);
      }
    }
    delay(DELAY_MS / 2); // Smoother transition with shorter delays
  }
}

/**
   @brief Set all servo positions from a LUT entry.
   @param positions Array of positions for all 6 legs (3 joints each)
*/
void setAllServos(int positions[][3])
{
  for (int leg_idx = 0; leg_idx < 3; leg_idx++)
  {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++)
    {
      // Clamp after adding the calibration offset. LUT values are in range by
      // construction, but streamed poses are not, and an out-of-range tick
      // drives the servo into its mechanical stop where it stalls and heats.
      right_pwm.setPWM(right_legs[leg_idx][joint_idx], 0,
                       constrain(positions[leg_idx][joint_idx] +
                                     right_offset_ticks[leg_idx][joint_idx],
                                 SERVOMIN, SERVOMAX));
      left_pwm.setPWM(left_legs[leg_idx][joint_idx], 0,
                      constrain(positions[leg_idx + 3][joint_idx] +
                                    left_offset_ticks[leg_idx][joint_idx],
                                SERVOMIN, SERVOMAX));
    }
  }
}

/**
   @brief Copy a 6x3 pose.
*/
void copyPose(int src[6][3], int dst[6][3])
{
  for (int leg_idx = 0; leg_idx < 6; leg_idx++)
  {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++)
    {
      dst[leg_idx][joint_idx] = src[leg_idx][joint_idx];
    }
  }
}

/**
   @brief Compare two 6x3 poses for exact equality.
*/
bool posesEqual(int a[6][3], int b[6][3])
{
  for (int leg_idx = 0; leg_idx < 6; leg_idx++)
  {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++)
    {
      if (a[leg_idx][joint_idx] != b[leg_idx][joint_idx])
      {
        return false;
      }
    }
  }
  return true;
}

/**
   @brief Enable or disable both PWM drivers (enable pins are active LOW).
*/
void setPwmEnabled(bool enabled)
{
  digitalWrite(LEFT_PWM_ENABLE_PIN, enabled ? LOW : HIGH);
  digitalWrite(RIGHT_PWM_ENABLE_PIN, enabled ? LOW : HIGH);
}
