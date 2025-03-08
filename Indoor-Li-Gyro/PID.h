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
