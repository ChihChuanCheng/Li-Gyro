#pragma once

#include <stdint.h>

uint16_t pid_read(int idx);
void pid_write(uint16_t pid_array[], int size);
