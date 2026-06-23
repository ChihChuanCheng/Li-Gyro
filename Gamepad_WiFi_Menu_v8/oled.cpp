#include "Options.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     4
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static bool displayDirty = false;
static bool oledAvailable = false;

void init_oled()
{
  display.setRotation(0);

  oledAvailable = display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  if (!oledAvailable)
  {
#ifdef __LOG__
    Serial.println(F("SSD1306 allocation failed"));
#endif
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void setOledClear()
{
  if (!oledAvailable) return;
  display.clearDisplay();
  displayDirty = true;
}

void setOledCursor(int32_t x, int32_t y)
{
  if (!oledAvailable) return;
  display.setCursor(x, y);
}

void setOledPrint(const char* msg)
{
  if (!oledAvailable) return;
  display.println(msg);
  displayDirty = true;
}

void setOledPrint(String msg)
{
  if (!oledAvailable) return;
  display.println(msg);
  displayDirty = true;
}

void oledRefresh()
{
  if (oledAvailable && displayDirty)
  {
    display.display();
    displayDirty = false;
  }
}

void fillOledRect(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h)
{
  if (!oledAvailable) return;
  display.fillRect(x1, y1, w, h, BLACK);
  displayDirty = true;
}

void showCurrentSSID(const char* ssid)
{
  char msg[64];
  snprintf(msg, sizeof(msg), "Connect to %s", ssid);
  setOledClear();
  setOledCursor(0,0);
  setOledPrint(msg);
}

void showCurrentStage(const char* stage)
{
  setOledClear();
  setOledCursor(0,0);
  setOledPrint(stage);
}

void showTextAtPos(int32_t x, int32_t y, const char* text)
{
  setOledCursor(x, y);
  setOledPrint(text);
}
