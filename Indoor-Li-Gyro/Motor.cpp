#include "Options.h"
#include "Arduino.h"

/* esp07 */
#define RUDDER0    12
#define RUDDER1    13
#define THROTTLE1  5
#define THROTTLE2  4

const uint8_t full_thr = 255;

void init_motor() {
  /* initialize dc motor */
  pinMode(THROTTLE1, OUTPUT);
  analogWrite(THROTTLE1, 0);
  pinMode(THROTTLE2, OUTPUT);
  analogWrite(THROTTLE2, 0);
  pinMode(RUDDER0, OUTPUT);
  analogWrite(RUDDER0, 0);
  pinMode(RUDDER1, OUTPUT);
  analogWrite(RUDDER1, 0);
  
  /* set a new input range for throttle */
  analogWriteRange(full_thr);
}

void motor_control(uint8_t rudder0, uint8_t rudder1, uint8_t throttle1, uint8_t throttle2) {
#ifdef __LOG__
  Serial.print("Rudder0: ");
  Serial.print(rudder0);
  Serial.print("Rudder1: ");
  Serial.print(rudder1);
  Serial.print(" throttle1: ");
  Serial.print(throttle1);
  Serial.print(" throttle2: ");
  Serial.println(throttle2);
#endif /* __LOG__ */

  /* commmand rudder and motor */
  analogWrite(RUDDER0, rudder0);
  analogWrite(RUDDER1, rudder1);
  analogWrite(THROTTLE1, throttle1);
  analogWrite(THROTTLE2, throttle2);
}
