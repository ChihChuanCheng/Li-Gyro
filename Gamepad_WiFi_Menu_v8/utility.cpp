#include "Options.h"
#include "utility.h"
#include <EEPROM.h>
#include <Wire.h>
#include <WiFiUdp.h>

const IPAddress serverIP(192,168,4,1);
unsigned int localUdpPort = 6188;

operation_mode_enum operation_mode;
WiFiUDP Udp;

void set_operation_mode(operation_mode_enum target_op_mode)
{
  operation_mode = target_op_mode;
}

operation_mode_enum get_operation_mode()
{
  return operation_mode;
}

void init_eeprom()
{
  EEPROM.begin(1024);
}

void init_i2c()
{
  Wire.begin(21, 22);
  Wire.setClock(400000);
}

void init_udp()
{
  Udp.begin(localUdpPort);
}

int read_eeprom(int i)
{
  return EEPROM.read(i);
}

void write_eeprom(int i, int ele)
{
  EEPROM.write(i, ele);
  EEPROM.commit();
}

void eeprom_write_block(int addr, const uint8_t* data, int len)
{
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addr + i, data[i]);
  }
  EEPROM.commit();
}

void eeprom_read_block(int addr, uint8_t* data, int len)
{
  for (int i = 0; i < len; i++)
  {
    data[i] = EEPROM.read(addr + i);
  }
}

void send_udp_msg(String msg)
{
  send_udp_msg(msg.c_str());
}

void send_udp_msg(const char* msg)
{
  Udp.beginPacket(serverIP, localUdpPort);
  Udp.write((const uint8_t*)msg, strlen(msg) + 1);
  Udp.endPacket();
}
