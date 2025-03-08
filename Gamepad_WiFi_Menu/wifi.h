#pragma once
#include "Arduino.h"

void ssid_pswd_read();
void ssid_pswd_write(String input_ssid, String input_pswd);

void prefix_init();
uint8_t prefix_matching(String temp_ssid);

void init_wifi();
void scan_wifi();
void inseart_designated_ssid_eeprom();
void reconnect_wifi();

String get_scaned_ssid(int i);
int get_num_scaned_ssid();
