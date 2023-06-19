#include <stm32f7xx.h>
#include <stm32f7xx_hal.h>
#include <stm32f7xx_hal_rcc.h>
#include <stm32f7xx_hal_cortex.h>

#include "syscalls.c" // printf redirection
#include "system.h" // HAL init & SysClkConfig
#include "oldlib/dout.h"
#include "oldlib/spi.h"
#include "oldlib/sai.h"

static Dout led2;
// static Dout led3;
// static Spi dac108;
static Sai dac108;

int main(void)
{
    system_init();

    // dout_init(&led2, "B7");
    // dout_init(&led3, "B14");

/*
    spi_init(&dac108, (SpiConfig){ .instance = 2
                                 , .sck  = "B13"
                                 , .mosi = "B15"
                                 , .miso = "B14"
                                 , .nss  = "B12"});

    static uint16_t hwords[2] = {0b1111000011001010, 0b0000111100110101};
    spi_write_hwords(&dac108, hwords, 2); // catch this on the scope
*/

    sai_init(&dac108, (SaiConfig){ .instance = 1
                                 , .sck  = "E5"
                                 , .sd_a = "E6"
                                 , .sd_b = "E3"
                                 , .fs   = "E4"
                                 , .debug = "B7"});

    // static uint16_t hwords[2] = {0b1111000011001010, 0b0000111100110101};
    static uint16_t hwords[2] = {0x1234,0xfedc};
    sai_write_hwords(&dac108, hwords, 2); // catch this on the scope
    
    // dout_write(&led2, 1);
    // dout_write(&led3, 1);

    // uint64_t lasttick = 0; // throttle
    volatile int d = 100;
    int s = 0;
    while(1){
        // if( (++lasttick) % 0xF0000 == 0 )
            // dout_flip(led2);
        // dout_write(&led3, s);
        // dout_write(&led2, dout_get(&led3));
        // dout_write(&led2, s);
        s ^= 1;

        while(d > 0){
            d--;
        }
        d = 0x400000;


    }
    return 0;
}
