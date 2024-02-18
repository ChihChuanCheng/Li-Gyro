#pragma once

void setRadioComm(unsigned long ch1_pwm, unsigned long ch2_pwm, unsigned long ch3_pwm);

//APIs for PID control
void getDesState();
void controlANGLE(); //stabilize on angle setpoint
void controlMixer();
void scaleCommands();

//APIS for motor command provisioning
int getThoCommand();
int getThoCommand2();
int getRudderCommand();
int getElevatorCommand();

//debug
void printDesiredState();
void printCommands();
void printPIDoutput();

//APIs for PID parameter update
void setproll(uint16_t pr);
void setiroll(uint16_t ir);
void setdroll(uint16_t dr);
void setppitch(uint16_t pp);
void setipitch(uint16_t ip);
void setdpitch(uint16_t dp);
void setpyaw(uint16_t py);
void setiyaw(uint16_t iy);
void setdyaw(uint16_t dy);

//APIs for PID parameter access from flash
uint16_t pid_read(int idx);
void pid_write(uint16_t pid_array[], int size);

//APIs for Signature access from flash
void signiture_write(uint16_t signiture);
uint16_t signiture_read();
