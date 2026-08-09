/**

  Real-time pose streaming -- slew-limited playback of streamed servo poses

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

*/

#include "hexapod.h"
#include "motion.h"

// Shared with the AsyncUDP task; see the notes in hexapod.h.
portMUX_TYPE realtime_mux = portMUX_INITIALIZER_UNLOCKED;

bool realtime_mode = false;
int realtime_target[6][3];
int realtime_current[6][3];
uint16_t realtime_max_step = REALTIME_DEFAULT_MAX_STEP;
bool realtime_snap = false;
bool realtime_returning = false;
unsigned long realtime_last_packet_time = 0;

/**
   @brief Enter real-time pose streaming mode.

   Seeds the working pose from standby so the first streamed pose is reached by
   a rate-limited slew from a known posture instead of an unpredictable jump.
*/
void enterRealtimeMode()
{
  if (realtime_mode)
  {
    return;
  }

  setPwmEnabled(true);

  portENTER_CRITICAL(&realtime_mux);
  copyPose(lut_standby[0], realtime_current);
  copyPose(lut_standby[0], realtime_target);
  realtime_snap = false;
  realtime_returning = false;
  realtime_mode = true;
  portEXIT_CRITICAL(&realtime_mux);

  realtime_last_packet_time = millis();

  // The streaming loop must not be delayed by OTA polling.
  ota_mode = false;

  Serial.println("Entered real-time mode");
}

/**
   @brief Leave real-time mode and hand control back to the motion LUT engine.
*/
void exitRealtimeMode()
{
  if (!realtime_mode)
  {
    return;
  }

  portENTER_CRITICAL(&realtime_mux);
  realtime_mode = false;
  realtime_returning = false;
  portEXIT_CRITICAL(&realtime_mux);

  // The LUT engine resumes from standby, which is the posture we eased into.
  current_motion_idx = CMD_STANDBY;
  next_motion_idx = CMD_STANDBY;
  last_udp_packet_time = millis();

  Serial.println("Exited real-time mode");
}

/**
   @brief Advance the streamed pose by one control cycle and drive the servos.

   Each joint moves toward its target by at most `realtime_max_step` ticks, so
   a large jump in the commanded pose becomes a controlled slew rather than a
   step input to 18 servos at once.
*/
void serviceRealtimePose()
{
  int target[6][3];
  uint16_t step;
  bool snap;

  portENTER_CRITICAL(&realtime_mux);
  copyPose(realtime_target, target);
  step = realtime_max_step;
  snap = realtime_snap;
  realtime_snap = false; // One-shot
  portEXIT_CRITICAL(&realtime_mux);

  if (step == 0)
  {
    step = REALTIME_DEFAULT_MAX_STEP;
  }

  for (int leg_idx = 0; leg_idx < 6; leg_idx++)
  {
    for (int joint_idx = 0; joint_idx < 3; joint_idx++)
    {
      int diff = target[leg_idx][joint_idx] - realtime_current[leg_idx][joint_idx];

      if (snap || abs(diff) <= (int)step)
      {
        realtime_current[leg_idx][joint_idx] = target[leg_idx][joint_idx];
      }
      else
      {
        realtime_current[leg_idx][joint_idx] += (diff > 0) ? (int)step : -(int)step;
      }
    }
  }

  setAllServos(realtime_current);
}
