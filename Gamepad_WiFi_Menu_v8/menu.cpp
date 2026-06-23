#include "Arduino.h"
#include "Options.h"
#include "menu.h"
#include "menu_shared.h"
#include "wifi.h"
#include "joystick.h"
#include "oled.h"
#include "utility.h"
#include "pid.h"
#include "Sensors.h"
#include "channel.h"

static unsigned long trigger_timer = 0;
static const unsigned long buffer_time = 500;
static unsigned long trigger_enable_time = 0;

int menu_cursor = 0;
int menu_current_page = 1;
int menu_page_scale = 5;
static const int MENU_ITEM_COUNT = 9;
bool showmenu = false;
bool is_init_menu_tuning = false;
static bool menu_confirm_ready = false;
static int menu_nav_state = 0;
static int menu_select_state = 0;

bool is_apply_manual_ssid = false;
bool show_ssid_menu = false;
int num_found_ssid = 0;
int current_page = 1;
int page_scale = 5;
int ssid_cursor = 0;
static int ssid_nav_state = 0;
static int ssid_select_state = 0;
static bool ssid_password_editing = false;
static int ssid_password_pos = 0;
static char ssid_password[32] = {0};
static int ssid_password_nav_state = 0;
static int ssid_password_select_state = 0;
static const char WIFI_PASSWORD_CHARS[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";
static const int WIFI_PASSWORD_CHAR_COUNT = sizeof(WIFI_PASSWORD_CHARS) - 1;

bool show_pid_menu = false;
bool is_init_pid_tuning = false;
bool is_pid_tuning = false;
int pid_cursor = 0;
int scale = 50;
uint32_t max_pid = 100000;
static const uint32_t PID_UNSET_VALUE = 99999UL;
static const int pid_scale_options[] = {10, 50, 500};
static const uint8_t pid_scale_option_count = sizeof(pid_scale_options) / sizeof(pid_scale_options[0]);
static uint8_t pid_scale_idx = 1;
uint8_t num_pid = 9;
#define PID_MENU_EXIT_IDX 9
uint32_t pid_list[9]={1000,0,0,1000,0,0,1000,0,0};
int pid_current_page = 1;
int pid_page_scale = 3;
static int pid_nav_state = 0;
static int pid_select_state = 0;

int testing_cursor = 0;
int testing_current_page = 1;
int testing_page_scale = 6;
bool is_testing_tuning = false;
bool is_init_testing_tuning = false;
bool show_testing_menu = false;
static int testing_nav_state = 0;
static int testing_select_state = 0;

int channel_cursor = 0;
int channel_current_page = 1;
int channel_page_scale = 4;
bool show_channel_menu = false;
bool is_init_channel_setup = false;
bool is_channel_editing = false;
static int channel_nav_state = 0;
static int channel_select_state = 0;
static const int CHANNEL_SETUP_FIELDS_PER_CHANNEL = 5;
static const int CHANNEL_SELECT_EXIT_IDX = CHANNEL_COUNT;
static const int CHANNEL_FIELD_SAVE_EXIT_IDX = CHANNEL_SETUP_FIELDS_PER_CHANNEL;
static bool is_channel_selected = false;
static uint8_t selected_channel_index = 0;

int model_cursor = 0;
bool is_init_model_setup = false;
bool show_model_menu = false;
static bool is_model_name_editing = false;
static int model_nav_state = 0;
static int model_select_state = 0;
static const int MODEL_NAME_ITEM_IDX = MODEL_COUNT;
static const int MODEL_EXIT_ITEM_IDX = MODEL_COUNT + 1;

int monitor_cursor = 0;
bool is_init_channel_monitor = false;
static int monitor_nav_state = 0;
static int monitor_select_state = 0;

int rate_cursor = 0;
bool is_init_rate_setup = false;
bool is_rate_editing = false;
bool show_rate_menu = false;
static int rate_nav_state = 0;
static int rate_select_state = 0;
static const int RATE_SETUP_ITEM_COUNT = 4;
static const int RATE_SETUP_SAVE_EXIT_IDX = RATE_SETUP_ITEM_COUNT - 1;

bool is_show_message = false;
bool is_centering = false;
bool is_init_calibaration_tuning = false;
static uint8_t calibration_phase = 0;
static int16_t cal_center_work[4] = {166, 166, 166, 166};
static int16_t cal_min_work[4] = {0, 0, 0, 0};
static int16_t cal_max_work[4] = {333, 333, 333, 333};

static bool is_confirm_input(int x_value)
{
  return 1750 < x_value;
}

static bool is_back_input(int x_value)
{
  return 1250 > x_value;
}

static int read_axis_step(int value, int& state)
{
  int direction = 0;

  if (1750 < value)
  {
    direction = -1;
  }
  else if (1250 > value)
  {
    direction = 1;
  }
  else
  {
    state = 0;
    return 0;
  }

  if (state == direction)
  {
    return 0;
  }

  state = direction;
  return direction;
}

static int read_select_step(int x_value, int& state)
{
  int direction = 0;

  if (is_confirm_input(x_value))
  {
    direction = 1;
  }
  else if (is_back_input(x_value))
  {
    direction = -1;
  }
  else
  {
    state = 0;
    return 0;
  }

  if (state == direction)
  {
    return 0;
  }

  state = direction;
  return direction;
}

static int clamp_cursor(int cursor, int min_value, int max_value)
{
  if (cursor < min_value) return min_value;
  if (cursor > max_value) return max_value;
  return cursor;
}

static void switch_to_operation_mode(const char* str)
{
#ifdef __OLED__
  setOledClear();
  setOledCursor(0, 0);
  setOledPrint(str);
#endif
}

static void switch_to_menu_mode()
{
  set_operation_mode(OPERATION_MODE_MENU);
  is_init_menu_tuning = false;
  switch_to_operation_mode("Entering Menu Mode...");
}

static void switch_to_control_mode(bool reconnect)
{
  set_operation_mode(OPERATION_MODE_CONTROL);
  switch_to_operation_mode("Entering Control Mode...");
  if (reconnect)
  {
    reconnect_wifi();
  }
}

static void print_main_menu_item(int item)
{
#ifdef __OLED__
  switch (item)
  {
    case 0: setOledPrint("SSID SCAN MODE"); break;
    case 1: setOledPrint("PID TUNING MODE"); break;
    case 2: setOledPrint("CHANNEL SETUP"); break;
    case 3: setOledPrint("CHANNEL MONITOR"); break;
    case 4: setOledPrint("MODEL SETUP"); break;
    case 5: setOledPrint("RATE/EXPO SETUP"); break;
    case 6: setOledPrint("TESTING MODE"); break;
    case 7: setOledPrint("CALIBRATION MODE"); break;
    case 8: setOledPrint("EXIT"); break;
    default: setOledPrint(""); break;
  }
#endif
}

void display_menu_oled()
{
#ifdef __OLED__
  char page_str[18];

  if (showmenu || ((menu_cursor / menu_page_scale) + 1) != menu_current_page)
  {
    setOledClear();
    setOledCursor(0, 0);
    menu_current_page = (menu_cursor / menu_page_scale) + 1;
    snprintf(page_str, sizeof(page_str), "[Page %d]", menu_current_page);
    setOledPrint(page_str);

    for (int i = 0; i < menu_page_scale; i++)
    {
      int item = (menu_cursor / menu_page_scale) * menu_page_scale + i;
      setOledCursor(20, i * 10 + 10);
      print_main_menu_item(item);
    }

    showmenu = false;
  }
  else
  {
    fillOledRect(0, 10, 20, 50);
    setOledCursor(0, (menu_cursor % menu_page_scale) * 10 + 10);
    setOledPrint(">");
  }
#endif
}

void update_menu_selection_status()
{
  menu_cursor = clamp_cursor(menu_cursor + read_axis_step(get_r_pos_y(), menu_nav_state), 0, MENU_ITEM_COUNT - 1);
}

void detect_menu_selection()
{
  int selection = read_select_step(get_r_pos_x(), menu_select_state);

  if (!menu_confirm_ready)
  {
    if (0 == selection)
    {
      menu_confirm_ready = true;
    }
    return;
  }

  if ((millis() - trigger_timer) <= buffer_time || 0 == selection)
  {
    return;
  }

  trigger_timer = millis();

  if (selection < 0)
  {
    switch_to_control_mode(true);
    is_init_menu_tuning = false;
    return;
  }

  switch(menu_cursor)
  {
    case 0:
      set_operation_mode(OPERATION_MODE_SSID_SCAN);
      is_init_menu_tuning = false;
      switch_to_operation_mode("Entering SSID SCAN Mode...");
      break;
    case 1:
      set_operation_mode(OPERATION_MODE_PID_TUNING);
      is_init_menu_tuning = false;
      switch_to_operation_mode("Entering PID Tuning Mode...");
      break;
    case 2:
      set_operation_mode(OPERATION_MODE_CHANNEL_SETUP);
      is_init_menu_tuning = false;
      switch_to_operation_mode("Entering Channel Setup...");
      break;
    case 3:
      set_operation_mode(OPERATION_MODE_CHANNEL_MONITOR);
      is_init_menu_tuning = false;
      switch_to_operation_mode("Entering CH Monitor...");
      break;
    case 4:
      set_operation_mode(OPERATION_MODE_MODEL_SETUP);
      is_init_menu_tuning = false;
      switch_to_operation_mode("Entering Model Setup...");
      break;
    case 5:
      set_operation_mode(OPERATION_MODE_RATE_EXPO_SETUP);
      is_init_menu_tuning = false;
      switch_to_operation_mode("Entering Rate/Expo...");
      break;
    case 6:
      set_operation_mode(OPERATION_MODE_TESTING);
      is_init_menu_tuning = false;
      switch_to_operation_mode("Entering Testing Mode...");
      break;
    case 7:
      set_operation_mode(OPERATION_MODE_CALIBRATION);
      is_init_menu_tuning = false;
      switch_to_operation_mode("Entering Calibration Mode...");
      break;
    default:
      is_init_menu_tuning = false;
      switch_to_control_mode(true);
      break;
  }
}

void menu_mode_handler()
{
  if (!is_init_menu_tuning)
  {
    menu_cursor = 0;
    menu_current_page = 1;
    menu_page_scale = 5;
    showmenu = true;
    menu_confirm_ready = false;
    menu_nav_state = 0;
    menu_select_state = 0;
    trigger_timer = millis();
    is_init_menu_tuning = true;
  }

  display_menu_oled();
  update_menu_selection_status();
  detect_menu_selection();
}

void display_scaned_ssid_oled()
{
#ifdef __OLED__
  char page_str[18];

  if (ssid_password_editing)
  {
    setOledClear();
    setOledCursor(0, 0);
    setOledPrint("WiFi Password");
    setOledCursor(0, 12);
    setOledPrint(get_scaned_ssid(ssid_cursor).substring(0, 16));
    setOledCursor(0, 28);
    setOledPrint(ssid_password);
    setOledCursor((ssid_password_pos % 16) * 6, 42);
    setOledPrint("^");
    setOledCursor(0, 54);
    setOledPrint("SWB Save");
    return;
  }

  if (show_ssid_menu || ((ssid_cursor / page_scale) + 1) != current_page)
  {
    setOledClear();
    setOledCursor(0, 0);
    current_page = (ssid_cursor / page_scale) + 1;
    snprintf(page_str, sizeof(page_str), "[Page %d]", current_page);
    setOledPrint(page_str);

    for (int i = 0; i < page_scale; i++)
    {
      int index = (ssid_cursor / page_scale) * page_scale + i;
      setOledCursor(10, i * 10 + 10);
      setOledPrint(get_scaned_ssid(index).substring(0, 9));
    }

    show_ssid_menu = false;
  }
  else
  {
    fillOledRect(0, 10, 10, 60);
  }

  setOledCursor(0, (ssid_cursor % page_scale) * 10 + 10);
  setOledPrint(">");
#endif
}

void update_ssid_selection_status()
{
  if (ssid_password_editing)
  {
    return;
  }

  int max_cursor = (0 < num_found_ssid) ? (num_found_ssid - 1) : 0;
  ssid_cursor = clamp_cursor(ssid_cursor + read_axis_step(get_r_pos_y(), ssid_nav_state), 0, max_cursor);
}

void trigger_ssid_connection()
{
  if (ssid_password_editing)
  {
    int selection = read_select_step(get_r_pos_x(), ssid_password_select_state);
    int step = read_axis_step(get_r_pos_y(), ssid_password_nav_state);
    if (0 != step)
    {
      char current = ssid_password[ssid_password_pos];
      int charIndex = 0;
      for (int i = 0; i < WIFI_PASSWORD_CHAR_COUNT; i++)
      {
        if (WIFI_PASSWORD_CHARS[i] == current)
        {
          charIndex = i;
          break;
        }
      }

      charIndex = (charIndex + ((step < 0) ? 1 : -1) + WIFI_PASSWORD_CHAR_COUNT) % WIFI_PASSWORD_CHAR_COUNT;
      ssid_password[ssid_password_pos] = WIFI_PASSWORD_CHARS[charIndex];
    }

    if (selection > 0 && ssid_password_pos < 30)
    {
      ssid_password_pos++;
      if (ssid_password[ssid_password_pos] == '\0')
      {
        ssid_password[ssid_password_pos] = ' ';
        ssid_password[ssid_password_pos + 1] = '\0';
      }
    }
    else if (selection < 0 && ssid_password_pos > 0)
    {
      ssid_password[ssid_password_pos] = '\0';
      ssid_password_pos--;
    }

    if (get_ButtonPressedSWB())
    {
      while (ssid_password_pos > 0 && ssid_password[ssid_password_pos] == ' ')
      {
        ssid_password[ssid_password_pos] = '\0';
        ssid_password_pos--;
      }
      ssid_pswd_write(get_scaned_ssid(ssid_cursor).c_str(), ssid_password);
      reconnect_wifi();
      set_operation_mode(OPERATION_MODE_CONTROL);
      is_apply_manual_ssid = false;
      ssid_password_editing = false;
    }
    return;
  }

  int selection = read_select_step(get_r_pos_x(), ssid_select_state);

  if ((millis() - trigger_timer) <= buffer_time || millis() < trigger_enable_time || 0 == selection)
  {
    return;
  }

  trigger_timer = millis();

  if (selection < 0)
  {
    is_apply_manual_ssid = false;
    switch_to_menu_mode();
    return;
  }

  strncpy(ssid_password, get_current_password(), sizeof(ssid_password) - 1);
  ssid_password[sizeof(ssid_password) - 1] = '\0';
  if (ssid_password[0] == '\0')
  {
    strncpy(ssid_password, "12345678", sizeof(ssid_password) - 1);
  }
  ssid_password_pos = strlen(ssid_password);
  if (ssid_password_pos > 30)
  {
    ssid_password_pos = 30;
  }
  ssid_password[ssid_password_pos] = ' ';
  ssid_password[ssid_password_pos + 1] = '\0';
  ssid_password_nav_state = 0;
  ssid_password_select_state = 0;
  ssid_password_editing = true;
}

void ssid_scan_mode_handler()
{
  if (!is_apply_manual_ssid)
  {
    is_apply_manual_ssid = true;
    num_found_ssid = 0;
    current_page = 1;
    page_scale = 5;
    ssid_cursor = 0;
    show_ssid_menu = true;
    ssid_nav_state = 0;
    ssid_select_state = 0;
    ssid_password_editing = false;

    scan_wifi();
    num_found_ssid = get_num_scaned_ssid();
    trigger_timer = millis();
    trigger_enable_time = millis() + 2000;
  }

  display_scaned_ssid_oled();
  update_ssid_selection_status();
  trigger_ssid_connection();
}

static void print_pid_item(int item)
{
#ifdef __OLED__
  char page_str[26];

  switch(item)
  {
    case 0: snprintf(page_str, sizeof(page_str), "p_roll: %lu", (unsigned long)pid_list[0]); break;
    case 1: snprintf(page_str, sizeof(page_str), "i_roll: %lu", (unsigned long)pid_list[1]); break;
    case 2: snprintf(page_str, sizeof(page_str), "d_roll: %lu", (unsigned long)pid_list[2]); break;
    case 3: snprintf(page_str, sizeof(page_str), "p_pitch: %lu", (unsigned long)pid_list[3]); break;
    case 4: snprintf(page_str, sizeof(page_str), "i_pitch: %lu", (unsigned long)pid_list[4]); break;
    case 5: snprintf(page_str, sizeof(page_str), "d_pitch: %lu", (unsigned long)pid_list[5]); break;
    case 6: snprintf(page_str, sizeof(page_str), "p_yaw: %lu", (unsigned long)pid_list[6]); break;
    case 7: snprintf(page_str, sizeof(page_str), "i_yaw: %lu", (unsigned long)pid_list[7]); break;
    case 8: snprintf(page_str, sizeof(page_str), "d_yaw: %lu", (unsigned long)pid_list[8]); break;
    case 9: snprintf(page_str, sizeof(page_str), "EXIT"); break;
    default: snprintf(page_str, sizeof(page_str), ""); break;
  }

  setOledPrint(page_str);
#endif
}

void display_pid_oled()
{
#ifdef __OLED__
  char page_str[18];

  if (show_pid_menu || ((pid_cursor / pid_page_scale) + 1) != pid_current_page)
  {
    setOledClear();
    setOledCursor(0, 0);
    pid_current_page = (pid_cursor / pid_page_scale) + 1;
    snprintf(page_str, sizeof(page_str), "[Page %d]", pid_current_page);
    setOledPrint(page_str);

    for (int i = 0; i < pid_page_scale; i++)
    {
      setOledCursor(20, i * 10 + 10);
      print_pid_item((pid_cursor / pid_page_scale) * pid_page_scale + i);
    }

    show_pid_menu = false;
  }
  else
  {
    fillOledRect(0, 10, 20, 50);
    for (int i = 0; i < pid_page_scale; i++)
    {
      setOledCursor(20, i * 10 + 10);
      fillOledRect(20, i * 10 + 10, 100, 10);
      print_pid_item((pid_cursor / pid_page_scale) * pid_page_scale + i);
    }
  }

  setOledCursor(0, (pid_cursor % pid_page_scale) * 10 + 10);
  setOledPrint(is_pid_tuning ? ">>" : ">");

  fillOledRect(75, 0, 53, 10);
  setOledCursor(75, 0);
  snprintf(page_str, sizeof(page_str), "S:%d", scale);
  setOledPrint(page_str);
#endif
}

void read_pid_from_flash()
{
  for (int i = 0; i < num_pid; i++)
  {
    uint32_t value = pid_read(i);
    pid_list[i] = (PID_UNSET_VALUE == value) ? pid_default_value(i) : value;
  }
}

static void update_pid_scale_and_reset()
{
  if (!is_pid_tuning || pid_cursor > (num_pid - 1))
  {
    return;
  }

  if (get_ButtonPressedSWA())
  {
    pid_scale_idx = (pid_scale_idx + 1) % pid_scale_option_count;
    scale = pid_scale_options[pid_scale_idx];
    show_pid_menu = true;
  }

  if (get_ButtonPressedSWB())
  {
    pid_list[pid_cursor] = pid_default_value(pid_cursor);
    show_pid_menu = true;
  }
}

void update_pid_menu_selection_status()
{
  int step = read_axis_step(get_r_pos_y(), pid_nav_state);
  if (0 == step)
  {
    return;
  }

  if (!is_pid_tuning)
  {
    pid_cursor = clamp_cursor(pid_cursor + step, 0, PID_MENU_EXIT_IDX);
    return;
  }

  if (pid_cursor <= (num_pid - 1))
  {
    if (step < 0)
    {
      pid_list[pid_cursor] = min(pid_list[pid_cursor] + (uint32_t)scale, max_pid);
    }
    else
    {
      pid_list[pid_cursor] = (pid_list[pid_cursor] > (uint32_t)scale) ? pid_list[pid_cursor] - scale : 0;
    }
  }
}

void update_pid_tunning()
{
  int selection = read_select_step(get_r_pos_x(), pid_select_state);

  if ((millis() - trigger_timer) <= buffer_time || 0 == selection)
  {
    return;
  }

  trigger_timer = millis();

  if (selection < 0)
  {
    if (is_pid_tuning)
    {
      is_pid_tuning = false;
    }
    else
    {
      is_init_pid_tuning = false;
      switch_to_menu_mode();
    }
    return;
  }

  is_pid_tuning = !is_pid_tuning;
}

void trigger_pid_update()
{
  if ((PID_MENU_EXIT_IDX != pid_cursor) || !is_pid_tuning)
  {
    return;
  }

  char msg[64];
  int len = snprintf(msg, sizeof(msg), "PID");
  for (int i = 0; i < num_pid && len < (int)sizeof(msg); i++)
  {
    len += snprintf(msg + len, sizeof(msg) - len, "%05lu", (unsigned long)pid_list[i]);
  }

  send_udp_msg(msg);
  pid_write(pid_list, num_pid);
  is_init_pid_tuning = false;
  switch_to_control_mode(true);
}

void pid_tuning_mode_handler()
{
  if (!is_init_pid_tuning)
  {
    pid_cursor = 0;
    pid_current_page = 1;
    pid_page_scale = 3;
    is_pid_tuning = false;
    pid_nav_state = 0;
    pid_select_state = 0;
    show_pid_menu = true;
    read_pid_from_flash();
    is_init_pid_tuning = true;
  }

  display_pid_oled();
  update_pid_scale_and_reset();
  update_pid_menu_selection_status();
  update_pid_tunning();
  trigger_pid_update();
}

void display_testing_oled()
{
#ifdef __OLED__
  char page_str[36];

  if (show_testing_menu)
  {
    setOledClear();
    setOledCursor(15, 0);
    setOledPrint("SWA:   SWB:");
    setOledCursor(15, 10);
    setOledPrint("L:");
    setOledCursor(15, 20);
    setOledPrint("R:");
    setOledCursor(15, 30);
    setOledPrint("G:");
    setOledCursor(15, 50);
    setOledPrint("EXIT");
    show_testing_menu = false;
  }
  else
  {
    fillOledRect(40, 0, 10, 10);
    fillOledRect(75, 0, 10, 10);
    setOledCursor(40, 0);
    setOledPrint((0 == get_ButtonStateSWA()) ? "T" : "F");
    setOledCursor(75, 0);
    setOledPrint((0 == get_ButtonStateSWB()) ? "T" : "F");

    fillOledRect(35, 10, 110, 10);
    setOledCursor(35, 10);
    snprintf(page_str, sizeof(page_str), "%c %d %d", (0 == get_ButtonStateJOYS()) ? 'T' : 'F', get_l_pos_x(), get_l_pos_y());
    setOledPrint(page_str);

    fillOledRect(35, 20, 110, 10);
    setOledCursor(35, 20);
    snprintf(page_str, sizeof(page_str), "%c %d %d", (0 == get_ButtonStateJOYP()) ? 'T' : 'F', get_r_pos_x(), get_r_pos_y());
    setOledPrint(page_str);

    fillOledRect(25, 30, 110, 10);
    setOledCursor(25, 30);
    snprintf(page_str, sizeof(page_str), "%.1f %.1f", getDeviceAngleX(), getDeviceAngleY());
    setOledPrint(page_str);

    fillOledRect(15, 40, 110, 10);
    setOledCursor(25, 40);
    snprintf(page_str, sizeof(page_str), "%.1f", getDeviceAngleZ());
    setOledPrint(page_str);
  }

  fillOledRect(0, 0, 10, 60);
  setOledCursor(0, (testing_cursor % testing_page_scale) * 10);
  setOledPrint(">");
#endif
}

void update_testing_menu_selection_status()
{
  testing_cursor = clamp_cursor(testing_cursor + read_axis_step(get_r_pos_y(), testing_nav_state), 0, testing_page_scale - 1);
}

void trigger_testing_update()
{
  int selection = read_select_step(get_r_pos_x(), testing_select_state);

  if (selection < 0 || (selection > 0 && testing_page_scale - 1 == testing_cursor))
  {
    is_init_testing_tuning = false;
    switch_to_control_mode(true);
  }
}

void testing_mode_handler()
{
  if (!is_init_testing_tuning)
  {
    testing_cursor = 0;
    testing_current_page = 1;
    testing_page_scale = 6;
    is_testing_tuning = false;
    testing_nav_state = 0;
    testing_select_state = 0;
    show_testing_menu = true;
    is_init_testing_tuning = true;
  }

  display_testing_oled();
  update_testing_menu_selection_status();
  trigger_testing_update();
}

static void print_channel_select_item(int item)
{
#ifdef __OLED__
  char line[24];

  if (CHANNEL_SELECT_EXIT_IDX == item)
  {
    setOledPrint("SAVE & EXIT");
    return;
  }

  if (item > CHANNEL_SELECT_EXIT_IDX)
  {
    setOledPrint("");
    return;
  }

  snprintf(line, sizeof(line), "CH%d", item + 1);
  setOledPrint(line);
#endif
}

static void print_channel_field_item(int item)
{
#ifdef __OLED__
  char line[24];
  uint8_t channelIndex = selected_channel_index;

  if (CHANNEL_FIELD_SAVE_EXIT_IDX == item)
  {
    setOledPrint("BACK");
    return;
  }

  if (item > CHANNEL_FIELD_SAVE_EXIT_IDX)
  {
    setOledPrint("");
    return;
  }

  switch (item)
  {
    case 0:
      snprintf(line, sizeof(line), "CH%d REV:%s", channelIndex + 1, get_channel_reversed(channelIndex) ? "R" : "N");
      break;
    case 1:
      snprintf(line, sizeof(line), "CH%d RATE:%3d%%", channelIndex + 1, get_channel_rate_percent(channelIndex));
      break;
    case 2:
      snprintf(line, sizeof(line), "CH%d SUB:%4d", channelIndex + 1, get_channel_subtrim(channelIndex));
      break;
    case 3:
      snprintf(line, sizeof(line), "CH%d LOW:%3d%%", channelIndex + 1, get_channel_low_endpoint_percent(channelIndex));
      break;
    case 4:
      snprintf(line, sizeof(line), "CH%d HIGH:%3d%%", channelIndex + 1, get_channel_high_endpoint_percent(channelIndex));
      break;
    default:
      snprintf(line, sizeof(line), "");
      break;
  }

  setOledPrint(line);
#endif
}

void display_channel_oled()
{
#ifdef __OLED__
  char page_str[18];
  int maxCursor = is_channel_selected ? CHANNEL_FIELD_SAVE_EXIT_IDX : CHANNEL_SELECT_EXIT_IDX;

  if (show_channel_menu || ((channel_cursor / channel_page_scale) + 1) != channel_current_page)
  {
    setOledClear();
    setOledCursor(0, 0);
    channel_current_page = (channel_cursor / channel_page_scale) + 1;
    if (is_channel_selected)
    {
      snprintf(page_str, sizeof(page_str), "CH%d SET", selected_channel_index + 1);
    }
    else
    {
      snprintf(page_str, sizeof(page_str), "SELECT CH");
    }
    setOledPrint(page_str);

    for (int i = 0; i < channel_page_scale; i++)
    {
      int item = (channel_cursor / channel_page_scale) * channel_page_scale + i;
      setOledCursor(20, i * 10 + 10);
      if (item <= maxCursor)
      {
        if (is_channel_selected)
        {
          print_channel_field_item(item);
        }
        else
        {
          print_channel_select_item(item);
        }
      }
    }

    show_channel_menu = false;
  }
  else
  {
    fillOledRect(0, 10, 20, 50);
    for (int i = 0; i < channel_page_scale; i++)
    {
      int item = (channel_cursor / channel_page_scale) * channel_page_scale + i;
      setOledCursor(20, i * 10 + 10);
      fillOledRect(20, i * 10 + 10, 108, 10);
      if (item <= maxCursor)
      {
        if (is_channel_selected)
        {
          print_channel_field_item(item);
        }
        else
        {
          print_channel_select_item(item);
        }
      }
    }
  }

  setOledCursor(0, (channel_cursor % channel_page_scale) * 10 + 10);
  setOledPrint(is_channel_editing ? ">>" : ">");
#endif
}

void update_channel_menu_selection_status()
{
  int step = read_axis_step(get_r_pos_y(), channel_nav_state);
  if (0 == step)
  {
    return;
  }

  if (!is_channel_editing)
  {
    int maxCursor = is_channel_selected ? CHANNEL_FIELD_SAVE_EXIT_IDX : CHANNEL_SELECT_EXIT_IDX;
    channel_cursor = clamp_cursor(channel_cursor + step, 0, maxCursor);
    return;
  }

  if (!is_channel_selected || channel_cursor >= CHANNEL_FIELD_SAVE_EXIT_IDX)
  {
    return;
  }

  uint8_t channelIndex = selected_channel_index;

  switch (channel_cursor)
  {
    case 0:
      set_channel_reversed(channelIndex, !get_channel_reversed(channelIndex));
      break;
    case 1:
    {
      int rate = get_channel_rate_percent(channelIndex);
      rate += (step < 0) ? 5 : -5;
      set_channel_rate_percent(channelIndex, (uint8_t)clamp_cursor(rate, CHANNEL_MIN_ENDPOINT, CHANNEL_MAX_ENDPOINT));
      break;
    }
    case 2:
    {
      int subtrim = get_channel_subtrim(channelIndex);
      subtrim += (step < 0) ? 5 : -5;
      set_channel_subtrim(channelIndex, (int16_t)clamp_cursor(subtrim, CHANNEL_MIN_SUBTRIM, CHANNEL_MAX_SUBTRIM));
      break;
    }
    case 3:
    {
      int endpoint = get_channel_low_endpoint_percent(channelIndex);
      endpoint += (step < 0) ? 5 : -5;
      set_channel_low_endpoint_percent(channelIndex, (uint8_t)clamp_cursor(endpoint, CHANNEL_MIN_ENDPOINT, CHANNEL_MAX_ENDPOINT));
      break;
    }
    case 4:
    {
      int endpoint = get_channel_high_endpoint_percent(channelIndex);
      endpoint += (step < 0) ? 5 : -5;
      set_channel_high_endpoint_percent(channelIndex, (uint8_t)clamp_cursor(endpoint, CHANNEL_MIN_ENDPOINT, CHANNEL_MAX_ENDPOINT));
      break;
    }
    default:
      break;
  }

  show_channel_menu = true;
}

void update_channel_setup()
{
  int selection = read_select_step(get_r_pos_x(), channel_select_state);

  if ((millis() - trigger_timer) <= buffer_time || 0 == selection)
  {
    return;
  }

  trigger_timer = millis();

  if (selection < 0)
  {
    if (is_channel_editing)
    {
      is_channel_editing = false;
      show_channel_menu = true;
    }
    else if (is_channel_selected)
    {
      is_channel_selected = false;
      channel_cursor = selected_channel_index;
      channel_current_page = (channel_cursor / channel_page_scale) + 1;
      show_channel_menu = true;
    }
    else
    {
      is_init_channel_setup = false;
      switch_to_menu_mode();
    }
    return;
  }

  if (!is_channel_selected)
  {
    if (CHANNEL_SELECT_EXIT_IDX == channel_cursor)
    {
      write_channel_config_to_flash();
      is_init_channel_setup = false;
      switch_to_control_mode(true);
      return;
    }

    selected_channel_index = channel_cursor;
    is_channel_selected = true;
    channel_cursor = 0;
    channel_current_page = 1;
    show_channel_menu = true;
    return;
  }

  if (CHANNEL_FIELD_SAVE_EXIT_IDX == channel_cursor)
  {
    is_channel_selected = false;
    channel_cursor = selected_channel_index;
    channel_current_page = (channel_cursor / channel_page_scale) + 1;
    show_channel_menu = true;
    return;
  }

  is_channel_editing = !is_channel_editing;
  show_channel_menu = true;
}

void channel_setup_mode_handler()
{
  if (!is_init_channel_setup)
  {
    channel_cursor = 0;
    channel_current_page = 1;
    channel_page_scale = 4;
    is_channel_editing = false;
    is_channel_selected = false;
    selected_channel_index = 0;
    channel_nav_state = 0;
    channel_select_state = 0;
    show_channel_menu = true;
    trigger_timer = millis();
    is_init_channel_setup = true;
  }

  display_channel_oled();
  update_channel_menu_selection_status();
  update_channel_setup();
}

void channel_monitor_mode_handler()
{
  if (!is_init_channel_monitor)
  {
    monitor_cursor = 0;
    monitor_nav_state = 0;
    monitor_select_state = 0;
    is_init_channel_monitor = true;
  }

  int16_t channels[CHANNEL_COUNT];
  build_channel_outputs(get_l_pos_x(),
                        get_l_pos_y(),
                        get_r_pos_y(),
                        get_r_pos_x(),
                        channels);

  monitor_cursor = clamp_cursor(monitor_cursor + read_axis_step(get_r_pos_y(), monitor_nav_state), 0, 1);

#ifdef __OLED__
  char line[24];
  setOledClear();
  setOledCursor(0, 0);
  snprintf(line, sizeof(line), "CH MONITOR %d/2", monitor_cursor + 1);
  setOledPrint(line);

  for (int i = 0; i < 4; i++)
  {
    int ch = monitor_cursor * 4 + i;
    setOledCursor(0, i * 12 + 14);
    snprintf(line, sizeof(line), "CH%d:%4d", ch + 1, channels[ch]);
    setOledPrint(line);
  }
#endif

  int selection = read_select_step(get_r_pos_x(), monitor_select_state);
  if (selection < 0)
  {
    is_init_channel_monitor = false;
    switch_to_menu_mode();
  }
}

void display_model_oled()
{
#ifdef __OLED__
  char line[24];
  setOledClear();
  setOledCursor(0, 0);
  setOledPrint("MODEL SETUP");

  for (int i = 0; i < MODEL_COUNT; i++)
  {
    setOledCursor(20, i * 12 + 14);
    snprintf(line, sizeof(line), "M%d %-7s%s", i + 1, (get_active_model_index() == i) ? get_active_model_name() : "", (get_active_model_index() == i) ? "*" : "");
    setOledPrint(line);
  }

  setOledCursor(20, 42);
  snprintf(line, sizeof(line), "NAME:%s", get_active_model_name());
  setOledPrint(line);
  setOledCursor(20, 54);
  setOledPrint("EXIT");
  setOledCursor(0, model_cursor * 12 + 14);
  if (model_cursor == MODEL_NAME_ITEM_IDX)
  {
    setOledCursor(0, 42);
  }
  else if (model_cursor == MODEL_EXIT_ITEM_IDX)
  {
    setOledCursor(0, 54);
  }
  setOledPrint(is_model_name_editing ? ">>" : ">");
#endif
}

void model_setup_mode_handler()
{
  if (!is_init_model_setup)
  {
    model_cursor = get_active_model_index();
    model_nav_state = 0;
    model_select_state = 0;
    is_model_name_editing = false;
    show_model_menu = true;
    trigger_timer = millis();
    is_init_model_setup = true;
  }

  int modelStep = read_axis_step(get_r_pos_y(), model_nav_state);
  if (0 != modelStep)
  {
    if (is_model_name_editing)
    {
      int nextName = get_model_name_index() + ((modelStep < 0) ? 1 : -1);
      nextName = (nextName + MODEL_NAME_COUNT) % MODEL_NAME_COUNT;
      set_model_name_index((uint8_t)nextName);
    }
    else
    {
      model_cursor = clamp_cursor(model_cursor + modelStep, 0, MODEL_EXIT_ITEM_IDX);
    }
  }
  display_model_oled();

  int selection = read_select_step(get_r_pos_x(), model_select_state);
  if ((millis() - trigger_timer) <= buffer_time || 0 == selection)
  {
    return;
  }
  trigger_timer = millis();

  if (selection < 0)
  {
    if (is_model_name_editing)
    {
      is_model_name_editing = false;
      write_channel_config_to_flash();
      return;
    }
    is_init_model_setup = false;
    switch_to_menu_mode();
    return;
  }

  if (model_cursor == MODEL_EXIT_ITEM_IDX)
  {
    write_channel_config_to_flash();
    is_init_model_setup = false;
    switch_to_menu_mode();
    return;
  }

  if (model_cursor == MODEL_NAME_ITEM_IDX)
  {
    is_model_name_editing = !is_model_name_editing;
    if (!is_model_name_editing)
    {
      write_channel_config_to_flash();
    }
    return;
  }

  set_active_model_index(model_cursor);
  is_init_model_setup = false;
  switch_to_control_mode(true);
}

static void print_rate_item(int item)
{
#ifdef __OLED__
  char line[24];
  switch (item)
  {
    case 0:
      snprintf(line, sizeof(line), "LOW RATE:%3d%%", get_low_rate_percent());
      break;
    case 1:
      snprintf(line, sizeof(line), "HIGH RATE:%3d%%", get_high_rate_percent());
      break;
    case 2:
      snprintf(line, sizeof(line), "EXPO:%3d%%", get_expo_percent());
      break;
    case 3:
      snprintf(line, sizeof(line), "SAVE & EXIT");
      break;
    default:
      snprintf(line, sizeof(line), "");
      break;
  }
  setOledPrint(line);
#endif
}

void display_rate_oled()
{
#ifdef __OLED__
  setOledClear();
  setOledCursor(0, 0);
  setOledPrint("RATE/EXPO");
  for (int i = 0; i < RATE_SETUP_ITEM_COUNT; i++)
  {
    setOledCursor(20, i * 12 + 14);
    print_rate_item(i);
  }
  setOledCursor(0, rate_cursor * 12 + 14);
  setOledPrint(is_rate_editing ? ">>" : ">");
#endif
}

void rate_expo_setup_mode_handler()
{
  if (!is_init_rate_setup)
  {
    rate_cursor = 0;
    rate_nav_state = 0;
    rate_select_state = 0;
    is_rate_editing = false;
    show_rate_menu = true;
    trigger_timer = millis();
    is_init_rate_setup = true;
  }

  int step = read_axis_step(get_r_pos_y(), rate_nav_state);
  if (0 != step)
  {
    if (!is_rate_editing)
    {
      rate_cursor = clamp_cursor(rate_cursor + step, 0, RATE_SETUP_SAVE_EXIT_IDX);
    }
    else
    {
      int delta = (step < 0) ? 5 : -5;
      if (rate_cursor == 0)
      {
        set_low_rate_percent((uint8_t)clamp_cursor(get_low_rate_percent() + delta, 30, 100));
      }
      else if (rate_cursor == 1)
      {
        set_high_rate_percent((uint8_t)clamp_cursor(get_high_rate_percent() + delta, 30, 125));
      }
      else if (rate_cursor == 2)
      {
        set_expo_percent((uint8_t)clamp_cursor(get_expo_percent() + delta, 0, 80));
      }
    }
  }

  display_rate_oled();

  int selection = read_select_step(get_r_pos_x(), rate_select_state);
  if ((millis() - trigger_timer) <= buffer_time || 0 == selection)
  {
    return;
  }
  trigger_timer = millis();

  if (selection < 0)
  {
    if (is_rate_editing)
    {
      is_rate_editing = false;
    }
    else
    {
      is_init_rate_setup = false;
      switch_to_menu_mode();
    }
    return;
  }

  if (rate_cursor == RATE_SETUP_SAVE_EXIT_IDX)
  {
    write_channel_config_to_flash();
    is_init_rate_setup = false;
    switch_to_control_mode(true);
    return;
  }

  is_rate_editing = !is_rate_editing;
}

void display_centering_oled()
{
#ifdef __OLED__
  char msg[64];

  if (is_centering)
  {
    if (!is_show_message)
    {
      setOledClear();
      setOledCursor(0, 0);
      if (0 == calibration_phase)
      {
        setOledPrint("Center sticks");
        setOledCursor(0, 10);
        setOledPrint("Press SWB");
      }
      else
      {
        setOledPrint("Move all ends");
        setOledCursor(0, 10);
        setOledPrint("Press SWB save");
      }
      is_show_message = true;
    }

    fillOledRect(25, 40, 110, 10);
    setOledCursor(25, 40);
    snprintf(msg, sizeof(msg), "L: %d %d", get_l_pos_x(), get_l_pos_y());
    setOledPrint(msg);

    fillOledRect(25, 50, 110, 10);
    setOledCursor(25, 50);
    snprintf(msg, sizeof(msg), "R: %d %d", get_r_pos_x(), get_r_pos_y());
    setOledPrint(msg);
  }
#endif
}

void detect_calibrating()
{
  if (is_centering && (0 == get_ButtonStateSWB()))
  {
    if (0 == calibration_phase)
    {
      cal_center_work[0] = get_l_pos_x_v();
      cal_center_work[1] = get_l_pos_y_v();
      cal_center_work[2] = get_r_pos_x_v();
      cal_center_work[3] = get_r_pos_y_v();
      for (int i = 0; i < 4; i++)
      {
        cal_min_work[i] = cal_center_work[i];
        cal_max_work[i] = cal_center_work[i];
      }
      calibration_phase = 1;
      is_show_message = false;
      trigger_timer = millis();
      return;
    }

    if ((millis() - trigger_timer) > 500)
    {
      cal_full_write(cal_center_work, cal_min_work, cal_max_work, 4);
      is_centering = true;
      is_init_calibaration_tuning = false;
      switch_to_control_mode(true);
    }
  }
}

void calibration_mode_handler()
{
  if (!is_init_calibaration_tuning)
  {
    is_show_message = false;
    is_centering = true;
    calibration_phase = 0;
    is_init_calibaration_tuning = true;
  }

  if (1 == calibration_phase)
  {
    int16_t raw[4] = {get_l_pos_x_v(), get_l_pos_y_v(), get_r_pos_x_v(), get_r_pos_y_v()};
    for (int i = 0; i < 4; i++)
    {
      if (raw[i] < cal_min_work[i])
      {
        cal_min_work[i] = raw[i];
      }
      if (raw[i] > cal_max_work[i])
      {
        cal_max_work[i] = raw[i];
      }
    }
  }

  display_centering_oled();
  detect_calibrating();
}

