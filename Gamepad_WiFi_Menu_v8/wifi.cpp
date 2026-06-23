#include "Options.h"
#include <WiFi.h>
#include <string.h>
#include "oled.h"
#include "utility.h"
#include "wifi.h"

const uint8_t size_prefix_str = 3;
String prefix[size_prefix_str];

static const int WIFI_STORE_ADDR = 0;
static const uint8_t WIFI_STORE_MAGIC0 = 'W';
static const uint8_t WIFI_STORE_MAGIC1 = '4';
static const uint8_t WIFI_STORE_VERSION = 1;
static const uint8_t WIFI_MAX_TEXT = 31;
static const unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000;

int num_scaned_ssid = 0;

char ssid[32];
char pswd[32];

static bool wifi_connecting = false;
static bool wifi_status_reported = false;
static unsigned long wifi_connect_started_ms = 0;

static bool copy_if_prefixed(const char* source_ssid, const char* source_pswd)
{
  uint8_t prefix_idx = prefix_matching(source_ssid);
  if (0xFF == prefix_idx)
  {
    ssid[0] = '\0';
    pswd[0] = '\0';
    return false;
  }

  strncpy(ssid, source_ssid, sizeof(ssid) - 1);
  ssid[sizeof(ssid) - 1] = '\0';
  strncpy(pswd, source_pswd, sizeof(pswd) - 1);
  pswd[sizeof(pswd) - 1] = '\0';
  return true;
}

static bool read_wifi_store_v4()
{
  uint8_t header[4];
  eeprom_read_block(WIFI_STORE_ADDR, header, sizeof(header));

  if ((header[0] != WIFI_STORE_MAGIC0) ||
      (header[1] != WIFI_STORE_MAGIC1) ||
      (header[2] != WIFI_STORE_VERSION) ||
      (header[3] > WIFI_MAX_TEXT))
  {
    return false;
  }

  uint8_t pswd_len = read_eeprom(WIFI_STORE_ADDR + 4);
  if (pswd_len > WIFI_MAX_TEXT)
  {
    return false;
  }

  char stored_ssid[32] = {0};
  char stored_pswd[32] = {0};
  eeprom_read_block(WIFI_STORE_ADDR + 5, (uint8_t*)stored_ssid, header[3]);
  eeprom_read_block(WIFI_STORE_ADDR + 37, (uint8_t*)stored_pswd, pswd_len);
  stored_ssid[header[3]] = '\0';
  stored_pswd[pswd_len] = '\0';

  return copy_if_prefixed(stored_ssid, stored_pswd);
}

static bool read_wifi_store_legacy()
{
  char temp_ssid[11] = {0};
  char temp_pswd[11] = {0};

  for (int i = 0; i < 10; ++i)
  {
    temp_ssid[i] = char(read_eeprom(i));
    temp_pswd[i] = char(read_eeprom(10 + i));
  }

  if (copy_if_prefixed(temp_ssid, temp_pswd))
  {
    ssid_pswd_write(ssid, pswd);
    return true;
  }

  return false;
}

static void start_wifi_connection()
{
  wifi_connecting = false;
  wifi_status_reported = false;

  if (ssid[0] == '\0')
  {
#ifdef __OLED__
    showCurrentStage("No WiFi config");
#endif
    return;
  }

#ifdef __OLED__
  showCurrentStage("Connecting...");
#endif

  WiFi.begin(ssid, pswd);
  wifi_connect_started_ms = millis();
  wifi_connecting = true;
}

static void show_wifi_result(bool connected)
{
  if (connected)
  {
#ifdef __OLED__
    showCurrentSSID(ssid);
#endif
#ifdef __LOG__
    Serial.println("Connected");
    Serial.print("IP Address:");
    Serial.println(WiFi.localIP());
#endif
  }
  else
  {
#ifdef __OLED__
    showCurrentStage("No connection");
#endif
#ifdef __LOG__
    Serial.println("No connection");
#endif
  }
}

void ssid_pswd_write(const char* input_ssid, const char* input_pswd)
{
  char stored_ssid[32] = {0};
  char stored_pswd[32] = {0};
  uint8_t buffer[69] = {0};
  uint8_t ssid_len = min((int)strlen(input_ssid), (int)WIFI_MAX_TEXT);
  uint8_t pswd_len = min((int)strlen(input_pswd), (int)WIFI_MAX_TEXT);

  strncpy(stored_ssid, input_ssid, ssid_len);
  strncpy(stored_pswd, input_pswd, pswd_len);

  buffer[0] = WIFI_STORE_MAGIC0;
  buffer[1] = WIFI_STORE_MAGIC1;
  buffer[2] = WIFI_STORE_VERSION;
  buffer[3] = ssid_len;
  buffer[4] = pswd_len;
  memcpy(&buffer[5], stored_ssid, ssid_len);
  memcpy(&buffer[37], stored_pswd, pswd_len);

  eeprom_write_block(WIFI_STORE_ADDR, buffer, sizeof(buffer));
}

void prefix_init()
{
  prefix[0] = "Wright";
  prefix[1] = "Hover";
  prefix[2] = "Drone";
}

uint8_t prefix_matching(const char* temp_ssid)
{
  for (int i = 0; i < size_prefix_str; i++)
  {
    if (strncmp(temp_ssid, prefix[i].c_str(), prefix[i].length()) == 0)
    {
      return i;
    }
  }
  return 0xFF;
}

void ssid_pswd_read()
{
  if (!read_wifi_store_v4() && !read_wifi_store_legacy())
  {
    ssid[0] = '\0';
    pswd[0] = '\0';
  }

#ifdef __LOG__
  Serial.print("ssid: ");
  Serial.println(ssid);
#endif
}

void scan_wifi()
{
  num_scaned_ssid = WiFi.scanNetworks();
#ifdef __LOG__
  Serial.println("scan Wi-Fi done");
  Serial.print(num_scaned_ssid);
  Serial.println(" Wi-Fi networks found");
#endif
}

void inseart_designated_ssid_eeprom()
{
  for (int i = 0; i < num_scaned_ssid; ++i)
  {
    if (0xFF != prefix_matching(WiFi.SSID(i).c_str()))
    {
      ssid_pswd_write(WiFi.SSID(i).c_str(), WiFi.SSID(i).c_str());
    }
  }
}

String get_scaned_ssid(int i)
{
  if ((0 <= i) && (i < num_scaned_ssid))
  {
    return WiFi.SSID(i);
  }
  return String("NULL");
}

int get_num_scaned_ssid()
{
  return num_scaned_ssid;
}

const char* get_current_ssid()
{
  return ssid;
}

const char* get_current_password()
{
  return pswd;
}

bool is_wifi_connected()
{
  return WiFi.status() == WL_CONNECTED;
}

const char* get_connected_ssid()
{
  static char connectedSsid[32];

  if (!is_wifi_connected())
  {
    connectedSsid[0] = '\0';
    return connectedSsid;
  }

  WiFi.SSID().toCharArray(connectedSsid, sizeof(connectedSsid));
  return connectedSsid;
}

void reconnect_wifi()
{
  WiFi.disconnect();
  ssid_pswd_read();
  start_wifi_connection();
}

void init_wifi()
{
  WiFi.mode(WIFI_STA);
  ssid_pswd_read();
  start_wifi_connection();
}

void service_wifi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!wifi_status_reported)
    {
      show_wifi_result(true);
      wifi_status_reported = true;
    }
    wifi_connecting = false;
    return;
  }

  if (!wifi_connecting)
  {
    return;
  }

  if (millis() - wifi_connect_started_ms >= WIFI_CONNECT_TIMEOUT_MS)
  {
    WiFi.disconnect();
    wifi_connecting = false;
    wifi_status_reported = true;
    show_wifi_result(false);
  }
}
