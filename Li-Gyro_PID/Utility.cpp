#include "utility.h"
#include "Options.h"
#include "Arduino.h"
#include <EEPROM.h>

/* initialize EEPROM */
void init_eeprom()
{
  /* ready to use the first 120 elements */
  EEPROM.begin(120);
}

/************************************
 *            EEPROM                *
 ************************************/
int read_eeprom(int i)
{
  Serial.print("EEPROM.read(): ");
  Serial.println(EEPROM.read(i));
  return EEPROM.read(i);
}

void write_eeprom(int i, int ele)
{
  Serial.print("EEPROM.write(): ");
  Serial.println(i, ele);
  EEPROM.write(i, ele);
  EEPROM.commit();
}
