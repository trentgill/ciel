#include <stm32f7xx.h>
#include <stm32f7xx_hal.h>
#include <stm32f7xx_hal_rcc.h>
#include <stm32f7xx_hal_cortex.h>

#include "syscalls.c" // printf redirection
#include "system.h" // HAL init & SysClkConfig
#include "oldlib/dout.h" // dout

// LD2 == PB7
// LD3 == PB14

// public defns
static Dout LD2;
static Dout LD3;

int main(void)
{
    system_init();

    dout_init(&LD2, "B7");
    dout_init(&LD3, "B14");
    
    dout_write(&LD2, 1);
    dout_write(&LD3, 1);

    // uint64_t lasttick = 0; // throttle
    volatile int d = 100;
    int s = 0;
    while(1){
        // if( (++lasttick) % 0xF0000 == 0 )
            // dout_flip(LD2);
        dout_write(&LD3, s);
        dout_write(&LD2, dout_get(&LD3));
        s ^= 1;

        while(d > 0){
            d--;
        }
        d = 1;


    }
    return 0;
}
