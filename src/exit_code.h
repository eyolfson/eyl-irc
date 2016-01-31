#pragma once

#include <stdbool.h>
#include <stdint.h>

uint8_t get_exit_code();
void set_exit_code(uint8_t value);

bool is_exiting();
