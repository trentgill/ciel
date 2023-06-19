#pragma once

#include <stm32f7xx.h>

#include "gpio.h"
#include "debug.h"

typedef struct sSai{
    SAI_HandleTypeDef hsai_a;
    SAI_HandleTypeDef hsai_b;
    DMA_HandleTypeDef hdma_tx_a;
    DMA_HandleTypeDef hdma_tx_b;
    Gpio sck;
    Gpio sd_a;
    Gpio sd_b;
    Gpio fs;
    Debug debug;
} Sai;

typedef struct{
    int instance;
    char* sck;
    char* sd_a;
    char* sd_b;
    char* fs;
    char* debug;
} SaiConfig;

void sai_init( Sai* self, SaiConfig config);

void sai_write_hwords( Sai* self, uint16_t* hwords, uint16_t count);



// Public declarations
// uint8_t CODEC_Start( block_process_fn_t processor );
// void CODEC_Stop( void );

// HAL level driver
void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai);
void DMA2_Stream5_IRQHandler(void);
// void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai);
// void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai);
