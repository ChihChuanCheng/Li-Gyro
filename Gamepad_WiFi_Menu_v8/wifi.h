#pragma once
#include "Arduino.h"

void ssid_pswd_read();
void ssid_pswd_write(const char* input_ssid, const char* input_pswd);

void prefix_init();
uint8_t prefix_matching(const char* temp_ssid);

void init_wifi();
void service_wifi();
void scan_wifi();
void inseart_designated_ssid_eeprom();
void reconnect_wifi();

String get_scaned_ssid(int i);
int get_num_scaned_ssid();
const char* get_current_ssid();
const char* get_current_password();
bool is_wifi_connected();
const char* get_connected_ssid();
