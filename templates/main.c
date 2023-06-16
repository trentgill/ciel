#include <stm32f7xx.h>
#include <stm32f7xx_hal.h>
#include <stm32f7xx_hal_rcc.h>
#include <stm32f7xx_hal_cortex.h>

//#include "syscalls.c" // printf redirection
#include "../fglib/system.h" // HAL init & SysClkConfig
#include "../fglib/interrupts.h" // IRQ Priorities
#include "../fglib/dout.h" // dout

#include "lib/screen.h"
#include "lib/draw_interface.h"

// LD2 == PB7
// LD3 == PB14

// public defns
static Dout* LD2;


void failure(void){
    // dout_set(LD3, 1);
}

int main(void)
{
    system_init();

    LD2 = dout_init("B7");
    
    dout_set(LD2, 1);

    if(screen_setup())
        failure();

    // main loop
    uint32_t lasttick = 0; // throttle
    int tik = 1;
    new_frame();
    while(1){
        // 1ms throttled sub-loop
        if( lasttick != HAL_GetTick() ){
            lasttick = HAL_GetTick();
            if((tik--) <= 0){
                tik = 33;
                dout_flip(LD2);
                // new_frame();
                // dout_flip(LD3);
            }
        }
    }
    return 0;
}
