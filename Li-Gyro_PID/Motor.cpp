#include "Options.h"
#include <Servo.h>

/* esp07 */
#define RUDDER    12
#define ELEVATOR  13
#define THROTTLE1  5
#define THROTTLE2  4

const uint8_t full_thr = 255;

Servo servoRudder;
Servo servoElevator;

void init_motor() {
  /* initialize servo */
  servoRudder.attach(RUDDER);
  servoRudder.write(90);
  servoElevator.attach(ELEVATOR);
  servoElevator.write(90);

  /* initialize dc motor */
  pinMode(THROTTLE1, OUTPUT);
  analogWrite(THROTTLE1, 0);
  pinMode(THROTTLE2, OUTPUT);
  analogWrite(THROTTLE2, 0);
  
  /* set a new input range for throttle */
  analogWriteRange(full_thr);
}

void motor_control(uint8_t rudder, uint8_t elevator, uint8_t throttle1, uint8_t throttle2) {
#ifdef __LOG__
  Serial.print("Rudder: ");
  Serial.print(rudder);
  Serial.print("elevator: ");
  Serial.print(elevator);
  Serial.print(" throttle1: ");
  Serial.print(throttle1);
  Serial.print(" throttle2: ");
  Serial.println(throttle2);
#endif /* __LOG__ */

  /* commmand rudder and motor */
  servoRudder.write(rudder);
  servoElevator.write(elevator);
  analogWrite(THROTTLE1, throttle1);
  analogWrite(THROTTLE2, throttle2);
}
