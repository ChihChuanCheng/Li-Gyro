#include "Options.h"
#include <Adafruit_ADS1X15.h>
#include "utility.h"
#include "joystick.h"

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
int16_t cal_center[4] = {166, 166, 166, 166};
int16_t cal_min[4] = {0, 0, 0, 0};
int16_t cal_max[4] = {333, 333, 333, 333};

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

byte ButtonPressedJOYS = LOW;
byte ButtonPressedJOYP = LOW;
byte ButtonPressedSWA = LOW;
byte ButtonPressedSWB = LOW;

byte ButtonReleasedJOYS = LOW;
byte ButtonReleasedJOYP = LOW;
byte ButtonReleasedSWA = LOW;
byte ButtonReleasedSWB = LOW;

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

byte getButtonState(unsigned long& lastTimeButtonStateChanged,
                    unsigned long  debounceDurationLocal,
                    int            btn_pin,
                    byte&          buttonState,
                    byte&          lastButtonState)
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

static int16_t calibratedCommandMapping(int16_t raw, uint8_t axisIndex)
{
  int16_t center = cal_center[axisIndex];
  int16_t low = cal_min[axisIndex];
  int16_t high = cal_max[axisIndex];
  int32_t mapped = 1500;

  if (center - low < 10 || high - center < 10)
  {
    mapped = commandMapping(raw, 0, 333, 1000, 2000);
  }
  else if (raw >= center)
  {
    mapped = map(raw, center, high, 1500, 2000);
  }
  else
  {
    mapped = map(raw, low, center, 1000, 1500);
  }

  if (mapped > 1492 && mapped < 1508)
  {
    mapped = 1500;
  }

  return setBound(3000 - mapped, 1000, 2000);
}

void get_joystick_status()
{
  byte prevButtonStateJOYS = ButtonStateJOYS;
  byte prevButtonStateJOYP = ButtonStateJOYP;
  byte prevButtonStateSWA = ButtonStateSWA;
  byte prevButtonStateSWB = ButtonStateSWB;

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
  r_pos_x = calibratedCommandMapping(r_pos_x_v, 2);
  r_pos_y = calibratedCommandMapping(r_pos_y_v, 3);
  l_pos_x = calibratedCommandMapping(l_pos_x_v, 0);
  l_pos_y = calibratedCommandMapping(l_pos_y_v, 1);

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

  ButtonPressedJOYS = (HIGH == prevButtonStateJOYS) && (LOW == ButtonStateJOYS);
  ButtonPressedJOYP = (HIGH == prevButtonStateJOYP) && (LOW == ButtonStateJOYP);
  ButtonPressedSWA = (HIGH == prevButtonStateSWA) && (LOW == ButtonStateSWA);
  ButtonPressedSWB = (HIGH == prevButtonStateSWB) && (LOW == ButtonStateSWB);

  ButtonReleasedJOYS = (LOW == prevButtonStateJOYS) && (HIGH == ButtonStateJOYS);
  ButtonReleasedJOYP = (LOW == prevButtonStateJOYP) && (HIGH == ButtonStateJOYP);
  ButtonReleasedSWA = (LOW == prevButtonStateSWA) && (HIGH == ButtonStateSWA);
  ButtonReleasedSWB = (LOW == prevButtonStateSWB) && (HIGH == ButtonStateSWB);
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

byte get_ButtonPressedJOYS()
{
  return ButtonPressedJOYS;
}

byte get_ButtonPressedJOYP()
{
  return ButtonPressedJOYP;
}

byte get_ButtonPressedSWA()
{
  return ButtonPressedSWA;
}

byte get_ButtonPressedSWB()
{
  return ButtonPressedSWB;
}

byte get_ButtonReleasedJOYS()
{
  return ButtonReleasedJOYS;
}

byte get_ButtonReleasedJOYP()
{
  return ButtonReleasedJOYP;
}

byte get_ButtonReleasedSWA()
{
  return ButtonReleasedSWA;
}

byte get_ButtonReleasedSWB()
{
  return ButtonReleasedSWB;
}

static const int CAL_STORE_ADDR = 200;
static const uint8_t CAL_STORE_MAGIC0 = 'C';
static const uint8_t CAL_STORE_MAGIC1 = 'F';
static const uint8_t CAL_STORE_VERSION = 2;
static const uint8_t CAL_LEGACY_MAGIC1 = '4';
static const uint8_t CAL_COUNT = 4;
static const uint8_t CAL_FULL_VALUES = 12;

static bool cal_store_valid()
{
  return (read_eeprom(CAL_STORE_ADDR) == CAL_STORE_MAGIC0) &&
         (read_eeprom(CAL_STORE_ADDR + 1) == CAL_STORE_MAGIC1) &&
         (read_eeprom(CAL_STORE_ADDR + 2) == CAL_STORE_VERSION) &&
         (read_eeprom(CAL_STORE_ADDR + 3) == CAL_FULL_VALUES);
}

static bool cal_legacy_store_valid()
{
  return (read_eeprom(CAL_STORE_ADDR) == CAL_STORE_MAGIC0) &&
         (read_eeprom(CAL_STORE_ADDR + 1) == CAL_LEGACY_MAGIC1) &&
         (read_eeprom(CAL_STORE_ADDR + 2) == 1) &&
         (read_eeprom(CAL_STORE_ADDR + 3) == CAL_COUNT);
}

static int16_t read_cal_int16(uint8_t idx)
{
  uint8_t data[2];
  eeprom_read_block(CAL_STORE_ADDR + 4 + idx * 2, data, sizeof(data));
  return (int16_t)(((uint16_t)data[0] << 8) | data[1]);
}

void cal_write(int16_t cal_array[], int size)
{
#ifdef __LOG__
  Serial.println("Writing CALIBRATION");
#endif

  uint8_t buffer[4 + CAL_COUNT * 2] = {0};
  int count = min(size, (int)CAL_COUNT);

  buffer[0] = CAL_STORE_MAGIC0;
  buffer[1] = CAL_STORE_MAGIC1;
  buffer[2] = CAL_STORE_VERSION;
  buffer[3] = CAL_COUNT;

  for (int i = 0; i < count; i++)
  {
    int offset = 4 + i * 2;
    buffer[offset] = (cal_array[i] >> 8) & 0xFF;
    buffer[offset + 1] = cal_array[i] & 0xFF;
  }

  eeprom_write_block(CAL_STORE_ADDR, buffer, sizeof(buffer));
}

void cal_full_write(int16_t center_array[], int16_t min_array[], int16_t max_array[], int size)
{
#ifdef __LOG__
  Serial.println("Writing FULL CALIBRATION");
#endif

  uint8_t buffer[4 + CAL_FULL_VALUES * 2] = {0};
  int count = min(size, (int)CAL_COUNT);

  buffer[0] = CAL_STORE_MAGIC0;
  buffer[1] = CAL_STORE_MAGIC1;
  buffer[2] = CAL_STORE_VERSION;
  buffer[3] = CAL_FULL_VALUES;

  for (int i = 0; i < count; i++)
  {
    int16_t values[3] = {center_array[i], min_array[i], max_array[i]};
    for (int j = 0; j < 3; j++)
    {
      int offset = 4 + (i * 3 + j) * 2;
      buffer[offset] = (values[j] >> 8) & 0xFF;
      buffer[offset + 1] = values[j] & 0xFF;
    }

    set_cal_center(i, center_array[i]);
    set_cal_min(i, min_array[i]);
    set_cal_max(i, max_array[i]);
  }

  eeprom_write_block(CAL_STORE_ADDR, buffer, sizeof(buffer));
}

int16_t cal_read(int idx)
{
  if ((idx < 0) || (idx >= CAL_COUNT) || !cal_legacy_store_valid())
  {
    return 0;
  }

  return read_cal_int16(idx);
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

int16_t get_cal_center(uint8_t idx)
{
  return (idx < CAL_COUNT) ? cal_center[idx] : 166;
}

int16_t get_cal_min(uint8_t idx)
{
  return (idx < CAL_COUNT) ? cal_min[idx] : 0;
}

int16_t get_cal_max(uint8_t idx)
{
  return (idx < CAL_COUNT) ? cal_max[idx] : 333;
}

void set_cal_center(uint8_t idx, int16_t value)
{
  if (idx < CAL_COUNT)
  {
    cal_center[idx] = value;
  }
}

void set_cal_min(uint8_t idx, int16_t value)
{
  if (idx < CAL_COUNT)
  {
    cal_min[idx] = value;
  }
}

void set_cal_max(uint8_t idx, int16_t value)
{
  if (idx < CAL_COUNT)
  {
    cal_max[idx] = value;
  }
}

void read_cal_from_flash()
{
  if (cal_store_valid())
  {
    for (int i = 0; i < CAL_COUNT; i++)
    {
      set_cal_center(i, read_cal_int16(i * 3));
      set_cal_min(i, read_cal_int16(i * 3 + 1));
      set_cal_max(i, read_cal_int16(i * 3 + 2));
    }
    return;
  }

  for (int i = 0; i < CAL_COUNT; i++)
  {
    switch (i)
    {
      case 0:
        set_cal_l_x_offset(cal_read(i));
        set_cal_center(i, 166 + get_cal_l_x_offset());
        break;
      case 1:
        set_cal_l_y_offset(cal_read(i));
        set_cal_center(i, 166 + get_cal_l_y_offset());
        break;
      case 2:
        set_cal_r_x_offset(cal_read(i));
        set_cal_center(i, 166 + get_cal_r_x_offset());
        break;
      case 3:
        set_cal_r_y_offset(cal_read(i));
        set_cal_center(i, 166 + get_cal_r_y_offset());
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
