#include "channel.h"
#include "utility.h"

static ChannelConfig channelConfigs[CHANNEL_COUNT];
static uint8_t activeModelIndex = 0;
static uint8_t lowRatePercent = 70;
static uint8_t highRatePercent = 100;
static uint8_t expoPercent = 30;
static uint8_t modelNameIndex = 0;
static bool lowRateMode = false;
static const char* MODEL_NAMES[MODEL_NAME_COUNT] = {
  "TRAINER",
  "SPORT",
  "GLIDER",
  "DELTA",
  "BOAT",
  "CUSTOM"
};

static const int CHANNEL_LEGACY_STORE_ADDR = 240;
static const int MODEL_STORE_ADDR = 300;
static const int CHANNEL_PROFILE_STORE_ADDR = 320;
static const int CHANNEL_PROFILE_STRIDE = 64;
static const int CONTROL_PROFILE_STORE_ADDR = 520;
static const int CONTROL_PROFILE_STRIDE = 8;
static const uint8_t MODEL_STORE_MAGIC0 = 'M';
static const uint8_t MODEL_STORE_MAGIC1 = '3';
static const uint8_t CHANNEL_STORE_MAGIC0 = 'H';
static const uint8_t CHANNEL_STORE_MAGIC1 = '8';
static const uint8_t CHANNEL_STORE_VERSION = 1;
static const uint8_t CHANNEL_CONFIG_SIZE = 6;
static const uint8_t CONTROL_STORE_MAGIC0 = 'D';
static const uint8_t CONTROL_STORE_MAGIC1 = 'R';
static const uint8_t CONTROL_STORE_VERSION = 1;

static uint8_t clamp_percent(uint8_t percent)
{
  if (percent < CHANNEL_MIN_ENDPOINT)
  {
    return CHANNEL_MIN_ENDPOINT;
  }
  if (percent > CHANNEL_MAX_ENDPOINT)
  {
    return CHANNEL_MAX_ENDPOINT;
  }
  return percent;
}

static uint8_t clamp_control_percent(uint8_t percent, uint8_t minPercent, uint8_t maxPercent)
{
  if (percent < minPercent)
  {
    return minPercent;
  }
  if (percent > maxPercent)
  {
    return maxPercent;
  }
  return percent;
}

static int channel_store_addr()
{
  return CHANNEL_PROFILE_STORE_ADDR + activeModelIndex * CHANNEL_PROFILE_STRIDE;
}

static int control_store_addr()
{
  return CONTROL_PROFILE_STORE_ADDR + activeModelIndex * CONTROL_PROFILE_STRIDE;
}

static void read_active_model_from_flash()
{
  if ((read_eeprom(MODEL_STORE_ADDR) == MODEL_STORE_MAGIC0) &&
      (read_eeprom(MODEL_STORE_ADDR + 1) == MODEL_STORE_MAGIC1) &&
      (read_eeprom(MODEL_STORE_ADDR + 2) < MODEL_COUNT))
  {
    activeModelIndex = read_eeprom(MODEL_STORE_ADDR + 2);
  }
}

static void write_active_model_to_flash()
{
  uint8_t buffer[3] = {MODEL_STORE_MAGIC0, MODEL_STORE_MAGIC1, activeModelIndex};
  eeprom_write_block(MODEL_STORE_ADDR, buffer, sizeof(buffer));
}

static int16_t clamp_subtrim(int16_t subtrim)
{
  if (subtrim < CHANNEL_MIN_SUBTRIM)
  {
    return CHANNEL_MIN_SUBTRIM;
  }
  if (subtrim > CHANNEL_MAX_SUBTRIM)
  {
    return CHANNEL_MAX_SUBTRIM;
  }
  return subtrim;
}

static int16_t clamp_channel_value(int32_t value)
{
  if (value < CHANNEL_MIN)
  {
    return CHANNEL_MIN;
  }
  if (value > CHANNEL_MAX)
  {
    return CHANNEL_MAX;
  }
  return (int16_t)value;
}

static int16_t apply_channel_config(int16_t input, const ChannelConfig& config)
{
  int32_t centered = (int32_t)input - CHANNEL_CENTER;

  if (config.reversed)
  {
    centered = -centered;
  }

  centered = centered * config.ratePercent / 100;

  if (centered < 0)
  {
    centered = centered * config.lowEndpointPercent / 100;
  }
  else
  {
    centered = centered * config.highEndpointPercent / 100;
  }

  return clamp_channel_value(CHANNEL_CENTER + centered + config.subtrim);
}

static void set_default_channel_configs()
{
  for (uint8_t i = 0; i < CHANNEL_COUNT; i++)
  {
    channelConfigs[i].reversed = false;
    channelConfigs[i].ratePercent = 100;
    channelConfigs[i].subtrim = 0;
    channelConfigs[i].lowEndpointPercent = 100;
    channelConfigs[i].highEndpointPercent = 100;
  }
}

static bool channel_store_valid()
{
  int addr = channel_store_addr();
  return (read_eeprom(addr) == CHANNEL_STORE_MAGIC0) &&
         (read_eeprom(addr + 1) == CHANNEL_STORE_MAGIC1) &&
         (read_eeprom(addr + 2) == CHANNEL_STORE_VERSION) &&
         (read_eeprom(addr + 3) == CHANNEL_COUNT);
}

static bool legacy_channel_store_valid()
{
  return (read_eeprom(CHANNEL_LEGACY_STORE_ADDR) == CHANNEL_STORE_MAGIC0) &&
         (read_eeprom(CHANNEL_LEGACY_STORE_ADDR + 1) == CHANNEL_STORE_MAGIC1) &&
         (read_eeprom(CHANNEL_LEGACY_STORE_ADDR + 2) == CHANNEL_STORE_VERSION) &&
         (read_eeprom(CHANNEL_LEGACY_STORE_ADDR + 3) == CHANNEL_COUNT);
}

static void read_channel_config_from_flash()
{
  int baseAddr = channel_store_addr();

  if (!channel_store_valid())
  {
    if (!legacy_channel_store_valid())
    {
      return;
    }
    baseAddr = CHANNEL_LEGACY_STORE_ADDR;
  }

  for (uint8_t i = 0; i < CHANNEL_COUNT; i++)
  {
    uint8_t data[CHANNEL_CONFIG_SIZE];
    int addr = baseAddr + 4 + i * CHANNEL_CONFIG_SIZE;
    eeprom_read_block(addr, data, sizeof(data));

    channelConfigs[i].reversed = data[0] != 0;
    channelConfigs[i].ratePercent = clamp_percent(data[1]);
    channelConfigs[i].subtrim = clamp_subtrim((int16_t)(((uint16_t)data[2] << 8) | data[3]));
    channelConfigs[i].lowEndpointPercent = clamp_percent(data[4]);
    channelConfigs[i].highEndpointPercent = clamp_percent(data[5]);
  }
}

static void set_default_control_profile()
{
  lowRatePercent = 70;
  highRatePercent = 100;
  expoPercent = 30;
  modelNameIndex = activeModelIndex;
  lowRateMode = false;
}

static void read_control_profile_from_flash()
{
  int addr = control_store_addr();
  if ((read_eeprom(addr) != CONTROL_STORE_MAGIC0) ||
      (read_eeprom(addr + 1) != CONTROL_STORE_MAGIC1) ||
      (read_eeprom(addr + 2) != CONTROL_STORE_VERSION))
  {
    return;
  }

  lowRatePercent = clamp_control_percent(read_eeprom(addr + 3), 30, 100);
  highRatePercent = clamp_control_percent(read_eeprom(addr + 4), 30, 125);
  expoPercent = clamp_control_percent(read_eeprom(addr + 5), 0, 80);
  modelNameIndex = clamp_control_percent(read_eeprom(addr + 6), 0, MODEL_NAME_COUNT - 1);
}

static void write_control_profile_to_flash()
{
  uint8_t buffer[7] = {
    CONTROL_STORE_MAGIC0,
    CONTROL_STORE_MAGIC1,
    CONTROL_STORE_VERSION,
    lowRatePercent,
    highRatePercent,
    expoPercent,
    modelNameIndex
  };
  eeprom_write_block(control_store_addr(), buffer, sizeof(buffer));
}

void init_channel_output()
{
  read_active_model_from_flash();
  set_default_channel_configs();
  set_default_control_profile();
  read_channel_config_from_flash();
  read_control_profile_from_flash();
}

uint8_t get_active_model_index()
{
  return activeModelIndex;
}

void set_active_model_index(uint8_t modelIndex)
{
  if (modelIndex >= MODEL_COUNT)
  {
    return;
  }

  activeModelIndex = modelIndex;
  write_active_model_to_flash();
  set_default_channel_configs();
  set_default_control_profile();
  read_channel_config_from_flash();
  read_control_profile_from_flash();
}

const char* get_active_model_name()
{
  return MODEL_NAMES[modelNameIndex];
}

const char* get_model_name(uint8_t nameIndex)
{
  if (nameIndex >= MODEL_NAME_COUNT)
  {
    return MODEL_NAMES[0];
  }
  return MODEL_NAMES[nameIndex];
}

uint8_t get_model_name_index()
{
  return modelNameIndex;
}

void set_model_name_index(uint8_t nameIndex)
{
  if (nameIndex >= MODEL_NAME_COUNT)
  {
    return;
  }
  modelNameIndex = nameIndex;
}

bool get_channel_reversed(uint8_t channelIndex)
{
  if (channelIndex >= CHANNEL_COUNT)
  {
    return false;
  }
  return channelConfigs[channelIndex].reversed;
}

uint8_t get_channel_rate_percent(uint8_t channelIndex)
{
  if (channelIndex >= CHANNEL_COUNT)
  {
    return 100;
  }
  return channelConfigs[channelIndex].ratePercent;
}

int16_t get_channel_subtrim(uint8_t channelIndex)
{
  if (channelIndex >= CHANNEL_COUNT)
  {
    return 0;
  }
  return channelConfigs[channelIndex].subtrim;
}

uint8_t get_channel_low_endpoint_percent(uint8_t channelIndex)
{
  if (channelIndex >= CHANNEL_COUNT)
  {
    return 100;
  }
  return channelConfigs[channelIndex].lowEndpointPercent;
}

uint8_t get_channel_high_endpoint_percent(uint8_t channelIndex)
{
  if (channelIndex >= CHANNEL_COUNT)
  {
    return 100;
  }
  return channelConfigs[channelIndex].highEndpointPercent;
}

void set_channel_reversed(uint8_t channelIndex, bool reversed)
{
  if (channelIndex >= CHANNEL_COUNT)
  {
    return;
  }
  channelConfigs[channelIndex].reversed = reversed;
}

void set_channel_rate_percent(uint8_t channelIndex, uint8_t ratePercent)
{
  if (channelIndex >= CHANNEL_COUNT)
  {
    return;
  }
  channelConfigs[channelIndex].ratePercent = clamp_percent(ratePercent);
}

void set_channel_subtrim(uint8_t channelIndex, int16_t subtrim)
{
  if (channelIndex >= CHANNEL_COUNT)
  {
    return;
  }
  channelConfigs[channelIndex].subtrim = clamp_subtrim(subtrim);
}

void set_channel_low_endpoint_percent(uint8_t channelIndex, uint8_t endpointPercent)
{
  if (channelIndex >= CHANNEL_COUNT)
  {
    return;
  }
  channelConfigs[channelIndex].lowEndpointPercent = clamp_percent(endpointPercent);
}

void set_channel_high_endpoint_percent(uint8_t channelIndex, uint8_t endpointPercent)
{
  if (channelIndex >= CHANNEL_COUNT)
  {
    return;
  }
  channelConfigs[channelIndex].highEndpointPercent = clamp_percent(endpointPercent);
}

uint8_t get_low_rate_percent()
{
  return lowRatePercent;
}

uint8_t get_high_rate_percent()
{
  return highRatePercent;
}

uint8_t get_expo_percent()
{
  return expoPercent;
}

bool get_low_rate_mode()
{
  return lowRateMode;
}

void set_low_rate_percent(uint8_t percent)
{
  lowRatePercent = clamp_control_percent(percent, 30, 100);
}

void set_high_rate_percent(uint8_t percent)
{
  highRatePercent = clamp_control_percent(percent, 30, 125);
}

void set_expo_percent(uint8_t percent)
{
  expoPercent = clamp_control_percent(percent, 0, 80);
}

void set_low_rate_mode(bool enabled)
{
  lowRateMode = enabled;
}

void write_channel_config_to_flash()
{
  uint8_t buffer[4 + CHANNEL_COUNT * CHANNEL_CONFIG_SIZE] = {0};

  buffer[0] = CHANNEL_STORE_MAGIC0;
  buffer[1] = CHANNEL_STORE_MAGIC1;
  buffer[2] = CHANNEL_STORE_VERSION;
  buffer[3] = CHANNEL_COUNT;

  for (uint8_t i = 0; i < CHANNEL_COUNT; i++)
  {
    int offset = 4 + i * CHANNEL_CONFIG_SIZE;
    buffer[offset] = channelConfigs[i].reversed ? 1 : 0;
    buffer[offset + 1] = clamp_percent(channelConfigs[i].ratePercent);
    buffer[offset + 2] = (channelConfigs[i].subtrim >> 8) & 0xFF;
    buffer[offset + 3] = channelConfigs[i].subtrim & 0xFF;
    buffer[offset + 4] = clamp_percent(channelConfigs[i].lowEndpointPercent);
    buffer[offset + 5] = clamp_percent(channelConfigs[i].highEndpointPercent);
  }

  eeprom_write_block(channel_store_addr(), buffer, sizeof(buffer));
  write_control_profile_to_flash();
}

static int16_t apply_dual_rate_expo(int16_t input)
{
  int32_t centered = (int32_t)input - CHANNEL_CENTER;
  int32_t sign = (centered < 0) ? -1 : 1;
  int32_t absValue = abs(centered);
  uint8_t rate = lowRateMode ? lowRatePercent : highRatePercent;

  absValue = absValue * rate / 100;

  if (expoPercent > 0)
  {
    int32_t cubic = absValue * absValue / 500;
    cubic = cubic * absValue / 500;
    absValue = ((100 - expoPercent) * absValue + expoPercent * cubic) / 100;
  }

  return clamp_channel_value(CHANNEL_CENTER + sign * absValue);
}

void build_channel_outputs(int16_t lx,
                           int16_t ly,
                           int16_t ry,
                           int16_t rx,
                           int16_t channels[CHANNEL_COUNT])
{
  int16_t rawChannels[CHANNEL_COUNT] = {
    apply_dual_rate_expo(lx),
    ly,
    apply_dual_rate_expo(ry),
    apply_dual_rate_expo(rx),
    CHANNEL_CENTER,
    CHANNEL_CENTER,
    CHANNEL_CENTER,
    CHANNEL_CENTER
  };

  for (uint8_t i = 0; i < CHANNEL_COUNT; i++)
  {
    channels[i] = apply_channel_config(rawChannels[i], channelConfigs[i]);
  }
}

void build_srv_message(const int16_t channels[CHANNEL_COUNT], char* buffer, size_t bufferSize)
{
  if (buffer == NULL || bufferSize == 0)
  {
    return;
  }

  snprintf(buffer,
           bufferSize,
           "SRV%04d%04d%04d%04d%04d%04d%04d%04d",
           channels[0],
           channels[1],
           channels[2],
           channels[3],
           channels[4],
           channels[5],
           channels[6],
           channels[7]);
}

void build_srv_message_from_sticks(int16_t lx,
                                   int16_t ly,
                                   int16_t ry,
                                   int16_t rx,
                                   char* buffer,
                                   size_t bufferSize)
{
  int16_t channels[CHANNEL_COUNT];
  build_channel_outputs(lx, ly, ry, rx, channels);
  build_srv_message(channels, buffer, bufferSize);
}
