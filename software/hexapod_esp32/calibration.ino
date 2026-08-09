/**

  Calibration storage -- servo offsets persisted in EEPROM

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

*/

#include <EEPROM.h>

#include "hexapod.h"

/**
   @brief Open the EEPROM and restore the saved offsets.

   Must run before the servos are driven: every position written from then on
   is shifted by these offsets.
*/
void setupCalibration()
{
  EEPROM.begin(EEPROM_SIZE);
  Serial.println("EEPROM initialized");
  loadOffsetsFromEEPROM();
}

/**
   @brief Load servo offset values from EEPROM.
*/
void loadOffsetsFromEEPROM()
{
  // Read magic number to verify EEPROM has valid data
  uint16_t magic = (EEPROM.read(EEPROM_ADDR_MAGIC) << 8) | EEPROM.read(EEPROM_ADDR_MAGIC + 1);

  if (magic != EEPROM_MAGIC)
  {
    Serial.println("No valid calibration data in EEPROM, using defaults from config.h");
    return;
  }

  Serial.println("Loading calibration offsets from EEPROM...");

  // Read left offsets
  int addr = EEPROM_ADDR_LEFT;
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      int16_t value = (EEPROM.read(addr) << 8) | EEPROM.read(addr + 1);
      left_offset_ticks[i][j] = value;
      addr += 2;
    }
  }

  // Read right offsets
  addr = EEPROM_ADDR_RIGHT;
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      int16_t value = (EEPROM.read(addr) << 8) | EEPROM.read(addr + 1);
      right_offset_ticks[i][j] = value;
      addr += 2;
    }
  }

  Serial.println("Calibration offsets loaded from EEPROM");
}

/**
   @brief Save servo offset values to EEPROM.
*/
void saveOffsetsToEEPROM()
{
  Serial.println("Saving calibration offsets to EEPROM...");

  // Write magic number
  EEPROM.write(EEPROM_ADDR_MAGIC, (EEPROM_MAGIC >> 8) & 0xFF);
  EEPROM.write(EEPROM_ADDR_MAGIC + 1, EEPROM_MAGIC & 0xFF);

  // Write left offsets
  int addr = EEPROM_ADDR_LEFT;
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      int16_t value = left_offset_ticks[i][j];
      EEPROM.write(addr, (value >> 8) & 0xFF);
      EEPROM.write(addr + 1, value & 0xFF);
      addr += 2;
    }
  }

  // Write right offsets
  addr = EEPROM_ADDR_RIGHT;
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      int16_t value = right_offset_ticks[i][j];
      EEPROM.write(addr, (value >> 8) & 0xFF);
      EEPROM.write(addr + 1, value & 0xFF);
      addr += 2;
    }
  }

  // Commit changes to EEPROM
  if (EEPROM.commit())
  {
    Serial.println("Calibration offsets saved to EEPROM successfully");
  }
  else
  {
    Serial.println("ERROR: Failed to save offsets to EEPROM");
  }
}
