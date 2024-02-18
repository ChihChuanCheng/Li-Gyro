#pragma once

/* EEPROM */
void init_eeprom();
void write_eeprom(int i, int ele);
int read_eeprom(int i);
