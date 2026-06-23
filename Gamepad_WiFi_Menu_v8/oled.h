#pragma once

void init_oled(void);

void setOledClear();

void setOledCursor(int32_t x, int32_t y);

void setOledPrint(String msg);
void setOledPrint(const char* msg);

void oledRefresh();

void fillOledRect(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h);

void showCurrentSSID(const char* ssid);

void showCurrentStage(const char* stage);

void showTextAtPos(int32_t x, int32_t y, const char* text);
