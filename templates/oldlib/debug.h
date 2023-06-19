#pragma once

#include "gpio.h"

typedef Gpio Debug;

void debug_init( Debug* self, char* portpin );

void debug(Debug* self, char* msg);
