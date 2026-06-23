#pragma once

#include <stdint.h>

uint32_t pid_read(int idx);
void pid_write(uint32_t pid_array[], int size);
uint32_t pid_default_value(int idx);
