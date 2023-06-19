#include "debug.h"

void debug_init( Debug* self, char* portpin ){
    gpio_config( self
        , (Gpio_init){ .portpin = portpin
                     , .mode    = GPIO_MODE_OUTPUT_PP
                     , .pull    = GPIO_NOPULL
                     } );
}

/////////////////////////////////////////////////////////////////

// generic debugging function across library
// uses a debug led to indicate an error
// sends a printf message if .debug_print is enabled
// ***** think about this ***** >>
    // make a new class called Debug which is in every object, so Dout.debug, or Spi.debug
    // then we can have a truly generic & configurable debug concept
    // easy to switch what you're debugging by plugging in your fav debug mechanism into the init fn
    // so debugging follows the same declarative style
// error_debug could be a vararg function with customizable implementation
// it would also be static inline & use #define to disable in release mode
// #include <stdio.h>

#include "dout.h" // should really have dout_write be gpio_write with an alias
void debug(Debug* self, char* msg){
    dout_write(self, 1);
    // if(self.debug_print)
    //     printf("%s\n\r", msg);
}
