#include "spi.h"

static void error_debug(Spi* self, char* msg);

static Spi* selves[6];
static SpiConfig* cfg;

// dumb helpers for finding the correct instance
static int instance_to_int(SPI_TypeDef* instance){
    if(instance == SPI1){ return 1; }
    else if(instance == SPI2){ return 2; }
    else if(instance == SPI3){ return 3; }
    else if(instance == SPI4){ return 4; }
    else if(instance == SPI5){ return 5; }
    else if(instance == SPI6){ return 6; }
    else return -1;
}

static SPI_TypeDef* int_to_instance(int i){
    switch(i){
        case 1: return SPI1;
        case 2: return SPI2;
        case 3: return SPI3;
        case 4: return SPI4;
        case 5: return SPI5;
        case 6: return SPI6;
        default: return NULL;
    }
}

void spi_init( Spi* self, SpiConfig config){
    SPI_TypeDef* instance = int_to_instance(config.instance);
    if(!instance){
        error_debug(self, "invalid SPI instance. must be 1-6");
        return;
    }
    self->hspi.Instance = instance;

    self->hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8; // /8 = 12.37MHz. supports up to 40MHz, so /4 would be good too.
    self->hspi.Init.Direction         = SPI_DIRECTION_2LINES;
    self->hspi.Init.CLKPhase          = SPI_PHASE_1EDGE;
    self->hspi.Init.CLKPolarity       = SPI_POLARITY_HIGH;
    self->hspi.Init.DataSize          = SPI_DATASIZE_16BIT;
    self->hspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    self->hspi.Init.TIMode            = SPI_TIMODE_DISABLE;
    self->hspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    self->hspi.Init.CRCPolynomial     = 7;
    self->hspi.Init.NSS               = SPI_NSS_HARD_OUTPUT; // if not working, SPI_NSS_SOFT
    self->hspi.Init.Mode              = SPI_MODE_MASTER;
    self->hspi.Init.NSSPMode          = SPI_NSS_PULSE_ENABLE;

    // save reference for interrupt lookup
    selves[config.instance -1] = self;

    // temporarily save config for initialization
    cfg = &config;

    if(HAL_SPI_Init(&self->hspi) != HAL_OK)
        error_debug(self, "HAL_SPI_Init failed");
}

static uint16_t d = 0;
// NOTE: val16 needs to be copied as it will likely be freed before the dma can use it
void spi_write_hword( Spi* self, uint16_t val16 ){
    d = val16;
    if(HAL_SPI_Transmit_DMA(&self->hspi, (uint8_t*)(&d), 2))
        error_debug(self, "transmit dma failed");
}

// NOTE: can't stress it enough. even a static array with no termination must be static
// the compiler will throw it away afterward if it knows it's never used again
// in the end we need a static DMA buffer anyway, and we just write data to that buffer
// but in terms of testing, just remember it for next time huh?
void spi_write_hwords( Spi* self, uint16_t* hwords, uint16_t count){
    if(HAL_SPI_Transmit_DMA(&self->hspi, (uint8_t*)hwords, 2*count))
        error_debug(self, "transmit dma failed");
}



// Configuration
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    Spi* self = selves[instance_to_int(hspi->Instance)-1];
    SpiConfig* config = cfg;

    // initialize GPIO pins
    Gpio_init ginit = { .mode  = GPIO_MODE_AF_PP
                      , .speed = GPIO_SPEED_FREQ_HIGH // GPIO_SPEED_HIGH??
                      , .pull  = GPIO_PULLDOWN};
    switch(config->instance){
        case 1: ginit.af = GPIO_AF5_SPI1; break;
        case 2: ginit.af = GPIO_AF5_SPI2; break; // could be 7
        case 3: ginit.af = GPIO_AF5_SPI3; break; // could be 6 or 7
        case 4: ginit.af = GPIO_AF5_SPI4; break;
        default: break;
    }
    ginit.portpin = config->sck;
    gpio_config(&self->sck, ginit);
    ginit.portpin = config->mosi;
    gpio_config(&self->mosi, ginit);
    ginit.portpin = config->miso;
    gpio_config(&self->miso, ginit);
    ginit.portpin = config->nss;
    gpio_config(&self->nss, ginit);

    // enable SPI clock
    switch(config->instance){
        case 1: __HAL_RCC_SPI1_CLK_ENABLE(); break;
        case 2: __HAL_RCC_SPI2_CLK_ENABLE(); break; // could be 7
        case 3: __HAL_RCC_SPI3_CLK_ENABLE(); break; // could be 6 or 7
        case 4: __HAL_RCC_SPI4_CLK_ENABLE(); break;
        default: break;
    }

    // enable DMA clock & config
    // TODO based on SPI selection? DMA arbiter?
    // __HAL_RCC_DMA2_CLK_ENABLE(); // SPI4
    __HAL_RCC_DMA1_CLK_ENABLE(); // SPI2

    // SPI2: D1.S4.C0 or D1.S6.C9
    // SPI4: D2.S1.C4
    self->hdma_tx.Instance                 = DMA1_Stream4;
    self->hdma_tx.Init.Channel             = DMA_CHANNEL_0;
    self->hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    self->hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    self->hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
    self->hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;
    self->hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    self->hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    self->hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    self->hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    self->hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    self->hdma_tx.Init.Mode                = DMA_NORMAL;
    self->hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;

    HAL_DMA_Init(&self->hdma_tx);

    /* Associate the initialized DMA handle to the the SPI handle */
    __HAL_LINKDMA(&self->hspi, hdmatx, self->hdma_tx);
    
    // HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, DAC_IRQPriority, 1);
    HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 3, 1); // IRQPriority should be configurable in init struct
    HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

    // Using Interrupt mode for simplicity
    // HAL_NVIC_SetPriority(SPI4_IRQn, DAC_IRQPriority, 0); // try increasing priority? 1?
    // HAL_NVIC_EnableIRQ(SPI4_IRQn);
}

void DMA1_Stream4_IRQHandler(void)
{
    // assume this is SPI2
    HAL_DMA_IRQHandler(selves[1]->hspi.hdmatx);
}

void SPI2_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&(selves[1]->hspi));
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    // if(_self && _self->tx_cplt)
    //     (_self->tx_cplt)(_self);
}

// unused.
// void HAL_SPI_TxHalfCpltCallback(SPI_HandleTypeDef *hspi){}
// void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi){}
// void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi){}



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
#include <stdio.h>
#include "dout.h"
static void error_debug(Spi* self, char* msg){
    // if(self.debug_pin)
    //     dout_write(self.debug_pin, 1);
    // if(self.debug_print)
    //     printf("%s\n\r", msg);
}
