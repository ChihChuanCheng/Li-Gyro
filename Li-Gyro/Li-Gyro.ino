/*
 * Provided by 飛行造物.
 * This code provides the implementation of a flight controller for Wright Flyer with Gyro. 
 * It is implemented based on the reference of dRehmFlight (https://github.com/nickrehm/dRehmFlight)
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "Arduino.h"
#include "Motor.h"
#include "Sensor.h"
#include "PID.h"
#include "Options.h"

//Lowest voltage for protection (OBSOLATE)
#define LOW_POWER_PROTECT_VALUE 880 //880 corresponds to 3.7v

//Server connect to WiFi Network
#define AP_SSID "Wright002" //改成你的AP帳號
#define AP_PSW "Wright002" //改成你的密碼

//Parameter for WiFi connection
unsigned int localPort = 6188;
const char* ssid = AP_SSID;
const char* password = AP_PSW;

WiFiUDP Udp; // Creation of wifi Udp instance

//To read residue power of Lipo battery
const int analogInPin = A0;  // ESP8266 Analog Pin ADC0 = A0

//Variable to read control command from V7RC
char packetBuffer[255];
int datafromV7RC[8]={1500,1500,1500,1500,1500,1500,1500,1500};

//Variables for motor control
uint8_t rudder = 90;
uint8_t elevator = 90;
uint8_t throttle = 0;
uint8_t throttle2 = 0;
uint16_t residualPower = 0;  // value read from analog port (OBSOLATE)

//=======================================================================
//                    Power on setup
//=======================================================================
void setup() 
{
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  //Initialize motors
  init_motor();

  //Initialize MPU6050
  //SDA = Pin 2, SCL = Pin 14
  init_mpu();

  //Residual power detection (OBSOLATE)
  pinMode(analogInPin, INPUT); 

  //Establish softAP for WiFi connection
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Connect to the Wright Flyer and go to: http://");
  Serial.println(IP);

  //UDP Here ... 
  Udp.begin(localPort);
}
//=======================================================================
//                    Loop
//=======================================================================
/*
 *  This function is used to get radio command from V7RC
 */
bool getRadioCommand();

/* 
 *  Because map command cannot exactly bound converted output values between lowerbound and upperboudn,
 *  this function is used to bound output values to
 *       (1) lowerbound, when they are below the lowerbound;
 *       (2) upperbound, when they are above the upperbound.
 */
uint8_t commandMapping(int x, int lowerest_input, int highest_input, int lowerbound, int upperbound)
{
   return map(x, lowerest_input, highest_input, lowerbound, upperbound) < lowerbound?
          lowerbound:
          map(x, lowerest_input, highest_input, lowerbound, upperbound) > upperbound?
          upperbound:
          map(x, lowerest_input, highest_input, lowerbound, upperbound);
}


/*
 * For each loop, the control steps are
 * (1) Read MPU6050 status to get current roll, pitch, and yaw of flight controller;
 * (2) Read radio command from V7RC;
 * (3) Get desired control state based on radio command from V7RC (operated by player);
 * (4) Calculate PID control based on Gyro status and desired control state
 * (5) Adjust dc motors' commands based on desired state and PID control result
 * (6) Apply derived commands to dc motors
 * (7) Feed radio command of V7RC to PID controller
 */
void loop() 
{
  bool hasCommand = false;

#ifdef __PID_LOG__
  //printGyroData();
  //printRollPitchYaw();
  //printDesiredState();
  printCommands();
  printPIDoutput();
#endif /* __PID_LOG__ */

  //(1) Read MPU6050 status to get current roll, pitch, and yaw of flight controller;
  genMPU6050Sample();

  if (0 == WiFi.softAPgetStationNum())
  {
    /* When there is no client connected, turn off throttle and set rudder to 90 degree */
    throttle = 0;
    throttle2 = 0;
    rudder = 90;
    elevator = 90;
#ifdef __LOG__
    Serial.print("Waiting for clients...., current number of clients is ");
    Serial.println(WiFi.softAPgetStationNum());
#endif /* __LOG__ */
    motor_control(rudder, elevator, throttle, throttle2);
  }
  else
  {
    //(2) Read radio command from V7RC;
    hasCommand = getRadioCommand();

    //Submit command to motors only when radio command has been received from V7RC
    //Otherwise, wait for next loop
    if (true == hasCommand)
    {
      // revert rudder
      // This is specific for Wright Flyer to save an additional setting in V7RC app
      datafromV7RC[3] = (2000 - datafromV7RC[3]) + 1000;
      
      //(3) Get desired control state based on radio command from V7RC (operated by player);
      getDesState();

      //(4) Calculate PID control based on Gyro status and desired control state;
      controlANGLE();

      //(5)Adjust dc motors' commands based on desired state and PID control result
      controlMixer();
      scaleCommands();

      //(6) Apply derived commands to dc motors
      throttle = (uint8_t) getThoCommand();
      throttle2 = (uint8_t) getThoCommand2();
      rudder = (uint8_t) getRudderCommand();
      elevator = (uint8_t) getElevatorCommand();

      if (LOW_POWER_PROTECT_VALUE >= getResidualPower())
      {
        //Low power protection (OBSOLATE)
        motor_control(rudder, elevator, throttle, throttle2);
      }
      else
      {
        // For the sake of safety:
        // enable moter control only when player manipulate throttle or rudder exceeding certain ranges
        if ((datafromV7RC[1] > 1060) ||
            (datafromV7RC[3] > 1560) ||
            (datafromV7RC[3] < 1440)
           )
        {
          motor_control(rudder, elevator, throttle, throttle2);
        }
        else
        {
          motor_control(rudder, elevator, 0, 0);
        }
      }

      // (7) Feed radio command of V7RC to PID controller
      setRadioComm((unsigned long) datafromV7RC[1], (unsigned long) datafromV7RC[3], (unsigned long) datafromV7RC[2]);
    }
  }

  //To adjust update rate of MPU6050
  loopRate(LOOP_RATE_TIMES);
}

bool getRadioCommand()
{
  int packetSize = Udp.parsePacket();
  bool hasCommand = false;

  if (packetSize) {
    String rxData;
    String data;
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len-1] = 0;

#ifdef __LOG__
    Serial.println(packetBuffer);
    Serial.println(len);
#endif /* __LOG__ */

    /* reply received data to transmitter for debugging */
    Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
    Udp.printf("received: ");
    Udp.printf(packetBuffer);
    Udp.printf("\r\n");
    Udp.endPacket();

    /* convert received data to string */
    if (len > 0) {
      for (int i = 0; i < len; i++){
        rxData+=packetBuffer[i];
      }
    }

    /* parse received string to V7RC commands */
    if(packetBuffer[1]=='R'){
      for(int i=0;i<4;i++){
        data = rxData.substring(i*4+3, i*4+7); 
        datafromV7RC[i] = data.toInt();
      }
    }

#ifdef __LOG__
    Serial.print(packetBuffer[2]);  /// should be V / T / 8 (2 ch, 4 ch , 8 ch )
    Serial.print(",");
    for(int i=0;i<8;i++){
      Serial.print(datafromV7RC[i]);
      Serial.print(",");
    }
    Serial.println(",");
    Serial.println();
#endif /* __LOG__ */

    hasCommand = true;
  }

  return hasCommand;
}

/*
 * R1 = 1k, R2 = 3.3k
 * Divided power to A0 = VCC*(R1/(R1+R2))
 * Vcc                      4.2         4.1         4           3.9         3.8         3.7         3.6         3.5
 * Divided Voltage          0.976744186 0.953488372 0.930232558 0.906976744 0.88372093  0.860465116 0.837209302 0.813953488
 * Returned Sensor Value    1000.186047 976.372093  952.5581395 928.744186  904.9302326 881.1162791 857.3023256 833.4883721
 */
uint16_t getResidualPower()
{
  uint16_t sensorValue = 0;
  sensorValue = analogRead(analogInPin);

#ifdef __LOG__
  // print the readings in the Serial Monitor
  Serial.print("sensor = ");
  Serial.println(sensorValue);
#endif /* __LOG__ */

  return sensorValue;
}
//=======================================================================
