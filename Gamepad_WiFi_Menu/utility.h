#pragma once

#include "Arduino.h"

/* Gamepad operation mode */
typedef enum
{
  OPERATION_MODE_CONTROL=0,
  OPERATION_MODE_MENU=1,
  OPERATION_MODE_SSID_SCAN=2,
  OPERATION_MODE_PID_TUNING=3,
  OPERATION_MODE_TESTING=4,
  OPERATION_MODE_CALIBRATION=5,
  OPERATION_MODE_MAX
} operation_mode_enum;

void set_operation_mode(operation_mode_enum target_op_mode);
operation_mode_enum get_operation_mode();


/* EEPROM */
void init_eeprom();
void init_udp();
void write_eeprom(int i, int ele);
int read_eeprom(int i);


/* UDP */
void send_udp_msg(String msg);
