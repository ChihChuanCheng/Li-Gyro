#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     4 // Reset pin 
#define SCREEN_ADDRESS 0x3C // Address; 0x3C for 128x64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void init_oled()
{
  Wire.begin();
  display.setRotation(0);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Clear the buffer

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void setOledClear()
{
  display.clearDisplay();
}

void setOledCursor(int32_t x, int32_t y)
{
  display.setCursor(x, y);
}

void setOledPrint(String msg)
{
  display.println(msg);
  display.display();
}

void fillOledRect(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h)
{
  display.fillRect(x1, y1, w, h, BLACK);
}

void showCurrentSSID(String ssid)
{
  String msg = "";

  setOledClear();
  setOledCursor(0,0);
  msg = String("Connect to ") + ssid;
  setOledPrint(msg);
}

void showCurrentStage(String stage)
{
  setOledClear();
  setOledCursor(0,0);
  setOledPrint(stage);
}

void showTextAtPos(int32_t x, int32_t y, String text)
{
  setOledCursor(x, y);
  setOledPrint(text);
}
