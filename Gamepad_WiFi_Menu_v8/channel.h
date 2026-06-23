#pragma once

#include <Arduino.h>

static const uint8_t CHANNEL_COUNT = 8;
static const uint8_t MODEL_COUNT = 3;
static const uint8_t MODEL_NAME_COUNT = 6;
static const int16_t CHANNEL_MIN = 1000;
static const int16_t CHANNEL_CENTER = 1500;
static const int16_t CHANNEL_MAX = 2000;
static const int16_t CHANNEL_MIN_SUBTRIM = -250;
static const int16_t CHANNEL_MAX_SUBTRIM = 250;
static const uint8_t CHANNEL_MIN_ENDPOINT = 30;
static const uint8_t CHANNEL_MAX_ENDPOINT = 125;

typedef struct {
  bool reversed;
  uint8_t ratePercent;
  int16_t subtrim;
  uint8_t lowEndpointPercent;
  uint8_t highEndpointPercent;
} ChannelConfig;

void init_channel_output();
uint8_t get_active_model_index();
void set_active_model_index(uint8_t modelIndex);
const char* get_active_model_name();
const char* get_model_name(uint8_t nameIndex);
uint8_t get_model_name_index();
void set_model_name_index(uint8_t nameIndex);
bool get_channel_reversed(uint8_t channelIndex);
uint8_t get_channel_rate_percent(uint8_t channelIndex);
int16_t get_channel_subtrim(uint8_t channelIndex);
uint8_t get_channel_low_endpoint_percent(uint8_t channelIndex);
uint8_t get_channel_high_endpoint_percent(uint8_t channelIndex);
void set_channel_reversed(uint8_t channelIndex, bool reversed);
void set_channel_rate_percent(uint8_t channelIndex, uint8_t ratePercent);
void set_channel_subtrim(uint8_t channelIndex, int16_t subtrim);
void set_channel_low_endpoint_percent(uint8_t channelIndex, uint8_t endpointPercent);
void set_channel_high_endpoint_percent(uint8_t channelIndex, uint8_t endpointPercent);
uint8_t get_low_rate_percent();
uint8_t get_high_rate_percent();
uint8_t get_expo_percent();
bool get_low_rate_mode();
void set_low_rate_percent(uint8_t percent);
void set_high_rate_percent(uint8_t percent);
void set_expo_percent(uint8_t percent);
void set_low_rate_mode(bool enabled);
void write_channel_config_to_flash();
void build_channel_outputs(int16_t lx,
                           int16_t ly,
                           int16_t ry,
                           int16_t rx,
                           int16_t channels[CHANNEL_COUNT]);
void build_srv_message(const int16_t channels[CHANNEL_COUNT], char* buffer, size_t bufferSize);
void build_srv_message_from_sticks(int16_t lx,
                                   int16_t ly,
                                   int16_t ry,
                                   int16_t rx,
                                   char* buffer,
                                   size_t bufferSize);
