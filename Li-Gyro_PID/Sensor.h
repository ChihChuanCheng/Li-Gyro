#pragma once

/* basic gyro parameters */
struct rawdata {
  int16_t AcX;
  int16_t AcY;
  int16_t AcZ;
  int16_t Tmp;
  int16_t GyX;
  int16_t GyY;
  int16_t GyZ;
};

struct scaleddata {
  float AcX;
  float AcY;
  float AcZ;
  float Tmp;
  float GyX;
  float GyY;
  float GyZ;
};

struct devicedata {
  float RX;
  float RY;
  float RZ;
};

float getDT();
void loopRate(int freq);

void init_mpu();
void calibrateMPU6050(byte addr, rawdata &offsets,char up_axis, int num_samples, bool Debug);
rawdata averageSamples(rawdata * samps,int len);
bool checkI2c(byte addr);
void mpu6050Begin(byte addr);
rawdata mpu6050Read(byte addr, bool Debug);
void setMPU6050scales(byte addr,uint8_t Gyro,uint8_t Accl);
void getMPU6050scales(byte addr,uint8_t &Gyro,uint8_t &Accl);
scaleddata convertRawToScaled(byte addr, rawdata data_in, bool Debug);
void calculateAngle(scaleddata data_in, bool Debug);
void initialDeviceAngle();
void genMPU6050Sample();

float getDeviceAngleX();
float getDeviceAngleY();
float getDeviceAngleZ();
float getDeviceGyX();
float getDeviceGyY();
float getDeviceGyZ();

void printRollPitchYaw();
