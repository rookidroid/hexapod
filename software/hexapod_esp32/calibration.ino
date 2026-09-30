/**

  Calibration storage -- servo offsets persisted in NVS flash

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

*/

#include <EEPROM.h>
#include <Preferences.h>

#include "hexapod.h"

// Stored as one blob so a save is all-or-nothing. `version` guards against
// reading a layout written by a different firmware.
struct CalibrationBlob
{
  uint8_t version;
  int16_t left[3][3];
  int16_t right[3][3];
};

/**
   @brief Limit an offset to the range a real installation error can need.
*/
int clampOffset(int offset)
{
  return constrain(offset, -CALIBRATION_MAX_OFFSET, CALIBRATION_MAX_OFFSET);
}

/**
   @brief Restore the saved offsets.

   Must run before the servos are driven: every position written from then on
   is shifted by these offsets.
*/
void setupCalibration()
{
  loadOffsets();
}

/**
   @brief Read offsets saved by older firmware in the emulated EEPROM.
   @return true if a valid calibration was found and loaded
*/
static bool loadLegacyEepromOffsets()
{
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    return false;
  }

  // Read magic number to verify EEPROM has valid data
  const uint16_t magic =
      (EEPROM.read(EEPROM_ADDR_MAGIC) << 8) | EEPROM.read(EEPROM_ADDR_MAGIC + 1);
  if (magic != EEPROM_MAGIC)
  {
    EEPROM.end();
    return false;
  }

  int left_addr = EEPROM_ADDR_LEFT;
  int right_addr = EEPROM_ADDR_RIGHT;
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      const int16_t left = (EEPROM.read(left_addr) << 8) | EEPROM.read(left_addr + 1);
      const int16_t right = (EEPROM.read(right_addr) << 8) | EEPROM.read(right_addr + 1);
      left_offset_ticks[i][j] = clampOffset(left);
      right_offset_ticks[i][j] = clampOffset(right);
      left_addr += 2;
      right_addr += 2;
    }
  }

  EEPROM.end();
  return true;
}

/**
   @brief Load servo offset values from NVS, migrating an old EEPROM
   calibration on first boot of this firmware.
*/
void loadOffsets()
{
  Preferences prefs;
  CalibrationBlob blob;

  prefs.begin(CALIBRATION_NVS_NAMESPACE, true);
  const size_t read = prefs.getBytes(CALIBRATION_NVS_KEY, &blob, sizeof(blob));
  prefs.end();

  if (read == sizeof(blob) && blob.version == CALIBRATION_VERSION)
  {
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        left_offset_ticks[i][j] = clampOffset(blob.left[i][j]);
        right_offset_ticks[i][j] = clampOffset(blob.right[i][j]);
      }
    }
    Serial.println("Calibration offsets loaded from flash");
    return;
  }

  if (loadLegacyEepromOffsets())
  {
    Serial.println("Migrating calibration offsets from EEPROM");
    saveOffsets();
    return;
  }

  Serial.println("No saved calibration, using defaults from config.h");
}

/**
   @brief Save servo offset values to NVS.
   @return true on success
*/
bool saveOffsets()
{
  CalibrationBlob blob;
  blob.version = CALIBRATION_VERSION;
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      blob.left[i][j] = left_offset_ticks[i][j];
      blob.right[i][j] = right_offset_ticks[i][j];
    }
  }

  Preferences prefs;
  bool ok = prefs.begin(CALIBRATION_NVS_NAMESPACE, false);
  ok = ok && prefs.putBytes(CALIBRATION_NVS_KEY, &blob, sizeof(blob)) == sizeof(blob);
  prefs.end();

  Serial.println(ok ? "Calibration offsets saved to flash"
                    : "ERROR: Failed to save calibration offsets");
  return ok;
}
