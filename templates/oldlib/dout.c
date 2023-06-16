#include "dout.h"

#include <stdio.h>
#include <stdlib.h>

Dout* dout_init( char* portpin )
{
    Dout* self = malloc( sizeof( Dout ));
    if( !self ){ printf("dout malloc!\n"); return NULL; }

    gpio_config( self
        , (Gpio_init){ .portpin = portpin
                     , .mode    = GPIO_MODE_OUTPUT_PP
                     , .pull    = GPIO_NOPULL
                     } );
    return self;
}

void dout_set( Dout* self, int state ){
    HAL_GPIO_WritePin( self->port, self->pin, state );
}

void dout_flip( Dout* self ){
    HAL_GPIO_TogglePin( self->port, self->pin );
}

int dout_get( Dout* self ){
    return (int)HAL_GPIO_ReadPin( self->port, self->pin );
}
