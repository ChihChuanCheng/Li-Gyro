#include "Options.h"
#include "Arduino.h"
#include "pid.h"
#include "utility.h"

void pid_write(uint16_t pid_array[], int size)
{
#ifdef __LOG__
  Serial.println("Writing PID");
#endif
  /* Write PID to EEPROM 40~120 */
  for (int i = 0; i < size; i++)
  {
    write_eeprom((40+i*5), pid_array[i] >> 8);
    write_eeprom((40+i*5)+1, pid_array[i]);
  }
}

uint16_t pid_read(int idx)
{
  byte byte1 = read_eeprom((40+idx*5));
  byte byte2 = read_eeprom((40+idx*5)+1);

  Serial.println(((byte1 << 8)+byte2));

  return ((byte1 << 8)+byte2);
}
