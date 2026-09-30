/**

  Real-time pose streaming -- slew-limited playback of streamed servo poses

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

*/

#include "hexapod.h"

// Shared with the AsyncUDP task; see the notes in hexapod.h.
portMUX_TYPE realtime_mux = portMUX_INITIALIZER_UNLOCKED;

volatile bool realtime_mode = false;
volatile bool relax_requested = false;
Pose realtime_target;
bool realtime_target_valid = false;
uint16_t realtime_max_step = REALTIME_DEFAULT_MAX_STEP;
bool realtime_snap = false;
volatile unsigned long realtime_last_packet_time = 0;

/**
   @brief Request real-time pose streaming mode (UDP task).

   The motion engine picks the request up at its next call and holds the
   current pose until a streamed pose arrives, so entering never moves the
   robot by itself. Also clears a pending relax.
*/
void enterRealtimeMode()
{
  relax_requested = false;
  realtime_last_packet_time = millis();

  if (realtime_mode)
  {
    return;
  }

  portENTER_CRITICAL(&realtime_mux);
  realtime_target_valid = false;
  realtime_snap = false;
  realtime_mode = true;
  portEXIT_CRITICAL(&realtime_mux);

  // The streaming loop must not be delayed by OTA polling.
  ota_mode = false;
}

/**
   @brief Request a return to LUT playback (UDP task or motion engine).

   The LUT engine eases from wherever the legs are back to standby.
*/
void exitRealtimeMode()
{
  if (!realtime_mode)
  {
    return;
  }

  portENTER_CRITICAL(&realtime_mux);
  realtime_mode = false;
  realtime_target_valid = false;
  portEXIT_CRITICAL(&realtime_mux);

  next_motion_idx = CMD_STANDBY;
  last_udp_packet_time = millis();
}

/**
   @brief Advance the streamed pose by one control cycle and drive the servos.

   Each joint moves toward its target by at most `realtime_max_step` ticks, so
   a large jump in the commanded pose becomes a controlled slew rather than a
   step input to 18 servos at once.
*/
void serviceRealtimePose()
{
  // Stream dropped: hand back to the LUT engine, which eases to standby rather
  // than freezing mid-pose or snapping.
  const unsigned long last_packet = realtime_last_packet_time;
  if ((long)(millis() - last_packet) > REALTIME_TIMEOUT_MS)
  {
    Serial.println("Real-time stream lost, returning to standby");
    exitRealtimeMode();
    return;
  }

  Pose target;
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

  Pose pose;
  copyPose(pose_current, pose);
  // A snap moves every joint the whole way, whatever the distance.
  slewToward(pose, target, snap ? SERVOMAX : step);
  writePose(pose);
}
