/*
 * Provided by 飛行造物.
 * This code implements WiFi gamepad which connects to WiFi connection with the following steps
 *  (1) using stored SSID;
 *  (2) using WiFiScan to find available SSID with the prefix of Wright or Hover;
 *  (3) using manual search with list derived from WiFi scan;
 */

#include <WiFiUdp.h>
#include <Adafruit_ADS1X15.h>
#include "Sensors.h"
#include "Options.h"
#include "oled.h"
#include "wifi.h"

/* Gamepad operation mode */
typedef enum
{
  OPERATION_MODE_CONTROL,
  OPERATION_MODE_SSID_SCAN
} operation_mode_enum;

operation_mode_enum operation_mode;

/* Parameters for menu display */
bool is_apply_manual_ssid;
int num_found_ssid;
int current_page;
int page_scale;
int ssid_cursor;

/* Gamepad ADC for joysticks */
Adafruit_ADS1115 ads;

/* Parameters for WiFi */
const IPAddress serverIP(192,168,4,1);
unsigned int localUdpPort = 6188;

/* V7RC communication settings */
String msg = "";
WiFiUDP Udp;  

/* Joystick pin settings */
#define JOYSTICK_BTN_S 16
#define JOYSTICK_BTN_P 32
#define SW_A           2
#define SW_B           23

/* left joystick */
uint16_t l_pos_y = 0;
int16_t l_offset_y = 0;
uint16_t   l_pos_y_v = 0;
uint16_t l_pos_x = 0;
int16_t l_offset_x = 0;
uint16_t   l_pos_x_v = 0;

/* right joystick */
uint16_t r_pos_y = 0;
int16_t r_offset_y = 0;
uint16_t   r_pos_y_v = 0;
uint16_t r_pos_x = 0;
int16_t r_offset_x = 0;
uint16_t   r_pos_x_v = 0;

/* button state */
byte lastButtonStateJOYS = LOW;
byte lastButtonStateJOYP = LOW;
byte lastButtonStateSWA = LOW;
byte lastButtonStateSWB = LOW;

byte ButtonStateJOYS = LOW;
byte ButtonStateJOYP = LOW;
byte ButtonStateSWA = LOW;
byte ButtonStateSWB = LOW;

/* avoid rebounce effect of button */
unsigned long lastTimeButtonStateChangedJOYS = 0;
unsigned long lastTimeButtonStateChangedJOYP = 0;
unsigned long lastTimeButtonStateChangedSWA = 0;
unsigned long lastTimeButtonStateChangedSWB = 0;

unsigned long debounceDuration = 100; // millis

/* set resolution of ADS1115 */
float Scalevoltage = 0.0001875;
/*
  ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
*/

byte getButtonState(unsigned long lastTimeButtonStateChanged,
                    unsigned long debounceDurationLocal,
                    int           btn_pin,
                    byte          buttonState,
                    byte          lastButtonState)
{
  byte tmpButtonState = buttonState;
  if (millis() - lastTimeButtonStateChanged > debounceDurationLocal) {
      tmpButtonState = digitalRead(btn_pin);
      if (tmpButtonState != lastButtonState) {
      lastTimeButtonStateChanged = millis();
      lastButtonState = tmpButtonState;
    }
  }

  return tmpButtonState;
}

/* 
 *  Because map command cannot exactly bound converted output values between lowerbound and upperboudn,
 *  this function is used to bound output values to
 *       (1) lowerbound, when they are below the lowerbound;
 *       (2) upperbound, when they are above the upperbound.
 */
uint16_t commandMapping(int x, int lowerest_input, int highest_input, int lowerbound, int upperbound)
{
   return map(x, lowerest_input, highest_input, lowerbound, upperbound) < lowerbound?
          lowerbound:
          map(x, lowerest_input, highest_input, lowerbound, upperbound) > upperbound?
          upperbound:
          map(x, lowerest_input, highest_input, lowerbound, upperbound);
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  pinMode(JOYSTICK_BTN_S, INPUT_PULLUP);
  pinMode(JOYSTICK_BTN_P, INPUT_PULLUP);
  pinMode(SW_A, INPUT_PULLUP);
  pinMode(SW_B, INPUT_PULLUP);

  /* initialize operation mode */
  operation_mode = OPERATION_MODE_CONTROL;
  is_apply_manual_ssid = false;

#ifdef __OLED__
  /* initialize OLED configuration */
  init_oled();
#endif

  /* initialize prefix of WiFi ssid to be connected */
  prefix_init();

  /* initialize WiFi configuration */
  init_wifi();

  /* initialize Gamepad ADC configuration */
  ads.setGain(GAIN_TWOTHIRDS);
  ads.begin();

  Udp.begin(localUdpPort);
}

void get_joystick_status()
{
  r_pos_x_v = (ads.readADC_SingleEnded(0) * Scalevoltage) * 100;
  r_pos_y_v = (ads.readADC_SingleEnded(1) * Scalevoltage) * 100;
  l_pos_x_v = (ads.readADC_SingleEnded(2) * Scalevoltage) * 100;
  l_pos_y_v = (ads.readADC_SingleEnded(3) * Scalevoltage) * 100;

  /* to convert analog signal to V7RC command
   * [NOTE] the direction of x's are opposite to those of V7RC
   */
  r_pos_x = (2000-commandMapping(r_pos_x_v, 0, 333, 1000, 2000))+1000;
  r_pos_y = commandMapping(r_pos_y_v, 0, 333, 1000, 2000);
  l_pos_x = (2000-commandMapping(l_pos_x_v, 0, 333, 1000, 2000))+1000;
  l_pos_y = commandMapping(l_pos_y_v, 0, 333, 1000, 2000);

  /* to get button status */
  ButtonStateJOYS = getButtonState(lastTimeButtonStateChangedJOYS,
                                  debounceDuration,
                                  JOYSTICK_BTN_S,
                                  ButtonStateJOYS,
                                  lastButtonStateJOYS);

  ButtonStateJOYP = getButtonState(lastTimeButtonStateChangedJOYP,
                                  debounceDuration,
                                  JOYSTICK_BTN_P,
                                  ButtonStateJOYP,
                                  lastButtonStateJOYP);

  ButtonStateSWA = getButtonState(lastTimeButtonStateChangedSWA,
                                  debounceDuration,
                                  SW_A,
                                  ButtonStateSWA,
                                  lastButtonStateSWA);

  ButtonStateSWB = getButtonState(lastTimeButtonStateChangedSWB,
                                  debounceDuration,
                                  SW_B,
                                  ButtonStateSWB,
                                  lastButtonStateSWB);
}

void control_mode_handler()
{
  /* compose V7RC command based on V7RC protocol */
  msg = "SRV" + String(l_pos_x) + String(l_pos_y) + String(r_pos_y) + String(r_pos_x) + "1500" + "1500" + "1500" + "1500";

  /* transmit V7RC command via UDP */
  Udp.beginPacket(serverIP, localUdpPort); //準備傳送資料
  Udp.write((uint8_t*) msg.c_str(),msg.length()+1); //複製資料到傳送快取
  Udp.endPacket();            //傳送資料

  /* 
   *  Enter SSID SCAN operation mode with the following gamepad status
   *  (1) pull left joystick to lower left position; and
   *  (2) press SWA button; and
   *  (3) press SWB button
   */
  if ((1250 > l_pos_x) &&
      (1250 > l_pos_y) &&
      (0 == ButtonStateSWA) &&
      (0 == ButtonStateSWB))
  {
    operation_mode = OPERATION_MODE_SSID_SCAN;

#ifdef __OLED__
    setOledClear();
    setOledCursor(0,0);
    setOledPrint("Entering Manual Mode...");
#endif
  }

#ifdef __LOG__
  Serial.print("l_pos_x: ");
  Serial.print(l_pos_x);

  Serial.print(", l_pos_y: ");
  Serial.print(l_pos_y);

  Serial.print(", r_pos_x: ");
  Serial.print(r_pos_x);

  Serial.print(", r_pos_y: ");
  Serial.print(r_pos_y);

  Serial.print(", joy_btn_s: ");
  Serial.print(ButtonStateJOYS);

  Serial.print(", joy_btn_p: ");
  Serial.print(ButtonStateJOYP);

  Serial.print(", sw_a: ");
  Serial.print(ButtonStateSWA);

  Serial.print(", sw_b: ");
  Serial.println(ButtonStateSWB);

  Serial.print("l_pos_x_v: ");
  Serial.print(l_pos_x_v);

  Serial.print(", l_pos_y_v: ");
  Serial.print(l_pos_y_v);

  Serial.print(", r_pos_x_v: ");
  Serial.print(r_pos_x_v);

  Serial.print(", r_pos_y_v: ");
  Serial.println(r_pos_y_v);
#endif
}


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

    /* display ssid cursor */
    setOledCursor(0,((int) (ssid_cursor%page_scale))*10+10);
    setOledPrint(">");
#endif
}

void update_menu_selection_status()
{
  if (1750 < r_pos_y)
  {
    if (ssid_cursor > 0)
    {
      ssid_cursor -= 1;
    }
  }

  if (1250 > r_pos_y)
  {
    if (ssid_cursor < num_found_ssid)
    {
      ssid_cursor += 1;
    }
  }
}

void trigger_ssid_connection()
{
  if (0 == ButtonStateJOYP)
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
    operation_mode = OPERATION_MODE_CONTROL;
    is_apply_manual_ssid = false;
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
  update_menu_selection_status();
  /* trigger WiFi reconnection when a scaned ssid has been selected by user */
  trigger_ssid_connection();
}

void loop() {
  /*  The execution flow for loop is
   *  (1) to get joystick status
   *  (2) invoke the main functions based on operation mode
   */
  get_joystick_status();

  if (OPERATION_MODE_CONTROL == operation_mode)
  {
    control_mode_handler();
  }
  else if (OPERATION_MODE_SSID_SCAN == operation_mode)
  {
    ssid_scan_mode_handler();
  }
  else
  {
    ;
  }

  delay(10);
}
