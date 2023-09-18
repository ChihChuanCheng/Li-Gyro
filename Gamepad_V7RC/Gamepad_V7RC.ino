/*
 * Provided by 飛行造物
 * This code implements V7RC gamepad
 * It is developed based on ESP32-BLE-Gamepad library (https://github.com/lemmingDev/ESP32-BLE-Gamepad)
 */

#include <Arduino.h>
#include <BleGamepad.h>
#include <Adafruit_ADS1X15.h>

/* left joystick */
int16_t l_pos_y = 0;
int16_t l_offset_y = 0;
int16_t l_pos_y_v = 0;
int16_t l_pos_x = 0;
int16_t l_offset_x = 0;
int16_t l_pos_x_v = 0;

/* right joystick */
int16_t r_pos_y = 0;
int16_t r_offset_y = 0;
int16_t r_pos_y_v = 0;
int16_t r_pos_x = 0;
int16_t r_offset_x = 0;
int16_t r_pos_x_v = 0;

BleGamepad bleGamepad;

Adafruit_ADS1115 ads;

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

/* 
 *  Because map command cannot exactly bound converted output values between lowerbound and upperboudn,
 *  this function is used to bound output values to
 *       (1) lowerbound, when they are below the lowerbound;
 *       (2) upperbound, when they are above the upperbound.
 */
int16_t commandMapping(int x, int lowerest_input, int highest_input, int lowerbound, int upperbound)
{
   return map(x, lowerest_input, highest_input, lowerbound, upperbound) < lowerbound?
          lowerbound:
          map(x, lowerest_input, highest_input, lowerbound, upperbound) > upperbound?
          upperbound:
          map(x, lowerest_input, highest_input, lowerbound, upperbound);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting BLE work!");

    ads.setGain(GAIN_TWOTHIRDS);
    ads.begin();

    bleGamepad.begin();
    // The default bleGamepad.begin() above enables 16 buttons, all axes, one hat, and no simulation controls or special buttons
}

void loop()
{
    if (bleGamepad.isConnected())
    {
        r_pos_x_v = (ads.readADC_SingleEnded(0) * Scalevoltage) * 100;
        r_pos_y_v = (ads.readADC_SingleEnded(1) * Scalevoltage) * 100;
        l_pos_x_v = (ads.readADC_SingleEnded(2) * Scalevoltage) * 100;
        l_pos_y_v = (ads.readADC_SingleEnded(3) * Scalevoltage) * 100;
      
        /* to convert analog signal to V7RC command
         * [NOTE] the direction of x's are opposite to those of V7RC
         */
        r_pos_x = 32767 - commandMapping(r_pos_x_v, 0, 333, 0, 32767);
        r_pos_y = 32767 - commandMapping(r_pos_y_v, 0, 333, 0, 32767);
        l_pos_x = 32767 - commandMapping(l_pos_x_v, 0, 333, 0, 32767);
        l_pos_y = 32767 - commandMapping(l_pos_y_v, 0, 333, 0, 32767);
        bleGamepad.setAxes(l_pos_x, l_pos_y, r_pos_x, r_pos_y, 0, 0, 0, 0);
        delay(10);

        Serial.print("l_pos_x: ");
        Serial.print(l_pos_x);
      
        Serial.print(", l_pos_y: ");
        Serial.print(l_pos_y);
      
        Serial.print(", r_pos_x: ");
        Serial.print(r_pos_x);
      
        Serial.print(", r_pos_y: ");
        Serial.println(r_pos_y);
    }
}
