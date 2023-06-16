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
static Dout* LD2;

int main(void)
{
    system_init();

    LD2 = dout_init("B7");
    
    dout_set(LD2, 1);

    uint32_t lasttick = 0; // throttle
    while(1){
        // 1ms throttled sub-loop
        if( lasttick != HAL_GetTick() ){
            lasttick = HAL_GetTick();
            if( lasttick % 66 == 0 )
                dout_flip(LD2);
        }
    }
    return 0;
}
