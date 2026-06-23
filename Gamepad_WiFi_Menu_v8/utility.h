#pragma once

#include "Arduino.h"

typedef enum
{
  OPERATION_MODE_CONTROL=0,
  OPERATION_MODE_MENU=1,
  OPERATION_MODE_SSID_SCAN=2,
  OPERATION_MODE_PID_TUNING=3,
  OPERATION_MODE_TESTING=4,
  OPERATION_MODE_CALIBRATION=5,
  OPERATION_MODE_CHANNEL_SETUP=6,
  OPERATION_MODE_MODEL_SETUP=7,
  OPERATION_MODE_RATE_EXPO_SETUP=8,
  OPERATION_MODE_CHANNEL_MONITOR=9,
  OPERATION_MODE_MAX
} operation_mode_enum;

void set_operation_mode(operation_mode_enum target_op_mode);
operation_mode_enum get_operation_mode();

void init_eeprom();
void init_i2c();
void init_udp();
void write_eeprom(int i, int ele);
int read_eeprom(int i);
void eeprom_write_block(int addr, const uint8_t* data, int len);
void eeprom_read_block(int addr, uint8_t* data, int len);

void send_udp_msg(String msg);
void send_udp_msg(const char* msg);
