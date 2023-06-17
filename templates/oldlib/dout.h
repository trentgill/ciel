#pragma once

#include "gpio.h"

// Dout is just a type-alias to Gpio which auto-fills some default configuration
// This is preferable as it makes usage more deliberate and object-purpose tracking easier

// In future a DoutAtomic type should be added for setting multiple pins simultaneously

typedef Gpio Dout;

void dout_init( Dout* self, char* portpin );

// all hardware-level accesses are inlined to maximize speed
// branchless code preferred wherever possible

static inline uint32_t dout_get( Dout* self ){
    return !!(self->port->IDR & self->pin);
}

static inline void dout_write( Dout* self, uint32_t state ){
    // writing to the lower 16bits *sets*, while upper 16 *resets*
    self->port->BSRR = self->pin << (!state * 16);
}

static inline void dout_flip( Dout* self ){
    self->port->ODR ^= self->pin;
}

// set & reset are not offered as there is no speedup benefit
