#include "Options.h"
#include <Adafruit_ADS1X15.h>
#include "utility.h"

/* Gamepad ADC for joysticks */
Adafruit_ADS1115 ads;

/* Joystick pin settings */
#define JOYSTICK_BTN_S 16
#define JOYSTICK_BTN_P 32
#define SW_A           2
#define SW_B           23

/* left joystick */
int16_t l_pos_y = 0;
int16_t l_offset_y = 0;
int16_t l_pos_y_v = 0;
int16_t l_pos_x = 0;
int16_t l_offset_x = 0;
int16_t l_pos_x_v = 0;
int16_t cal_l_x_offset = 0;
int16_t cal_l_y_offset = 0;

/* right joystick */
int16_t r_pos_y = 0;
int16_t r_offset_y = 0;
int16_t r_pos_y_v = 0;
int16_t r_pos_x = 0;
int16_t r_offset_x = 0;
int16_t r_pos_x_v = 0;
int16_t cal_r_x_offset = 0;
int16_t cal_r_y_offset = 0;

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

int16_t setBound(int16_t x, int16_t lower, int16_t upper)
{
  if (x < lower)
  {
    return lower;
  }
  else if (x > upper)
  {
    return upper;
  }
  else
  {
    return x;
  }
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

void get_joystick_status()
{
#ifdef __LEFT_HAND_THROTTLE__
  r_pos_x_v = (ads.readADC_SingleEnded(1) * Scalevoltage) * 100;
  r_pos_y_v = (ads.readADC_SingleEnded(0) * Scalevoltage) * 100;
  l_pos_x_v = (ads.readADC_SingleEnded(3) * Scalevoltage) * 100;
  l_pos_y_v = (ads.readADC_SingleEnded(2) * Scalevoltage) * 100;
#endif

#ifdef __RIGHT_HAND_THROTTLE__
  r_pos_x_v = (ads.readADC_SingleEnded(3) * Scalevoltage) * 100;
  r_pos_y_v = (ads.readADC_SingleEnded(2) * Scalevoltage) * 100;
  l_pos_x_v = (ads.readADC_SingleEnded(1) * Scalevoltage) * 100;
  l_pos_y_v = (ads.readADC_SingleEnded(0) * Scalevoltage) * 100;
#endif

  /* to convert analog signal to V7RC command
   * [NOTE] the direction of x's are opposite to those of V7RC
   */
  r_pos_x = setBound((2000-commandMapping(r_pos_x_v, 0, 333, 1000, 2000))+1000-cal_r_x_offset, 1000, 2000);
  r_pos_y = setBound((2000-commandMapping(r_pos_y_v, 0, 333, 1000, 2000))+1000-cal_r_y_offset, 1000, 2000);
  l_pos_x = setBound((2000-commandMapping(l_pos_x_v, 0, 333, 1000, 2000))+1000-cal_l_x_offset, 1000, 2000);
  l_pos_y = setBound((2000-commandMapping(l_pos_y_v, 0, 333, 1000, 2000))+1000-cal_l_y_offset, 1000, 2000);

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

/* initialize joystick buttons and ads1115 */
void init_joystick()
{  
  pinMode(JOYSTICK_BTN_S, INPUT_PULLUP);
  pinMode(JOYSTICK_BTN_P, INPUT_PULLUP);
  pinMode(SW_A, INPUT_PULLUP);
  pinMode(SW_B, INPUT_PULLUP);

  ads.setGain(GAIN_TWOTHIRDS);
  ads.begin();
}

/* left joystick */
int16_t get_l_pos_y()
{
  return l_pos_y;
}

int16_t get_l_offset_y()
{
  return l_offset_y;
}

int16_t get_l_pos_y_v()
{
  return l_pos_y_v;
}

int16_t get_l_pos_x()
{
  return l_pos_x;
}

int16_t get_l_offset_x()
{
  return l_offset_x;
}

int16_t get_l_pos_x_v()
{
  return l_pos_x_v;
}

/* right joystick */
int16_t get_r_pos_y()
{
  return r_pos_y;
}

int16_t get_r_offset_y()
{
  return r_offset_y;
}

int16_t get_r_pos_y_v()
{
  return r_pos_y_v;
}

int16_t get_r_pos_x()
{
  return r_pos_x;
}

int16_t get_r_offset_x()
{
  return r_offset_x;
}

int16_t get_r_pos_x_v()
{
  return r_pos_x_v;
}

/* button state */
byte get_ButtonStateJOYS()
{
  return ButtonStateJOYS;
}

byte get_ButtonStateJOYP()
{
  return ButtonStateJOYP;
}

byte get_ButtonStateSWA()
{
  return ButtonStateSWA;
}

byte get_ButtonStateSWB()
{
  return ButtonStateSWB;
}

void cal_write(int16_t cal_array[], int size)
{
#ifdef __LOG__
  Serial.println("Writing CALIBRATION");
#endif
  /* Write CALIBRATION to EEPROM 200~210 */
  for (int i = 0; i < size; i++)
  {
    write_eeprom((200+i*5), cal_array[i] >> 8);
    write_eeprom((200+i*5)+1, cal_array[i]);
  }
}

int16_t cal_read(int idx)
{
  byte byte1 = read_eeprom((200+idx*5));
  byte byte2 = read_eeprom((200+idx*5)+1);

  Serial.println(((byte1 << 8)+byte2));

  return ((byte1 << 8)+byte2);
}

void set_cal_l_x_offset(int16_t x_offset)
{
#ifdef __LOG__
  Serial.print("set_cal_l_x_offset: ");
  Serial.println(x_offset);
#endif /* __LOG__ */

  cal_l_x_offset = x_offset;
}

void set_cal_l_y_offset(int16_t y_offset)
{
#ifdef __LOG__
  Serial.print("set_cal_l_y_offset: ");
  Serial.println(y_offset);
#endif /* __LOG__ */

  cal_l_y_offset = y_offset;
}

void set_cal_r_x_offset(int16_t x_offset)
{
#ifdef __LOG__
  Serial.print("set_cal_r_x_offset: ");
  Serial.println(x_offset);
#endif /* __LOG__ */

  cal_r_x_offset = x_offset;
}

void set_cal_r_y_offset(int16_t y_offset)
{
#ifdef __LOG__
  Serial.print("set_cal_r_y_offset: ");
  Serial.println(y_offset);
#endif /* __LOG__ */

  cal_r_y_offset = y_offset;
}

int16_t get_cal_l_x_offset()
{
  return cal_l_x_offset;
}

int16_t get_cal_l_y_offset()
{
  return cal_l_y_offset;
}

int16_t get_cal_r_x_offset()
{
  return cal_r_x_offset;
}

int16_t get_cal_r_y_offset()
{
  return cal_r_y_offset;
}

void read_cal_from_flash()
{
  uint8_t num_cal = 4;

  for (int i = 0; i < num_cal; i++)
  {
    switch (i)
    {
      case 0:
        set_cal_l_x_offset(cal_read(i));
        break;
      case 1:
        set_cal_l_y_offset(cal_read(i));
        break;
      case 2:
        set_cal_r_x_offset(cal_read(i));
        break;
      case 3:
        set_cal_r_y_offset(cal_read(i));
        break;
      default:
        break;
    }

#ifdef __LOG__
    Serial.print("cal_read[" + String(i) + "]: ");
    Serial.println(cal_read(i));
#endif
  }
}
