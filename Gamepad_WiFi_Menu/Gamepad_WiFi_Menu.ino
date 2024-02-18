/*
 * Provided by 飛行造物.
 * This code implements WiFi gamepad with menu mode for SSID SCAN, PID TUNING, TESTING, and CALIBRATION
 */

#include "Options.h"
#include "oled.h"
#include "wifi.h"
#include "utility.h"
#include "Sensors.h"
#include "joystick.h"
#include "menu.h"
#include "pid.h"

void setup() {
  Serial.begin(115200);
  Serial.println();

  /* initialize operation mode */
  set_operation_mode(OPERATION_MODE_CONTROL);

#ifdef __OLED__
  /* initialize OLED configuration */
  init_oled();
#endif

  /* initialize EEPROM */
  init_eeprom();

  /* initialize prefix of WiFi ssid to be connected */
  prefix_init();

#ifdef __SENSORS__
  /* initialize MPU6050 */
  init_mpu();
#endif

  /* initialize WiFi configuration */
  init_wifi();

  /* initialize Gamepad ADC configuration */
  init_joystick();

  /* initialize utility */
  init_udp();

  read_cal_from_flash();
}

void control_mode_handler()
{
  /* compose V7RC command based on V7RC protocol */
  String msg = "SRV" + String(get_l_pos_x()) + String(get_l_pos_y()) + String(get_r_pos_y()) + String(get_r_pos_x()) + "1500" + "1500" + "1500" + "1500";
  send_udp_msg(msg);

  /* 
   *  Enter MENU operation mode with the following gamepad status
   *  (1) pull left joystick to lower left position; and
   *  (2) press SWA button; and
   *  (3) press SWB button
   */
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
  /*  The execution flow for loop is
   *  (1) to get joystick status
   *  (2) invoke the main functions based on operation mode
   */
  get_joystick_status();

#ifdef __SENSORS__
  /* Read MPU6050 status to get current roll, pitch, and yaw */
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
  else
  {
    ;
  }

  delay(10);
}
