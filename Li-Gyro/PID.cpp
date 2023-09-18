#include "Options.h"
#include "Arduino.h"
#include "Sensor.h"

//Normalized desired state:
float thro_des, roll_des, pitch_des, yaw_des;
float roll_passthru, pitch_passthru, yaw_passthru;

//Radio comm:
unsigned long channel_1_pwm, channel_2_pwm, channel_3_pwm; //, channel_4_pwm, channel_5_pwm, channel_6_pwm;
unsigned long channel_1_pwm_prev, channel_2_pwm_prev, channel_3_pwm_prev; //, channel_4_pwm_prev;

//Controller:
float error_roll, error_roll_prev, roll_des_prev, integral_roll, integral_roll_il, integral_roll_ol, integral_roll_prev, integral_roll_prev_il, integral_roll_prev_ol, derivative_roll, roll_PID = 0;
float error_pitch, error_pitch_prev, pitch_des_prev, integral_pitch, integral_pitch_il, integral_pitch_ol, integral_pitch_prev, integral_pitch_prev_il, integral_pitch_prev_ol, derivative_pitch, pitch_PID = 0;
float error_yaw, error_yaw_prev, integral_yaw, integral_yaw_prev, derivative_yaw, yaw_PID = 0;

//Mixer
float m1_command_scaled, m2_command_scaled;
int m1_command_PWM, m2_command_PWM;
float s1_command_scaled, s2_command_scaled;
int s1_command_PWM, s2_command_PWM;

//Controller parameters (take note of defaults before modifying!): 
float i_limit = 25.0;  // Integrator saturation level, mostly for safety (default 25.0)
float maxRoll = 40.0;  // default: 30.0;  //Max roll angle in degrees for angle mode (maximum 60 degrees), deg/sec for rate mode 
float maxPitch = 40.0; // default: 30.0;  //Max pitch angle in degrees for angle mode (maximum 60 degrees), deg/sec for rate mode
float maxYaw = 160;    // Max yaw rate in deg/sec

float Kp_roll_angle = 0.5;  // default: 0.2;  //Roll P-gain - angle mode 
float Ki_roll_angle = 0.3;  // default: 0.3;  // Roll I-gain - angle mode
float Kd_roll_angle = 0.05; // default: 0.05; //Roll D-gain - angle mode (if using controlANGLE2(), has no effect. Use B_loop_roll)
float Kp_pitch_angle = 0.9; // default: 0.2;  //Pitch P-gain - angle mode
float Ki_pitch_angle = 0.3; // default: 0.3;  //Pitch I-gain - angle mode
float Kd_pitch_angle = 0.1; // default: 0.05; //Pitch D-gain - angle mode (if using controlANGLE2(), has no effect. Use B_loop_pitch)
float Kp_yaw = 0.3;         // Yaw P-gain
float Ki_yaw = 0;           // Yaw I-gain
float Kd_yaw = 0;           // Yaw D-gain (be careful when increasing too high, motors will begin to overheat!)

float setBound(float x, float lower, float upper)
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

void getDesState() {
  //DESCRIPTION: Normalizes desired control values to appropriate values
  /*
   * Updates the desired state variables thro_des, roll_des, pitch_des, and yaw_des. These are computed by using the raw
   * RC pwm commands and scaling them to be within our limits defined in setup. thro_des stays within 0 to 1 range.
   * roll_des and pitch_des are scaled to be within max roll/pitch amount in either degrees (angle mode) or degrees/sec
   * (rate mode). yaw_des is scaled to be within max yaw in degrees/sec. Also creates roll_passthru, pitch_passthru, and
   * yaw_passthru variables, to be used in commanding motors/servos with direct unstabilized commands in controlMixer().
   */
  thro_des = (channel_1_pwm - 1000.0)/1000.0; //between 0 and 1
  roll_des = (channel_2_pwm - 1500.0)/500.0;  //between -1 and 1
  pitch_des = (channel_3_pwm - 1500.0)/500.0; //between -1 and 1
  yaw_des = (channel_2_pwm - 1500.0)/500.0;   //between -1 and 1
  
  //Constrain within normalized bounds
  thro_des = setBound(thro_des, 0.0, 1.0);             //between 0 and 1
  roll_des = setBound(roll_des, -1.0, 1.0)*maxRoll;    //between -maxRoll and +maxRoll
  pitch_des = setBound(pitch_des, -1.0, 1.0)*maxPitch; //between -maxPitch and +maxPitch
  yaw_des = setBound(yaw_des, -0.5, 0.5)*maxYaw;       //Between -maxYaw and +maxYaw

  //allow plane to make turns at fixed location
  if (thro_des < 0.1)
  {
    yaw_des = yaw_des * 2;
  }
}

void setRadioComm(unsigned long ch1_pwm, unsigned long ch2_pwm, unsigned long ch3_pwm)
{
  channel_1_pwm = ch1_pwm;
  channel_2_pwm = ch2_pwm;
  channel_3_pwm = ch3_pwm;
}

void controlANGLE() {
  //DESCRIPTION: Computes control commands based on state error (angle)
  /*
   * Basic PID control to stablize on angle setpoint based on desired states roll_des, pitch_des, and yaw_des computed in 
   * getDesState(). Error is simply the desired state minus the actual state (ex. roll_des - roll_IMU). Two safety features
   * are implimented here regarding the I terms. The I terms are saturated within specified limits on startup to prevent 
   * excessive buildup. This can be seen by holding the vehicle at an angle and seeing the motors ramp up on one side until
   * they've maxed out throttle...saturating I to a specified limit fixes this. The second feature defaults the I terms to 0
   * if the throttle is at the minimum setting. This means the motors will not start spooling up on the ground, and the I 
   * terms will always start from 0 on takeoff. This function updates the variables roll_PID, pitch_PID, and yaw_PID which
   * can be thought of as 1-D stablized signals. They are mixed to the configuration of the vehicle in controlMixer().
   */
  //Roll
  error_roll = roll_des - getDeviceAngleY();     // Horizontal Startup
  integral_roll = integral_roll_prev + error_roll*getDT();
  if (channel_1_pwm < 1060) {   //don't let integrator build if throttle is too low
    integral_roll = 0;
  }
  integral_roll = setBound(integral_roll, -i_limit, i_limit); //saturate integrator to prevent unsafe buildup
  derivative_roll = getDeviceGyX();
  roll_PID = 0.01*(Kp_roll_angle*error_roll + Ki_roll_angle*integral_roll - Kd_roll_angle*derivative_roll); //scaled by .01 to bring within -1 to 1 range

  //Pitch
  error_pitch = pitch_des - getDeviceAngleX();
  integral_pitch = integral_pitch_prev + error_pitch*getDT();
  if (channel_1_pwm < 1060) {   //don't let integrator build if throttle is too low
    integral_pitch = 0;
  }
  integral_pitch = setBound(integral_pitch, -i_limit, i_limit); //saturate integrator to prevent unsafe buildup
  derivative_pitch = getDeviceGyY();
  pitch_PID = .01*(Kp_pitch_angle*error_pitch + Ki_pitch_angle*integral_pitch - Kd_pitch_angle*derivative_pitch); //scaled by .01 to bring within -1 to 1 range

  //Yaw, stablize on rate from GyroZ
  error_yaw = yaw_des - getDeviceGyZ();
  integral_yaw = integral_yaw_prev + error_yaw*getDT();
  if (channel_1_pwm < 1060) {   //Don't let integrator build if throttle is too low
    integral_yaw = 0;
  }
  integral_yaw = setBound(integral_yaw, -i_limit, i_limit); //Saturate integrator to prevent unsafe buildup
  derivative_yaw = (error_yaw - error_yaw_prev)/getDT(); 
  yaw_PID = .01*(Kp_yaw*error_yaw + Ki_yaw*integral_yaw + Kd_yaw*derivative_yaw); //Scaled by .01 to bring within -1 to 1 range

#ifdef __LOG__
  Serial.print(F(" error_yaw_prev: "));
  Serial.print(error_yaw_prev);
  Serial.print(F(" error_yaw: "));
  Serial.print(error_yaw);
  Serial.print(F(" integral_yaw: "));
  Serial.println(integral_yaw);
  Serial.print(F(" derivative_yaw: "));
  Serial.print(derivative_yaw);
  Serial.print(F(" getDT: "));
  Serial.print(getDT(),3);
  Serial.println();
#endif

  //Update roll variables
  integral_roll_prev = integral_roll;
  //Update pitch variables
  integral_pitch_prev = integral_pitch;
  //Update yaw variables
  error_yaw_prev = error_yaw;
  integral_yaw_prev = integral_yaw;
}

void controlMixer() {
  //DESCRIPTION: Mixes scaled commands from PID controller to actuator outputs based on vehicle configuration
  /*
   * Takes roll_PID, pitch_PID, and yaw_PID computed from the PID controller and appropriately mixes them for the desired
   * vehicle configuration. For example on a quadcopter, the left two motors should have +roll_PID while the right two motors
   * should have -roll_PID. Front two should have -pitch_PID and the back two should have +pitch_PID etc... every motor has
   * normalized (0 to 1) thro_des command for throttle control. Can also apply direct unstabilized commands from the transmitter with 
   * roll_passthru, pitch_passthru, and yaw_passthu. mX_command_scaled and sX_command scaled variables are used in scaleCommands() 
   * in preparation to be sent to the motor ESCs and servos.
   */
  /* Wright Flyer (differential trust) */
  int yaw_weight = 0.2;
  m1_command_scaled = thro_des+yaw_weight*(yaw_des/maxYaw)+yaw_PID;
  m2_command_scaled = thro_des-yaw_weight*(yaw_des/maxYaw)-yaw_PID;

  s1_command_scaled = 0;
  s2_command_scaled = 0;

  /* Hovercraft (lift control plus differential trust on servos) */
  /*
   * m1_command_scaled = thro_des;
   * m2_command_scaled = thro_des;
   *
   * s1_command_scaled = (pitch_des/maxPitch)+(yaw_des/maxYaw)+yaw_PID;
   * s2_command_scaled = (pitch_des/maxPitch)-(yaw_des/maxYaw)-yaw_PID;
   */

  /* Fixed wing */
  /* m1_command_scaled = thro_des;
   * m2_command_scaled = thro_des;
   *
   * s1_command_scaled = (yaw_des/maxYaw)+yaw_PID;
   * s2_command_scaled = (pitch_des/maxPitch)-pitch_PID;
   */
}

void scaleCommands() {
  //DESCRIPTION: Scale normalized actuator commands to values for ESC/Servo protocol
  /*
   * mX_command_scaled variables from the mixer function are scaled to 125-250us for OneShot125 protocol. sX_command_scaled variables from
   * the mixer function are scaled to 0-180 for the servo library using standard PWM.
   * mX_command_PWM are updated here which are used to command the motors in commandMotors(). sX_command_PWM are updated 
   * which are used to command the servos.
   */
  //Scaled to 125us - 250us for oneshot125 protocol
  m1_command_PWM = m1_command_scaled*255;
  //Constrain commands to motors within oneshot125 bounds
  m1_command_PWM = setBound(m1_command_PWM, 0, 255);

  //Scaled to 125us - 250us for oneshot125 protocol
  m2_command_PWM = m2_command_scaled*255;
  //Constrain commands to motors within oneshot125 bounds
  m2_command_PWM = setBound(m2_command_PWM, 0, 255);

  s1_command_PWM = s1_command_scaled+90;
  //Constrain commands to motors within oneshot125 bounds
  s1_command_PWM = setBound(s1_command_PWM, 0, 180);

  s2_command_PWM = s2_command_scaled+90;
  //Constrain commands to motors within oneshot125 bounds
  s2_command_PWM = setBound(s2_command_PWM, 0, 180);
}

int getThoCommand()
{
  return m1_command_PWM;
}

int getThoCommand2()
{
  return m2_command_PWM;
}

int getRudderCommand()
{
  return s1_command_PWM;
}

int getElevatorCommand()
{
  return s2_command_PWM;
}

void printDesiredState() {
  Serial.print(F("thro_des: "));
  Serial.print(thro_des);
  Serial.print(F(" roll_des: "));
  Serial.print(roll_des);
  Serial.print(F(" pitch_des: "));
  Serial.print(pitch_des);
  Serial.print(F(" yaw_des: "));
  Serial.println(yaw_des);
}

void printCommands()
{
  Serial.print(F("m1_command: "));
  Serial.print(m1_command_PWM);
  Serial.print(F(" m2_command: "));
  Serial.print(m2_command_PWM);
  Serial.print(F(" s1_command: "));
  Serial.print(s1_command_PWM);
  Serial.print(F(" s2_command: "));
  Serial.println(s2_command_PWM);
}

void printPIDoutput() {
  Serial.print(F("roll_PID: "));
  Serial.print(roll_PID);
  Serial.print(F(" pitch_PID: "));
  Serial.print(pitch_PID);
  Serial.print(F(" yaw_PID: "));
  Serial.println(yaw_PID);
  
}
