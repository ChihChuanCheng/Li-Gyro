#pragma once

void init_oled(void);

void setOledClear();

void setOledCursor(int32_t x, int32_t y);

void setOledPrint(String msg);

void showCurrentSSID(String ssid);

void showCurrentStage(String stage);

void showTextAtPos(int32_t x, int32_t y, String text);
