#include "dout.h"

void dout_init( Dout* self, char* portpin ){
    gpio_config( self
        , (Gpio_init){ .portpin = portpin
                     , .mode    = GPIO_MODE_OUTPUT_PP
                     , .pull    = GPIO_NOPULL
                     } );
}
