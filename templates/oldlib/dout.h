#pragma once

#include "gpio.h"

typedef Gpio Dout;

Dout* dout_init( char* portpin );
void dout_set( Dout* self, int state );
void dout_flip( Dout* self );
int dout_get( Dout* self );
