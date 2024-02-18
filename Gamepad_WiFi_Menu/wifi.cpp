#include "Options.h"
#include <WiFi.h>
#include "oled.h"
#include "utility.h"

/* WiFi settings */
const uint8_t size_prefix_str = 2;
String prefix[size_prefix_str];

int num_scaned_ssid = 0;

String ssid = "";
String pswd = "";
uint8_t wifi_trial = 0;
uint8_t wifi_trial_max = 10;

void ssid_pswd_write(String input_ssid, String input_pswd) {
#ifdef __LOG__
  Serial.println("Writing SSID");
#endif
  /* Write SSID to EEPROM 0~9 */
  for (int i = 0; i < input_ssid.length(); i++)
  {
    write_eeprom(i, input_ssid[i]);
#ifdef __LOG__
    Serial.print(char(read_eeprom(i)));
#endif
  }
#ifdef __LOG__
  Serial.println();

  Serial.println("Writing PSWD");
#endif
  /* Write PSWD to EEPROM 10~19 */
  for (int i = 0; i < input_pswd.length(); i++)
  {
    write_eeprom(10+i, input_pswd[i]);
#ifdef __LOG__
    Serial.print(char(read_eeprom(10+i)));
#endif
  }
#ifdef __LOG__
  Serial.println();
#endif
}

void prefix_init()
{
  prefix[0] = "Wright";
  prefix[1] = "Hover";
}

/* 
 *  This function is used to compare if temp_ssid matches the predefined ssid prefix
 */
uint8_t prefix_matching(String temp_ssid)
{
  /* i-th element of return_value indicates that i-th redefined prefix matches temp_ssid */
  bool return_value[size_prefix_str];

  /* initialize return_value */
  for (int i = 0; i < size_prefix_str; i++)
  { 
    return_value[i] = true;
  }

  /* if i-th redefined prefix doesn't match temp_ssid,
   * set the value of the corresponding return_value as false
   */
  for (int i = 0; i < size_prefix_str; i++)
  {
    for (int j = 0; j < prefix[i].length(); j++)
    {
       if (temp_ssid[j] != prefix[i][j])
       {
         return_value[i] = false;
       }
    }
  }

#ifdef __LOG__
  Serial.print("return_value[0]: ");
  Serial.println(return_value[0]);
  Serial.print("return_value[1]: ");
  Serial.println(return_value[1]);
  Serial.println("temp_ssid: " + temp_ssid);
  Serial.println("prefix[0]: " + prefix[0]);
  Serial.println("prefix[1]: " + prefix[1]);
#endif

  /* return the index of matched predefined prefix */
  for (int i = 0; i < size_prefix_str; i++)
  { 
    if (true == return_value[i])
    {
      return i;
    }
  }

  /* if not match any predefined prefix, return 0xFF  */
  return 0xFF;
}

void ssid_pswd_read()
{
  Serial.println("Reading SSID");
  /*Read SSID from EEPROM 0~9*/
  String temp_ssid = "";
  String temp_pswd = "";
  for (int i = 0; i < 10; ++i)
  {
    temp_ssid += char(read_eeprom(i));
  }

#ifdef __LOG__
  Serial.println("Reading PSWD");
#endif
  /* Read PSWD from EEPROM 10~19 */
  for (int i = 10; i < 20; ++i)
  {
    temp_pswd += char(read_eeprom(i));
  }

#ifdef __LOG__
  Serial.print("read_ssid: ");
  Serial.println(temp_ssid);

  Serial.print("read_pswd: ");
  Serial.println(temp_pswd);
#endif

  /* If temporary SSID matches the predefined prefix, apply it */
  if (0xFF != prefix_matching(temp_ssid))
  {
    ssid = temp_ssid.substring(0,prefix[prefix_matching(temp_ssid)].length()+3);
    pswd = temp_pswd.substring(0,prefix[prefix_matching(temp_ssid)].length()+3);
  }

#ifdef __LOG__
  Serial.print("ssid: ");
  Serial.println(ssid);

  Serial.print("pswd: ");
  Serial.println(pswd);
#endif
}

void scan_wifi()
{
  num_scaned_ssid = WiFi.scanNetworks();
  delay(1000);
  Serial.println("scan Wi-Fi done");
  if (num_scaned_ssid == 0)
    Serial.println("no Wi-Fi networks found");
  else
  {
    Serial.print(num_scaned_ssid);
    Serial.println(" Wi-Fi networks found");
  }
}

void inseart_designated_ssid_eeprom()
{
  /* insert scaned ssid that matches the predefined ssid into EEPROM */
  for (int i = 0; i < num_scaned_ssid; ++i)
  {
    if (0xFF != prefix_matching(WiFi.SSID(i)))
    {
      ssid_pswd_write(WiFi.SSID(i),WiFi.SSID(i));
    }
    delay(10);
  }
}

String get_scaned_ssid(int i)
{
  if (i < num_scaned_ssid)
  {
    return WiFi.SSID(i);
  }
  else
  {
    return String("NULL");
  }
}

int get_num_scaned_ssid()
{
  return num_scaned_ssid;
}

void reconnect_wifi()
{
  WiFi.disconnect();
  delay(500);

  /* read ssid and password from EEPROM */
  ssid_pswd_read();
  /* connect to WiFi using ssid and pswd */
  WiFi.begin(ssid.c_str(), pswd.c_str());

  wifi_trial = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    wifi_trial = wifi_trial+1;
    Serial.print(wifi_trial);
  }

#ifdef __OLED__
  showCurrentSSID(ssid);
#endif

#ifdef __LOG__
  Serial.println("Connected"); 
  Serial.print("IP Address:"); 
  Serial.println(WiFi.localIP());
#endif
}

/*
 * During WiFi initialization
 * (1) connect to SSID based on stored SSID in EEPROM
 * (2) connect to SSID from SSID list derived by WiFiScan with the predefined prefix
 */
void init_wifi()
{
  WiFi.mode(WIFI_STA);
  ssid_pswd_read();

  WiFi.begin(ssid.c_str(), pswd.c_str());

  wifi_trial = 0;

#ifdef __OLED__
  showCurrentStage(String("Connecting..."));
#endif

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
    wifi_trial = wifi_trial+1;
    Serial.print(wifi_trial);

    if (wifi_trial_max == wifi_trial) {
      /* if failed to connect to WiFi after 10 trials, perform WiFi scan */
      WiFi.disconnect();

      scan_wifi();
      inseart_designated_ssid_eeprom();

      ssid_pswd_read();
      WiFi.begin(ssid.c_str(), pswd.c_str());
      /* reset connecting */
      wifi_trial = 0;
    }
  }

#ifdef __OLED__
  showCurrentSSID(ssid);
#endif

  Serial.println("Connected"); 
  Serial.print("IP Address:"); 
  Serial.println(WiFi.localIP());
}
