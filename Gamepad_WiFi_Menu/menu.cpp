#include "Arduino.h"
#include "Options.h"
#include "menu.h"
#include "wifi.h"
#include "joystick.h"
#include "oled.h"
#include "utility.h"
#include "pid.h"
#include "Sensors.h"

/* Timer for trigger menu selection */
float trigger_timer = 0;
float buffer_time = 8000;

/* Parameters for menu display */
int menu_cursor = 0;
int menu_current_page = 1;
int menu_page_scale = 4;
bool showmenu = false;
bool is_init_menu_tuning = false;

/* Parameters for ssid scan display */
bool is_apply_manual_ssid = false;
bool show_ssid_menu = false;
int num_found_ssid;
int current_page;
int page_scale;
int ssid_cursor;

/* Parameter for PID tuning display */
bool show_pid_menu = false;
bool is_init_pid_tuning = false;
bool is_pid_tuning;
int pid_cursor;
int scale = 50;
int max_pid = 100000;
/* 99999 means no setting */
uint8_t num_pid = 9;
uint16_t pid_list[9]={99999,99999,99999,99999,99999,99999,99999,99999,99999}; /* [p_roll, i_roll, d_roll, p_pitch, i_pitch, d_pitch, p_yaw, i_yaw, d_yaw] */
int pid_current_page;
int pid_page_scale;

/* Parameter for TESTING display */
int testing_cursor = 0;
int testing_current_page = 1;
int testing_page_scale = 6;
bool is_testing_tuning = false;
bool is_init_testing_tuning = false;
bool show_testing_menu = false;

/* Prarmeter for CALIBRATION display */
bool is_show_message = false;
bool is_centering = false;
bool is_init_calibaration_tuning = false;


/****************************************
 *           MENU MODE            *
 ****************************************/
void display_menu_oled()
{
#ifdef __OLED__
  String page_str = "";
  if (true == showmenu)
  {
    setOledClear();

    /* display page information */
    setOledCursor(0, 0);
    menu_current_page = ((int) (menu_cursor / menu_page_scale)) + 1; // number of page = ssid_cursor / page_scale
    page_str = "[Page " + String(menu_current_page) +"]";
    setOledPrint(page_str);
  
    setOledCursor(20, (OPERATION_MODE_SSID_SCAN-2)*10+10);
    page_str = "SSID SCAN MODE";
    setOledPrint(page_str);
  
    setOledCursor(20, (OPERATION_MODE_PID_TUNING-2)*10+10);
    page_str = "PID TUNING MODE";
    setOledPrint(page_str);
  
    setOledCursor(20, (OPERATION_MODE_TESTING-2)*10+10);
    page_str = "TESTING MODE";
    setOledPrint(page_str);

    setOledCursor(20, (OPERATION_MODE_CALIBRATION-2)*10+10);
    page_str = "CALIBRATION MODE";
    setOledPrint(page_str);
  
    setOledCursor(20, (OPERATION_MODE_MAX-2)*10+10);
    page_str = "EXIT";
    setOledPrint(page_str);

    showmenu = false;
  }
  else
  {
    fillOledRect(0, 10, 20, 50);

    /* display ssid cursor */
    setOledCursor(0,((int) (menu_cursor%menu_page_scale))*10+10);
    setOledPrint(">");
  }
#endif
}

void update_menu_selection_status()
{
  if (1750 < get_r_pos_y())
  {
    if (0 <= menu_cursor)
    {
      if (0 != menu_cursor)
      {
        menu_cursor -= 1;
      }
    }
  }

  if (1250 > get_r_pos_y())
  {
    if (menu_cursor < (menu_page_scale-1))
    {
      menu_cursor += 1;
    }
  }
}

void switch_to_operation_mode(String str)
{
#ifdef __OLED__
   setOledClear();
   setOledCursor(0,0);
   setOledPrint(str);
#endif
}

void switch_to_control_mode()
{
#ifdef __OLED__
   setOledClear();
   setOledCursor(0,0);
   setOledPrint("Entering Control Mode...");
   reconnect_wifi();
#endif
}

void detect_menu_selection()
{
  if (1750 < get_r_pos_x())
  {
    switch(menu_cursor)
    {
      case (OPERATION_MODE_SSID_SCAN-2):
        set_operation_mode(OPERATION_MODE_SSID_SCAN);
        is_init_menu_tuning = false;

        switch_to_operation_mode(String("Entering SSID SCAN Mode..."));
        break;
      case (OPERATION_MODE_PID_TUNING-2):
        set_operation_mode(OPERATION_MODE_PID_TUNING);
        is_init_menu_tuning = false;

        switch_to_operation_mode(String("Entering PID Tuning Mode..."));
        break;
      case (OPERATION_MODE_TESTING-2):
        set_operation_mode(OPERATION_MODE_TESTING);
        is_init_menu_tuning = false;

        switch_to_operation_mode(String("Entering Testing Mode..."));
        break;
      case (OPERATION_MODE_CALIBRATION-2):
        set_operation_mode(OPERATION_MODE_CALIBRATION);
        is_init_menu_tuning = false;

        switch_to_operation_mode(String("Entering Calibration Mode..."));
        break;
      default:
        set_operation_mode(OPERATION_MODE_CONTROL);
        is_init_menu_tuning = false;

        switch_to_operation_mode(String("Entering Control Mode..."));
        reconnect_wifi();
        break;
    }

    trigger_timer =  millis();
  }
}

void menu_mode_handler()
{
  Serial.println("Menu Mode Handler");

  if (false == is_init_menu_tuning)
  {
    menu_cursor = 0;
    menu_current_page = 1;
    menu_page_scale = 5;

    showmenu = true;
    is_init_menu_tuning = true;
  }

  display_menu_oled();
  update_menu_selection_status();
  detect_menu_selection();
}

/****************************************
 *           SSIC SCAN MODE             *
 ****************************************/
/*
 * This function is used to display ssid menu
 * - current_page: indicates current page of ssid menu
 * - page_scale: indicates the maximum number of ssid items of each page
 * - ssid_cursor: indicates the position of ssid cursor
 */
void display_scaned_ssid_oled()
{
#ifdef __OLED__
    String page_str = "";
    
    if ((true == show_ssid_menu) ||
        (((int) (ssid_cursor / page_scale)) + 1) != current_page)
    {
      setOledClear();

      /* display page information */
      setOledCursor(0, 0);

      current_page = ((int) (ssid_cursor / page_scale)) + 1; // number of page = ssid_cursor / page_scale
      page_str = "[Page " + String(current_page) +"]";
      setOledPrint(page_str);
  
      /* display ssid menu as per number of page */
      for (int i = 0; i < page_scale; i++)
      {
        setOledCursor(10, i*10+10);
        setOledPrint(get_scaned_ssid(((ssid_cursor/page_scale)*page_scale + i)).substring(0,9));
      }

      show_ssid_menu = false;
    }
    else
    {
      fillOledRect(0, 10, 10, 60);
    }

    /* display ssid cursor */
    setOledCursor(0,((int) (ssid_cursor%page_scale))*10+10);
    setOledPrint(">");

    delay(5);
#endif
}

void update_ssid_selection_status()
{
  if (1750 < get_r_pos_y())
  {
    if (ssid_cursor > 0)
    {
      ssid_cursor -= 1;
    }
  }

  if (1250 > get_r_pos_y())
  {
    if (ssid_cursor < num_found_ssid)
    {
      ssid_cursor += 1;
    }
  }
}

void trigger_ssid_connection()
{
  if ((buffer_time < (millis() - trigger_timer)) &&
      (1750 < get_r_pos_x()))
  {
#ifdef __OLED__
    setOledClear();
    setOledCursor(0,0);
    setOledPrint("Entering Control Mode...");
#endif

    /* write ssid selected by user to EEPROM */
    ssid_pswd_write(get_scaned_ssid(ssid_cursor), get_scaned_ssid(ssid_cursor));
    /* reconnect to WiFi with respect to the selected ssid */
    reconnect_wifi();
    /* enter CONTROL operation mode */
    set_operation_mode(OPERATION_MODE_CONTROL);
    is_apply_manual_ssid = false;

    delay(500);
  }
}

void ssid_scan_mode_handler()
{
  /* initialize parameters of ssid menu when entering SSID SCAN operation mode */
  if (false == is_apply_manual_ssid)
  {
    is_apply_manual_ssid = true;
    num_found_ssid = 0;
    current_page = 1;
    page_scale = 5;
    ssid_cursor = 0;
    show_ssid_menu = true;

    /* scan WiFi to get SSID list */
    scan_wifi();
    num_found_ssid = get_num_scaned_ssid();

#ifdef __LOG__
    Serial.print("num_found_ssid: ");
    Serial.println(num_found_ssid);
#endif
  }

  /* display ssid menu based on scaned ssid list */
  display_scaned_ssid_oled();
  /* update ssid cursor based on joystick control */
  update_ssid_selection_status();
  /* trigger WiFi reconnection when a scaned ssid has been selected by user */
  trigger_ssid_connection();
}

/****************************************
 *           PID TUNING MODE            *
 ****************************************/
void display_pid_oled()
{
#ifdef __OLED__
    String page_str = "";

    if ((true == show_pid_menu) ||
        ((((int) (pid_cursor / pid_page_scale)) + 1) != pid_current_page))
    {
      setOledClear();
  
      /* display page information */
      setOledCursor(0, 0);
      pid_current_page = ((int) (pid_cursor / pid_page_scale)) + 1; // number of page = ssid_cursor / page_scale
      page_str = "[Page " + String(pid_current_page) +"]";
      setOledPrint(page_str);
  
      /* display ssid menu as per number of page */
      for (int i = 0; i < pid_page_scale; i++)
      {
        setOledCursor(20, i*10+10);
  
        switch((pid_cursor/pid_page_scale)*pid_page_scale + i)
        {
          case 0:
            page_str = "p_roll: " + String(pid_list[0]);
            setOledPrint(page_str);
            break;
          case 1:
            page_str = "i_roll: " + String(pid_list[1]);
            setOledPrint(page_str);
            break;
          case 2:
            page_str = "d_roll: " + String(pid_list[2]);
            setOledPrint(page_str);
            break;
          case 3:
            page_str = "p_pitch: " + String(pid_list[3]);
            setOledPrint(page_str);
            break;
          case 4:
            page_str = "i_pitch: " + String(pid_list[4]);
            setOledPrint(page_str);
            break;
          case 5:
            page_str = "d_pitch: " + String(pid_list[5]);
            setOledPrint(page_str);
            break;
          case 6:
            page_str = "p_yaw: " + String(pid_list[6]);
            setOledPrint(page_str);
            break;
          case 7:
            page_str = "i_yaw: " + String(pid_list[7]);
            setOledPrint(page_str);
            break;
          case 8:
            page_str = "d_yaw: " + String(pid_list[8]);
            setOledPrint(page_str);
            break;
          case 9:
            page_str = "EXIT";
            setOledPrint(page_str);
          default:
            break;
        }
      }

      show_pid_menu = false;
    }
    else
    {
      fillOledRect(0, 10, 20, 50);

      /* display ssid menu as per number of page */
      for (int i = 0; i < pid_page_scale; i++)
      {
        setOledCursor(20, i*10+10);
        fillOledRect(20, i*10+10, 100, 10);
  
        switch((pid_cursor/pid_page_scale)*pid_page_scale + i)
        {
          case 0:
            page_str = "p_roll: " + String(pid_list[0]);
            setOledPrint(page_str);
            break;
          case 1:
            page_str = "i_roll: " + String(pid_list[1]);
            setOledPrint(page_str);
            break;
          case 2:
            page_str = "d_roll: " + String(pid_list[2]);
            setOledPrint(page_str);
            break;
          case 3:
            page_str = "p_pitch: " + String(pid_list[3]);
            setOledPrint(page_str);
            break;
          case 4:
            page_str = "i_pitch: " + String(pid_list[4]);
            setOledPrint(page_str);
            break;
          case 5:
            page_str = "d_pitch: " + String(pid_list[5]);
            setOledPrint(page_str);
            break;
          case 6:
            page_str = "p_yaw: " + String(pid_list[6]);
            setOledPrint(page_str);
            break;
          case 7:
            page_str = "i_yaw: " + String(pid_list[7]);
            setOledPrint(page_str);
            break;
          case 8:
            page_str = "d_yaw: " + String(pid_list[8]);
            setOledPrint(page_str);
            break;
          case 9:
            page_str = "EXIT";
            setOledPrint(page_str);
          default:
            break;
        }
      }
    }

    /* display ssid cursor */
    setOledCursor(0,((int) (pid_cursor%pid_page_scale))*10+10);

    if (false == is_pid_tuning)
    {
      setOledPrint(">");
    }
    else /* true == is_pid_tuning */
    {
      setOledPrint(">>");
    }
#endif
}

void read_pid_from_flash()
{
  for (int i = 0; i < num_pid; i++)
  {
    pid_list[i] = pid_read(i);

#ifdef __LOG__
    Serial.print("pid_list[" + String(i) + "]: ");
    Serial.println(pid_list[i]);
#endif
  }
}

void update_pid_menu_selection_status()
{
  if (1750 < get_r_pos_y())
  {
    if (0 <= pid_cursor)
    {
      if (false == is_pid_tuning)
      {
        if (0 != pid_cursor)
        {
          pid_cursor -= 1;
        }
      }
      else /* true == is_pid_tuning */
      {
        if (pid_cursor <= (num_pid-1))
        {
          if ((max_pid - scale) > pid_list[pid_cursor])
          {
            pid_list[pid_cursor] = (pid_list[pid_cursor]+scale)%max_pid;
          }
          else
          {
            pid_list[pid_cursor] = max_pid;
          }
        }
      }
    }
  }

  if (1250 > get_r_pos_y())
  {
    if (pid_cursor <= 9)
    {
      if (false == is_pid_tuning)
      {
        if (9 != pid_cursor)
        {
          pid_cursor += 1;
        }
      }
      else /* true == is_pid_tuning */
      {
        if (pid_cursor <= (num_pid-1))
        {
          if (scale < pid_list[pid_cursor])
          {
            pid_list[pid_cursor] = (pid_list[pid_cursor]-scale)%max_pid;
          }
          else
          {
            pid_list[pid_cursor] = 0;
          }
        }
      }
    }
  }
}

void update_pid_tunning()
{
  if ((buffer_time < (millis() - trigger_timer)) &&
      (1750 < get_r_pos_x()))
  {
    if (false == is_pid_tuning)
    {
      is_pid_tuning = true;
    }
    else
    {
      is_pid_tuning = false;
    }
  }
}

void trigger_pid_update()
{
  int digits = 10000;
  String msg = "";
  /* 
   *  Enter CONTROL operation mode with the following gamepad status
   *  (1) pull left joystick to lower right position; and
   *  (2) press SWA button; and
   *  (3) press SWB button
   */
  if ((num_pid == pid_cursor) &&
      (true == is_pid_tuning))
  {
    set_operation_mode(OPERATION_MODE_CONTROL);
    is_init_pid_tuning = false;

    /* compose V7RC command based on V7RC protocol */
    msg = "PID";
    
    for (int i=0; i<num_pid; i++)
    {
      digits = 10000;

      while (1 != digits)
      {
        if (0 == (pid_list[i]/digits))
        {
          msg += "0";
        }
        digits = digits/10;
      }

      msg += String(pid_list[i]);
    }

    Serial.println(msg);

    /* transmit PID tunning parameter- command via UDP */
    send_udp_msg(msg);

    pid_write(pid_list, num_pid);

#ifdef __OLED__
    setOledClear();
    setOledCursor(0,0);
    setOledPrint("Entering Control Mode...");
#endif
    reconnect_wifi();
  }
}


void pid_tuning_mode_handler()
{
  if (false == is_init_pid_tuning)
  {
    pid_cursor = 0;
    pid_current_page = 1;
    pid_page_scale = 3;
    is_pid_tuning = false;

    is_init_pid_tuning = true;

    show_pid_menu = true;
    read_pid_from_flash();
  }

  display_pid_oled();
  update_pid_menu_selection_status();
  update_pid_tunning();
  trigger_pid_update();
}

/*******************************************
 *           TESTING MODE                  *
 *******************************************/

void display_testing_oled()
{
#ifdef __OLED__
  String page_str = "";

  if (true == show_testing_menu)
  {
    setOledClear();

    /* display page information */
    setOledCursor(15, 0);
    
    page_str = "SWA:";
    page_str += " ";
    page_str += " SWB:";
    page_str += " ";
    setOledPrint(page_str);
  
    setOledCursor(15, 10);
    page_str = "L:";
    setOledPrint(page_str);
  
    setOledCursor(15, 20);
    page_str = "R:";
    setOledPrint(page_str);
  
    setOledCursor(15, 30);
    page_str = "G:";
    setOledPrint(page_str);
  
    setOledCursor(15, 50);
    page_str = "EXIT";
    setOledPrint(page_str);

    show_testing_menu = false;
  }
  else
  {
    fillOledRect(40, 0, 10, 10);
    fillOledRect(75, 0, 10, 10);
    
    /* display page information */
    setOledCursor(40, 0);
    page_str = (0 == get_ButtonStateSWA()) ? "T" : "F";
    setOledPrint(page_str);

    setOledCursor(75, 0);
    page_str = (0 == get_ButtonStateSWB()) ? "T" : "F";
    setOledPrint(page_str);

    fillOledRect(35, 10, 110, 10);
    setOledCursor(35, 10);
    page_str = (0 == get_ButtonStateJOYS()) ? "T" : "F";
    page_str += " " + String(get_l_pos_x());
    page_str += " " + String(get_l_pos_y());
    setOledPrint(page_str);

    fillOledRect(35, 20, 110, 10);
    setOledCursor(35, 20);
    page_str = (0 == get_ButtonStateJOYP()) ? "T" : "F";
    page_str += " " + String(get_r_pos_x());
    page_str += " " + String(get_r_pos_y());
    setOledPrint(page_str);

    fillOledRect(25, 30, 110, 10);
    setOledCursor(25, 30);
    page_str = " " + String(getDeviceAngleX());
    page_str += " " + String(getDeviceAngleY());
    setOledPrint(page_str);

    fillOledRect(15, 40, 110, 10);
    setOledCursor(25, 40);
    page_str = " " + String(getDeviceAngleZ());
    setOledPrint(page_str);
  }

  fillOledRect(0, 0, 10, 60);
  /* display ssid cursor */
  setOledCursor(0,((int) (testing_cursor%testing_page_scale))*10);
  setOledPrint(">");
#endif
}

void update_testing_menu_selection_status()
{
  if (1750 < get_r_pos_y())
  {
    if (0 <= testing_cursor)
    {
      if (0 != testing_cursor)
      {
        testing_cursor -= 1;
      }
    }
  }

  if (1250 > get_r_pos_y())
  {
    if (testing_cursor < (testing_page_scale-1))
    {
      testing_cursor += 1;
    }
  }
}

void trigger_testing_update()
{
  if (1750 < get_r_pos_x())
  {
    if (testing_page_scale-1 == testing_cursor)
    {
      set_operation_mode(OPERATION_MODE_CONTROL);
      is_init_testing_tuning = false;

#ifdef __OLED__
      setOledClear();
      setOledCursor(0,0);
      setOledPrint("Entering Control Mode...");
#endif
      reconnect_wifi();
    }
  }
}

void testing_mode_handler()
{
  if (false == is_init_testing_tuning)
  {
    testing_cursor = 0;
    testing_current_page = 1;
    testing_page_scale = 6;
    is_testing_tuning = false;
    show_testing_menu = true;

    is_init_testing_tuning = true;
  }

  display_testing_oled();
  update_testing_menu_selection_status();
  trigger_testing_update();
}

void display_centering_oled()
{
  String centering_msg = "";

  if (true == is_centering)
  {
    if (false == is_show_message)
    {
      setOledClear();

      /* display centeringe information */
      setOledCursor(0, 0);
      centering_msg = "Please adjust both joysticks to center position and press Switch B on the upper left side...";
      setOledPrint(centering_msg);

      is_show_message = true;
    }

    fillOledRect(25, 40, 110, 10);
    setOledCursor(25, 40);
    centering_msg = "L:";
    centering_msg += " " + String(get_l_pos_x());
    centering_msg += " " + String(get_l_pos_y());
    setOledPrint(centering_msg);

    fillOledRect(25, 50, 110, 10);
    setOledCursor(25, 50);
    centering_msg = "R:";
    centering_msg += " " + String(get_r_pos_x());
    centering_msg += " " + String(get_r_pos_y());
    setOledPrint(centering_msg);
  }
}

void detect_calibrating()
{
  if ((true == is_centering) &&
      (0 == get_ButtonStateSWB()))
  {
    uint8_t num_cal = 4;
    int16_t cal_list[4]={0,0,0,0}; /* [cal_l_x_offset, cal_l_y_offset, cal_r_x_offset, cal_r_y_offset] */

    set_cal_l_x_offset(((int16_t) get_l_pos_x()) - 1500);
    set_cal_l_y_offset(((int16_t) get_l_pos_y()) - 1500);
    set_cal_r_x_offset(((int16_t) get_r_pos_x()) - 1500);
    set_cal_r_y_offset(((int16_t) get_r_pos_y()) - 1500);

    cal_list[0] = get_cal_l_x_offset();
    cal_list[1] = get_cal_l_y_offset();
    cal_list[2] = get_cal_r_x_offset();
    cal_list[3] = get_cal_r_y_offset();

    cal_write(cal_list, num_cal);

    is_centering = true;

    set_operation_mode(OPERATION_MODE_CONTROL);
    is_init_calibaration_tuning = false;

#ifdef __OLED__
    setOledClear();
    setOledCursor(0,0);
    setOledPrint("Entering Control Mode...");
#endif
    reconnect_wifi();
  }
}

void calibration_mode_handler()
{
  if (false == is_init_calibaration_tuning)
  {
    is_show_message = false;
    is_centering = true;

    set_cal_l_x_offset(0);
    set_cal_l_y_offset(0);
    set_cal_r_x_offset(0);
    set_cal_r_y_offset(0);

    is_init_calibaration_tuning = true;
  }

  display_centering_oled();
  detect_calibrating();
}
