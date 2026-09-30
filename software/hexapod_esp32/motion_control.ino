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

// Motion configuration table - add new motions here. The index of each entry
// is its RobotCommand ID, so the order must match protocol.h.
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

static_assert(sizeof(motion_config) / sizeof(motion_config[0]) == CMD_COUNT,
              "motion_config[] needs exactly one entry per RobotCommand");

Pose pose_current;

// ============================================================================
// Motion engine state
// ============================================================================
// The engine is ticked from loop() and never blocks: each call does at most one
// control step, so the web interface, OTA and the failsafes stay responsive
// while the robot walks. Its mode and phase types live in hexapod.h.

static EngineMode engine_mode = ENGINE_LUT;
static LutPhase lut_phase = LUT_TO_STANDBY;
static int active_motion = CMD_STANDBY;
static int frame_idx = 0;
static unsigned long last_tick_ms = 0;

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
  left_pwm.setOscillatorFrequency(PCA9685_OSC_HZ);
  left_pwm.setPWMFreq(SERVO_PWM_FREQ);

  right_pwm.begin();
  right_pwm.setOscillatorFrequency(PCA9685_OSC_HZ);
  right_pwm.setPWMFreq(SERVO_PWM_FREQ);

  // After begin(), which (re)starts the bus at its default speed.
  Wire.setClock(I2C_CLOCK_HZ);
  Serial.println("PWM drivers initialized");

  // Configure PWM driver enable pins (active LOW)
  pinMode(LEFT_PWM_ENABLE_PIN, OUTPUT);
  pinMode(RIGHT_PWM_ENABLE_PIN, OUTPUT);

  setPwmEnabled(true);
}

/**
   @brief Write one servo, applying its calibration offset and range clamp.
   @param leg_idx Leg in LUT order (0-2 right, 3-5 left)
   @param joint_idx Joint (coxa, femur, tibia)
   @param ticks Uncalibrated position in servo ticks
*/
void writeServo(int leg_idx, int joint_idx, int ticks)
{
  // Clamp after adding the calibration offset. LUT values are in range by
  // construction, but streamed poses are not, and an out-of-range tick drives
  // the servo into its mechanical stop where it stalls and heats.
  if (leg_idx < 3)
  {
    right_pwm.setPWM(right_legs[leg_idx][joint_idx], 0,
                     constrain(ticks + right_offset_ticks[leg_idx][joint_idx],
                               SERVOMIN, SERVOMAX));
  }
  else
  {
    left_pwm.setPWM(left_legs[leg_idx - 3][joint_idx], 0,
                    constrain(ticks + left_offset_ticks[leg_idx - 3][joint_idx],
                              SERVOMIN, SERVOMAX));
  }
  pose_current[leg_idx][joint_idx] = ticks;
}

/**
   @brief Write a whole-body pose to all 18 servos.
   @param pose Positions for all 6 legs (3 joints each)
*/
void writePose(const Pose pose)
{
  for (int leg_idx = 0; leg_idx < 3; leg_idx++)
  {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++)
    {
      writeServo(leg_idx, joint_idx, pose[leg_idx][joint_idx]);
      writeServo(leg_idx + 3, joint_idx, pose[leg_idx + 3][joint_idx]);
    }
  }
}

/**
   @brief The calibration posture: every joint at its mechanical middle.
*/
static void neutralPose(Pose pose)
{
  for (int leg_idx = 0; leg_idx < 6; leg_idx++)
  {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++)
    {
      pose[leg_idx][joint_idx] = SERVOMID;
    }
  }
}

/**
   @brief Set all servos to neutral position using calibration offsets.

   Jumps straight there. Once the motion engine runs, calibration mode eases
   into this posture instead.
*/
void posture_calibration()
{
  Pose neutral;
  neutralPose(neutral);
  writePose(neutral);
}

/**
   @brief Execute boot sequence to stand up the robot.

   Blocking, but it runs once, before the motion engine takes over.
   @param lut_size Number of steps in the motion sequence
   @param lut Look-up table with servo positions for each step
*/
void boot_up_motion(int lut_size, const Pose lut[])
{
  Serial.println("Starting boot sequence...");

  // Phase 1: Initialize servos to starting position
  // Gradual activation prevents current spikes and sudden movements
  for (int leg_idx = 0; leg_idx < 3; leg_idx++)
  {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++)
    {
      writeServo(leg_idx, joint_idx, lut[0][leg_idx][joint_idx]);
      delay(SERVO_INIT_DELAY_MS);
      writeServo(leg_idx + 3, joint_idx, lut[0][leg_idx + 3][joint_idx]);
      delay(SERVO_INIT_DELAY_MS);
    }
  }

  // Phase 2: Execute stand-up motion sequence
  // Step through each position in the LUT to stand up robot
  for (int lut_idx = 0; lut_idx < lut_size; lut_idx++)
  {
    writePose(lut[lut_idx]);
    delay(DELAY_MS);
  }

  Serial.println("Boot sequence complete");
}

/**
   @brief Move every joint of `pose` toward `target` by at most `step` ticks.
   @return true once `pose` equals `target`
*/
bool slewToward(Pose pose, const Pose target, int step)
{
  bool reached = true;
  for (int leg_idx = 0; leg_idx < 6; leg_idx++)
  {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++)
    {
      const int diff = target[leg_idx][joint_idx] - pose[leg_idx][joint_idx];
      if (abs(diff) <= step)
      {
        pose[leg_idx][joint_idx] = target[leg_idx][joint_idx];
      }
      else
      {
        pose[leg_idx][joint_idx] += (diff > 0) ? step : -step;
        reached = false;
      }
    }
  }
  return reached;
}

/**
   @brief Copy a 6x3 pose.
*/
void copyPose(const Pose src, Pose dst)
{
  memcpy(dst, src, sizeof(Pose));
}

/**
   @brief Enable or disable both PWM drivers (enable pins are active LOW).
*/
void setPwmEnabled(bool enabled)
{
  digitalWrite(LEFT_PWM_ENABLE_PIN, enabled ? LOW : HIGH);
  digitalWrite(RIGHT_PWM_ENABLE_PIN, enabled ? LOW : HIGH);
}

/**
   @brief Slew the servos one step from where they are toward `target`.
   @return true once the target is reached
*/
static bool slewAndWrite(const Pose target, int step)
{
  Pose pose;
  copyPose(pose_current, pose);
  const bool reached = slewToward(pose, target, step);
  writePose(pose);
  return reached;
}

/**
   @brief The mode the requests from the UDP task and web interface ask for.
*/
static EngineMode requestedMode()
{
  if (calibration_mode)
  {
    return ENGINE_CALIBRATION;
  }
  if (relax_requested)
  {
    return ENGINE_RELAXED;
  }
  if (realtime_mode)
  {
    return ENGINE_REALTIME;
  }
  return ENGINE_LUT;
}

/**
   @brief Switch the engine to `mode`, running the entry action for it.
*/
static void enterEngineMode(EngineMode mode)
{
  if (engine_mode == ENGINE_RELAXED)
  {
    setPwmEnabled(true);
  }

  switch (mode)
  {
  case ENGINE_LUT:
    // Resume from wherever the last mode left the legs.
    lut_phase = LUT_TO_STANDBY;
    Serial.println("Motion engine: LUT playback");
    break;
  case ENGINE_REALTIME:
    // Hold the current pose until the first streamed pose arrives, so entering
    // real-time mode never moves the robot by itself.
    portENTER_CRITICAL(&realtime_mux);
    if (!realtime_target_valid)
    {
      copyPose(pose_current, realtime_target);
    }
    portEXIT_CRITICAL(&realtime_mux);
    Serial.println("Motion engine: real-time streaming");
    break;
  case ENGINE_CALIBRATION:
    Serial.println("Motion engine: calibration");
    break;
  case ENGINE_RELAXED:
    setPwmEnabled(false);
    Serial.println("Motion engine: servos relaxed");
    break;
  }

  engine_mode = mode;
}

/**
   @brief One LUT playback step: play a frame, or slew toward the next motion.
*/
static void tickLut()
{
  // Failsafe: the operator must keep sending, or the robot stops. The signed
  // difference tolerates the UDP task stamping a time newer than `now`.
  const unsigned long last_packet = last_udp_packet_time;
  if (last_packet > 0 && (long)(millis() - last_packet) > MOTION_TIMEOUT_MS)
  {
    next_motion_idx = CMD_STANDBY;
  }

  const int requested = next_motion_idx;

  if (lut_phase == LUT_PLAYING)
  {
    const MotionConfig &motion = motion_config[active_motion];

    // Leave a gait only at its start or midpoint, the stable points of the
    // cycle.
    const bool at_switch_point =
        frame_idx == 0 || frame_idx == motion.length / 2;
    if (requested == active_motion || !at_switch_point)
    {
      writePose(motion.lut[frame_idx]);
      frame_idx = (frame_idx + 1) % motion.length;
      return;
    }
    lut_phase = LUT_TO_STANDBY;
  }

  if (lut_phase == LUT_TO_STANDBY)
  {
    if (slewAndWrite(lut_standby[0], TRANSITION_TICK_STEP))
    {
      active_motion = requested;
      lut_phase = LUT_TO_START;
    }
    return;
  }

  // LUT_TO_START
  if (slewAndWrite(motion_config[active_motion].lut[0], TRANSITION_TICK_STEP))
  {
    frame_idx = 0;
    lut_phase = LUT_PLAYING;
  }
}

/**
   @brief Control period of the current mode (ms).
*/
static unsigned long tickPeriod()
{
  if (engine_mode == ENGINE_REALTIME)
  {
    return REALTIME_PERIOD_MS;
  }
  // Transitions step at twice the frame rate for a smoother slew.
  if (engine_mode == ENGINE_CALIBRATION || lut_phase != LUT_PLAYING)
  {
    return DELAY_MS / 2;
  }
  // Only gait playback follows the speed setting; transitions keep their pace
  // so stopping and changing gait stay responsive.
  const unsigned long speed_pct = motion_speed_pct;
  return (unsigned long)DELAY_MS * 100 / speed_pct;
}

/**
   @brief Run the motion engine. Call from loop() as often as possible.

   Acts on mode requests immediately and runs one control step whenever the
   current mode's period has elapsed. Returns without blocking otherwise.
*/
void serviceMotionEngine()
{
  const EngineMode requested = requestedMode();
  if (requested != engine_mode)
  {
    enterEngineMode(requested);
  }

  // Fixed-rate ticks, so loop() jitter does not stretch the frame period. After
  // a long stall (a slow web request, say) resync instead of bursting frames.
  const unsigned long now = millis();
  const unsigned long period = tickPeriod();
  if (now - last_tick_ms < period)
  {
    return;
  }
  last_tick_ms = (now - last_tick_ms < 2 * period) ? last_tick_ms + period : now;

  switch (engine_mode)
  {
  case ENGINE_LUT:
    tickLut();
    break;
  case ENGINE_REALTIME:
    serviceRealtimePose();
    break;
  case ENGINE_CALIBRATION:
  {
    // Ease into the calibration posture, then keep rewriting it so offsets
    // edited on the web page take effect at the next tick.
    Pose neutral;
    neutralPose(neutral);
    slewAndWrite(neutral, TRANSITION_TICK_STEP);
    break;
  }
  case ENGINE_RELAXED:
    break;
  }
}
