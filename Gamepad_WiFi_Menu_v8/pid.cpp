#include "Options.h"
#include "Arduino.h"
#include "pid.h"
#include "utility.h"

static const int PID_STORE_ADDR = 120;
static const uint8_t PID_STORE_MAGIC0 = 'P';
static const uint8_t PID_STORE_MAGIC1 = '4';
static const uint8_t PID_STORE_VERSION = 1;
static const uint8_t PID_COUNT = 9;
static const uint32_t PID_DEFAULT_VALUE = 99999UL;
static const uint32_t PID_MAX_VALUE = 100000UL;
static const uint32_t PID_DEFAULT_VALUES[PID_COUNT] = {
  1000UL, 0UL, 0UL,
  1000UL, 0UL, 0UL,
  1000UL, 0UL, 0UL
};

uint32_t pid_default_value(int idx)
{
  if ((idx < 0) || (idx >= PID_COUNT))
  {
    return 0;
  }

  return PID_DEFAULT_VALUES[idx];
}

static bool pid_store_valid()
{
  return (read_eeprom(PID_STORE_ADDR) == PID_STORE_MAGIC0) &&
         (read_eeprom(PID_STORE_ADDR + 1) == PID_STORE_MAGIC1) &&
         (read_eeprom(PID_STORE_ADDR + 2) == PID_STORE_VERSION) &&
         (read_eeprom(PID_STORE_ADDR + 3) == PID_COUNT);
}

void pid_write(uint32_t pid_array[], int size)
{
  uint8_t buffer[4 + PID_COUNT * 4] = {0};
  int count = min(size, (int)PID_COUNT);

#ifdef __LOG__
  Serial.println("Writing PID");
#endif

  buffer[0] = PID_STORE_MAGIC0;
  buffer[1] = PID_STORE_MAGIC1;
  buffer[2] = PID_STORE_VERSION;
  buffer[3] = PID_COUNT;

  for (int i = 0; i < count; i++)
  {
    uint32_t value = (pid_array[i] <= PID_MAX_VALUE) ? pid_array[i] : PID_DEFAULT_VALUE;
    int offset = 4 + i * 4;
    buffer[offset] = (value >> 24) & 0xFF;
    buffer[offset + 1] = (value >> 16) & 0xFF;
    buffer[offset + 2] = (value >> 8) & 0xFF;
    buffer[offset + 3] = value & 0xFF;
  }

  eeprom_write_block(PID_STORE_ADDR, buffer, sizeof(buffer));
}

uint32_t pid_read(int idx)
{
  if ((idx < 0) || (idx >= PID_COUNT) || !pid_store_valid())
  {
    return PID_DEFAULT_VALUE;
  }

  uint8_t data[4];
  eeprom_read_block(PID_STORE_ADDR + 4 + idx * 4, data, sizeof(data));

  uint32_t val = ((uint32_t)data[0] << 24) |
                 ((uint32_t)data[1] << 16) |
                 ((uint32_t)data[2] << 8) |
                 (uint32_t)data[3];

  if (val > PID_MAX_VALUE)
  {
    val = PID_DEFAULT_VALUE;
  }

  return val;
}
