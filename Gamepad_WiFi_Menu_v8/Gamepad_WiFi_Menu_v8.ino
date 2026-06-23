/*
 * V7 ESP32 WiFi gamepad with menu mode for SSID SCAN, PID TUNING, TESTING, and CALIBRATION.
 */

#include "Options.h"
#include "oled.h"
#include "wifi.h"
#include "utility.h"
#include "Sensors.h"
#include "joystick.h"
#include "menu.h"
#include "pid.h"
#include "channel.h"
#include <ArduinoOTA.h>

bool otaEnabled = false;
static unsigned long lastLoopRunMs = 0;
static bool controlArmed = false;
static bool throttleSafetyCleared = false;
static unsigned long armToggleMs = 0;
static unsigned long lastControlStatusMs = 0;

static int16_t get_throttle_stick_value()
{
#ifdef __LEFT_HAND_THROTTLE__
  return get_l_pos_y();
#else
  return get_r_pos_y();
#endif
}

static uint8_t get_throttle_channel_index()
{
#ifdef __LEFT_HAND_THROTTLE__
  return 1;
#else
  return 2;
#endif
}

static void build_control_outputs(int16_t channels[CHANNEL_COUNT])
{
#ifdef __LEFT_HAND_THROTTLE__
  build_channel_outputs(get_l_pos_x(),
                        get_l_pos_y(),
                        get_r_pos_y(),
                        get_r_pos_x(),
                        channels);
#else
  build_channel_outputs(get_l_pos_x(),
                        get_l_pos_y(),
                        get_r_pos_y(),
                        get_r_pos_x(),
                        channels);
#endif
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  set_operation_mode(OPERATION_MODE_CONTROL);

  init_eeprom();
  init_i2c();
  prefix_init();

  init_joystick();
  init_channel_output();
  delay(120);
  get_joystick_status();

#ifdef __OLED__
  init_oled();
#endif

#ifdef __SENSORS__
  init_mpu();
#endif

  init_wifi();

#ifdef __OTA__
  otaEnabled = ((0 == get_ButtonStateSWA()) && (0 == get_ButtonStateSWB()));
  if (otaEnabled)
  {
    ArduinoOTA.setHostname("ESP32-Gamepad");
    ArduinoOTA.setPassword("12345678");
    ArduinoOTA.begin();
#ifdef __OLED__
    showCurrentStage("OTA enabled");
#endif
  }
#endif

  init_udp();
  read_cal_from_flash();
}

void control_mode_handler()
{
  char msg[40];
  int16_t channels[CHANNEL_COUNT];
  uint8_t throttleChannel = get_throttle_channel_index();
  bool throttleLow = get_throttle_stick_value() < 1100;
  bool throttleCut = (0 == get_ButtonStateSWA());

  if (throttleLow)
  {
    throttleSafetyCleared = true;
  }

  set_low_rate_mode(0 == get_ButtonStateSWB());

  if (!controlArmed &&
      (0 == get_ButtonStateJOYS()) &&
      (0 == get_ButtonStateJOYP()) &&
      throttleLow &&
      throttleSafetyCleared &&
      ((millis() - armToggleMs) > 800))
  {
    controlArmed = true;
    armToggleMs = millis();
  }

  build_control_outputs(channels);

  if (!controlArmed || throttleCut)
  {
    channels[throttleChannel] = CHANNEL_MIN;
  }

  build_srv_message(channels, msg, sizeof(msg));
  send_udp_msg(msg);

#ifdef __OLED__
  if ((millis() - lastControlStatusMs) > 250)
  {
    char line[22];
    const char* connectedSsid = get_connected_ssid();
    setOledClear();
    setOledCursor(0, 0);
    snprintf(line, sizeof(line), "%s", is_wifi_connected() ? connectedSsid : "WiFi:OFF");
    setOledPrint(line);
    setOledCursor(0, 12);
    if (!throttleSafetyCleared)
    {
      snprintf(line, sizeof(line), "THROTTLE HIGH");
    }
    else
    {
      snprintf(line, sizeof(line), "%s %s %s", get_active_model_name(), controlArmed ? "ARM" : "LOCK", get_low_rate_mode() ? "LOW" : "HIGH");
    }
    setOledPrint(line);
    setOledCursor(0, 24);
    snprintf(line, sizeof(line), "T:%d C:%s", channels[throttleChannel], throttleCut ? "ON" : "OFF");
    setOledPrint(line);
    setOledCursor(0, 36);
    snprintf(line, sizeof(line), "E:%d%% LR:%d%%", get_expo_percent(), get_low_rate_percent());
    setOledPrint(line);
    lastControlStatusMs = millis();
  }
#endif

  if ((1250 > get_l_pos_x()) &&
      (1250 > get_l_pos_y()) &&
      (0 == get_ButtonStateSWB()) &&
      (0 == get_ButtonStateSWA()))
  {
    set_operation_mode(OPERATION_MODE_MENU);

#ifdef __OLED__
    setOledClear();
    setOledCursor(0,0);
    setOledPrint("Entering Menu Mode...");
#endif
  }

#ifdef __LOG__
  Serial.print("l_pos_x: ");
  Serial.print(get_l_pos_x());
  Serial.print(", l_pos_y: ");
  Serial.print(get_l_pos_y());
  Serial.print(", r_pos_x: ");
  Serial.print(get_r_pos_x());
  Serial.print(", r_pos_y: ");
  Serial.print(get_r_pos_y());
  Serial.print(", joy_btn_s: ");
  Serial.print(get_ButtonStateJOYS());
  Serial.print(", joy_btn_p: ");
  Serial.print(get_ButtonStateJOYP());
  Serial.print(", sw_a: ");
  Serial.print(get_ButtonStateSWA());
  Serial.print(", sw_b: ");
  Serial.println(get_ButtonStateSWB());
  Serial.print("l_pos_x_v: ");
  Serial.print(get_l_pos_x_v());
  Serial.print(", l_pos_y_v: ");
  Serial.print(get_l_pos_y_v());
  Serial.print(", r_pos_x_v: ");
  Serial.print(get_r_pos_x_v());
  Serial.print(", r_pos_y_v: ");
  Serial.println(get_r_pos_y_v());
#endif
}

void loop() {
  unsigned long now = millis();
  if (now - lastLoopRunMs < 10)
  {
    return;
  }
  lastLoopRunMs = now;

  get_joystick_status();

#ifdef __OTA__
  if (otaEnabled)
  {
    ArduinoOTA.handle();
  }
#endif

  service_wifi();

#ifdef __SENSORS__
  genMPU6050Sample();
#endif

  if (OPERATION_MODE_CONTROL == get_operation_mode())
  {
    control_mode_handler();
  }
  else if (OPERATION_MODE_SSID_SCAN == get_operation_mode())
  {
    ssid_scan_mode_handler();
  }
  else if (OPERATION_MODE_PID_TUNING == get_operation_mode())
  {
    pid_tuning_mode_handler();
  }
  else if (OPERATION_MODE_TESTING == get_operation_mode())
  {
    testing_mode_handler();
  }
  else if (OPERATION_MODE_MENU == get_operation_mode())
  {
    menu_mode_handler();
  }
  else if (OPERATION_MODE_CALIBRATION == get_operation_mode())
  {
    calibration_mode_handler();
  }
  else if (OPERATION_MODE_CHANNEL_SETUP == get_operation_mode())
  {
    channel_setup_mode_handler();
  }
  else if (OPERATION_MODE_MODEL_SETUP == get_operation_mode())
  {
    model_setup_mode_handler();
  }
  else if (OPERATION_MODE_RATE_EXPO_SETUP == get_operation_mode())
  {
    rate_expo_setup_mode_handler();
  }
  else if (OPERATION_MODE_CHANNEL_MONITOR == get_operation_mode())
  {
    channel_monitor_mode_handler();
  }

#ifdef __OLED__
  oledRefresh();
#endif

}

