#pragma once

#include <stm32f7xx.h> // HAL access

#include "gpio.h"

typedef struct sSpi{
    SPI_HandleTypeDef hspi;
    DMA_HandleTypeDef hdma_tx;
    // uint16_t frame[8];
    Gpio sck;
    Gpio mosi;
    Gpio miso;
    Gpio nss;
    // void (*tx_cplt)(struct sDAC108* self);
    // only valid during initialization
    // void* tmp_init; // (DAC108_init*)


    // add a GPIO_Debug portpin in *_init_t to act as a "error light"
        // this is generally useful for debugging, just add the "*.debugpp = B7" and the driver will use the led
} Spi;

typedef struct{
    int instance;
    char* sck;
    char* mosi;
    char* miso;
    char* nss;
    // void (*tx_cplt)(DAC108* self);
    // irq priority
} SpiConfig;

void spi_init( Spi* self, SpiConfig config);

void spi_write_hword( Spi* self, uint16_t val16 );
void spi_write_hwords( Spi* self, uint16_t* hwords, uint16_t count);

// HAL hooks

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi);

void DMA1_Stream4_IRQHandler(void);
void SPI2_IRQHandler(void);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
// void HAL_SPI_TxHalfCpltCallback(SPI_HandleTypeDef *hspi);
// void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi);
// void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi);
