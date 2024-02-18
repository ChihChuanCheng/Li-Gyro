#include "Options.h"
#include "utility.h"
#include <EEPROM.h>
#include <WiFiUdp.h>

/* Parameters for WiFi */
const IPAddress serverIP(192,168,4,1);
unsigned int localUdpPort = 6188;

operation_mode_enum operation_mode;
WiFiUDP Udp;

/* operation mode manupulation */
void set_operation_mode(operation_mode_enum target_op_mode)
{
  operation_mode = target_op_mode;
}

operation_mode_enum get_operation_mode()
{
  return operation_mode;
}

/* initialize EEPROM */
void init_eeprom()
{
  /* ready to use the first 120 elements */
  EEPROM.begin(400);
}

/* initilize UDP */
void init_udp()
{
  /* setup UDP
   * [NOTE] this initilization should be invoked after WiFi connection has been established
   */
  Udp.begin(localUdpPort);
}

/************************************
 *            EEPROM                *
 ************************************/
int read_eeprom(int i)
{
  Serial.print("EEPROM.read(): ");
  Serial.println(EEPROM.read(i));
  return EEPROM.read(i);
}

void write_eeprom(int i, int ele)
{
  Serial.print("EEPROM.write(): ");
  Serial.println(i, ele);
  EEPROM.write(i, ele);
  EEPROM.commit();
}

/***********************************
 *             UDP                 *
 ***********************************/
void send_udp_msg(String msg)
{
  /* transmit V7RC command via UDP */
  Udp.beginPacket(serverIP, localUdpPort); //準備傳送資料
  Udp.write((uint8_t*) msg.c_str(),msg.length()+1); //複製資料到傳送快取
  Udp.endPacket();            //傳送資料
}
