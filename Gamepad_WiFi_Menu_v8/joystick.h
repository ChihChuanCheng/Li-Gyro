#pragma once

void init_joystick();

void get_joystick_status();

int16_t get_l_pos_y();
int16_t get_l_offset_y();
int16_t get_l_pos_y_v();

int16_t get_l_pos_x();
int16_t get_l_offset_x();
int16_t get_l_pos_x_v();

int16_t get_r_pos_y();
int16_t get_r_offset_y();
int16_t get_r_pos_y_v();

int16_t get_r_pos_x();
int16_t get_r_offset_x();
int16_t get_r_pos_x_v();

int16_t cal_read(int idx);
void cal_write(int16_t cal_array[], int size);
void read_cal_from_flash();
void cal_full_write(int16_t center_array[], int16_t min_array[], int16_t max_array[], int size);

void set_cal_l_x_offset(int16_t x_offset);
void set_cal_l_y_offset(int16_t y_offset);
void set_cal_r_x_offset(int16_t x_offset);
void set_cal_r_y_offset(int16_t y_offset);

int16_t get_cal_l_x_offset();
int16_t get_cal_l_y_offset();
int16_t get_cal_r_x_offset();
int16_t get_cal_r_y_offset();
int16_t get_cal_center(uint8_t idx);
int16_t get_cal_min(uint8_t idx);
int16_t get_cal_max(uint8_t idx);
void set_cal_center(uint8_t idx, int16_t value);
void set_cal_min(uint8_t idx, int16_t value);
void set_cal_max(uint8_t idx, int16_t value);

byte get_ButtonStateJOYS();
byte get_ButtonStateJOYP();
byte get_ButtonStateSWA();
byte get_ButtonStateSWB();

byte get_ButtonPressedJOYS();
byte get_ButtonPressedJOYP();
byte get_ButtonPressedSWA();
byte get_ButtonPressedSWB();

byte get_ButtonReleasedJOYS();
byte get_ButtonReleasedJOYP();
byte get_ButtonReleasedSWA();
byte get_ButtonReleasedSWB();
