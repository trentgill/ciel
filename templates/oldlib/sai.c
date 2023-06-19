#include "sai.h"

static Sai* selves[2];
static SaiConfig* cfg;





// audio buffers for codec DMA
// int       samp_rate;
// int       b_size;
// int       buffer_size;
// uint32_t* inBuff;
// uint32_t* outBuff;
// block_process_fn_t block_process_fn = NULL;

////////////////////////////////////
// private declarations

static void codec_to_floats( float* f0, float* f1, __IO uint32_t* codec, int b_size);
static void floats_to_codec(__IO uint32_t* codec, float* f, int b_size);


//////////////////////////////////////
// public fns

// dumb helpers for finding the correct instance
// static int instance_to_int(SAI_TypeDef* instance){
//     if(instance == SPI1){ return 1; }
//     else if(instance == SPI2){ return 2; }
//     else if(instance == SPI3){ return 3; }
//     else if(instance == SPI4){ return 4; }
//     else if(instance == SPI5){ return 5; }
//     else if(instance == SPI6){ return 6; }
//     else return -1;
// }

// static SPI_TypeDef* int_to_instance(int i){
//     switch(i){
//         case 1: return SPI1;
//         case 2: return SPI2;
//         case 3: return SPI3;
//         case 4: return SPI4;
//         case 5: return SPI5;
//         case 6: return SPI6;
//         default: return NULL;
//     }
// }

void sai_init( Sai* self, SaiConfig config){
    // save references for future IRQ lookup
    // TODO check the config struct has the required fields first
    selves[config.instance-1] = self;
    cfg = &config;

    if(config.debug)
        debug_init(&self->debug, config.debug);

    // uint8_t error = 0;

    // debug(&self->debug, "oops");

    // samp_rate   = SampleRate;
    // b_size      = BlockSize;
    // buffer_size = b_size * 2 * 2; // flip-flop DMA, 2 channels

    // // Allocate codec buffers
    // // uint32_t contains signed-24b, left-justified sample
    // inBuff = malloc( sizeof(uint32_t) * buffer_size * 2 );
    // if( !inBuff ){ error = 3; }
    // for( int i=0; i<(buffer_size * 2); i++ ){ inBuff[i] = 0;}
    // outBuff = &(inBuff[buffer_size]);


    RCC_PeriphCLKInitTypeDef rcc;
    rcc.PeriphClockSelection    = RCC_PERIPHCLK_SAI1; // TODO follow instance
    rcc.Sai1ClockSelection      = RCC_SAI1CLKSOURCE_PLLSAI;
    // Configure PLLSAI prescalers
    // Just use 1MHz as the starting point (i think xtal / 8)
    // PLLSAIN is the PLL multiplier [50 ~ 432]
    // PLLSAIQ is the division from N [2 ~ 15]
    // Finally the PLLSAIDivQ is a further division [1 ~ 32]
        // PLLSAI_VCO: VCO_429M
        // SAI_CLK(first level) = PLLSAI_VCO/PLLSAIQ = 429/2 = 214.5 Mhz
        // SAI_CLK_x = SAI_CLK(first level)/PLLSAIDIVQ = 214.5/19 = 11.289 Mhz
    rcc.PLLSAI.PLLSAIN          = 187;
    rcc.PLLSAI.PLLSAIQ          = 15    ;
        // 12.46667 (goal was 12.48MHz for 24kHz samplerate worstcase)

    // rcc.PLLSAI.PLLSAIN          = 344; // 429  344
    // rcc.PLLSAI.PLLSAIQ          = 7;   // 2    7
    rcc.PLLSAIDivQ              = 1;   // 19   2
    HAL_RCCEx_PeriphCLKConfig(&rcc);

    // Initialize SAI
    __HAL_SAI_RESET_HANDLE_STATE(&self->hsai_a);
    __HAL_SAI_RESET_HANDLE_STATE(&self->hsai_b);

    // block A
    self->hsai_a.Instance = SAI1_Block_A; // TODO follow instance
    __HAL_SAI_DISABLE(&self->hsai_a);
    self->hsai_a.Init.AudioMode         = SAI_MODEMASTER_TX;
    self->hsai_a.Init.Synchro           = SAI_ASYNCHRONOUS;
    self->hsai_a.Init.SynchroExt        = SAI_SYNCEXT_OUTBLOCKA_ENABLE;
    self->hsai_a.Init.OutputDrive       = SAI_OUTPUTDRIVE_ENABLE;
    self->hsai_a.Init.NoDivider         = SAI_MASTERDIVIDER_DISABLE;
    self->hsai_a.Init.FIFOThreshold     = SAI_FIFOTHRESHOLD_1QF;
    self->hsai_a.Init.AudioFrequency    = SAI_AUDIO_FREQUENCY_48K; // _48K or _96K or _192K
    self->hsai_a.Init.Protocol          = SAI_FREE_PROTOCOL;
    self->hsai_a.Init.DataSize          = SAI_DATASIZE_16;
    self->hsai_a.Init.FirstBit          = SAI_FIRSTBIT_MSB;
    self->hsai_a.Init.ClockStrobing     = SAI_CLOCKSTROBING_RISINGEDGE; // CONFIRM

    self->hsai_a.FrameInit.FrameLength          = 33;
    self->hsai_a.FrameInit.ActiveFrameLength    = 1; // 1 ?
    self->hsai_a.FrameInit.FSDefinition         = SAI_FS_STARTFRAME;
    self->hsai_a.FrameInit.FSPolarity           = SAI_FS_ACTIVE_HIGH;
    self->hsai_a.FrameInit.FSOffset             = SAI_FS_BEFOREFIRSTBIT;

    self->hsai_a.SlotInit.FirstBitOffset    = 0;
    self->hsai_a.SlotInit.SlotSize          = SAI_SLOTSIZE_16B;
    self->hsai_a.SlotInit.SlotNumber        = 2;
    self->hsai_a.SlotInit.SlotActive        = SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1;

    if(HAL_SAI_Init(&self->hsai_a)){
        debug(&self->debug, "SAI_init failed");
        return;
    }

    // ADC
    self->hsai_b.Instance = SAI1_Block_B;
    __HAL_SAI_DISABLE(&self->hsai_b);
    self->hsai_b.Init.AudioMode         = SAI_MODESLAVE_TX;
    self->hsai_b.Init.Synchro           = SAI_SYNCHRONOUS; // lock to other block
    self->hsai_b.Init.SynchroExt        = SAI_SYNCEXT_OUTBLOCKA_ENABLE;
    self->hsai_b.Init.OutputDrive       = SAI_OUTPUTDRIVE_ENABLE;
    self->hsai_b.Init.NoDivider         = SAI_MASTERDIVIDER_DISABLE;
    self->hsai_b.Init.FIFOThreshold     = SAI_FIFOTHRESHOLD_1QF;
    self->hsai_b.Init.AudioFrequency    = SAI_AUDIO_FREQUENCY_48K;
    self->hsai_b.Init.Protocol          = SAI_FREE_PROTOCOL;
    self->hsai_b.Init.DataSize          = SAI_DATASIZE_16;
    self->hsai_b.Init.FirstBit          = SAI_FIRSTBIT_MSB;
    self->hsai_b.Init.ClockStrobing     = SAI_CLOCKSTROBING_RISINGEDGE; // unsure?

    self->hsai_b.FrameInit.FrameLength          = 33;
    self->hsai_b.FrameInit.ActiveFrameLength    = 1;
    self->hsai_b.FrameInit.FSDefinition         = SAI_FS_STARTFRAME;
    self->hsai_b.FrameInit.FSPolarity           = SAI_FS_ACTIVE_HIGH;
    self->hsai_b.FrameInit.FSOffset             = SAI_FS_BEFOREFIRSTBIT; // SAI_FS_FIRSTBIT, SAI_FS_BEFOREFIRSTBIT

    self->hsai_b.SlotInit.FirstBitOffset    = 0;
    self->hsai_b.SlotInit.SlotSize          = SAI_SLOTSIZE_16B;
    self->hsai_b.SlotInit.SlotNumber        = 2;
    self->hsai_b.SlotInit.SlotActive        = SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1;

    // if( HAL_OK != HAL_SAI_Init(&self->hsai_b)){
    //     debug(&self->debug, "SAI_init failed");
    //     return;
    // }

    // Enable SAI to generate clock used by audio driver
    // __HAL_SAI_ENABLE(&self->hsai_b); // adc before dac
    __HAL_SAI_ENABLE(&self->hsai_a);
}

void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai)
{
    GPIO_InitTypeDef  GPIO_Init;
    GPIO_Init.Mode  = GPIO_MODE_AF_PP;
    GPIO_Init.Pull  = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    __HAL_RCC_DMA2_CLK_ENABLE();

    if(hsai == &selves[0]->hsai_a){
        Sai* self = selves[0];
        __HAL_RCC_SAI1_CLK_ENABLE(); // RCC
        __HAL_RCC_GPIOE_CLK_ENABLE();
        GPIO_Init.Alternate     = GPIO_AF6_SAI1;
        GPIO_Init.Pin           = GPIO_PIN_4;
        HAL_GPIO_Init(GPIOE, &GPIO_Init);
        GPIO_Init.Alternate     = GPIO_AF6_SAI1;
        GPIO_Init.Pin           = GPIO_PIN_5;
        HAL_GPIO_Init(GPIOE, &GPIO_Init);
        GPIO_Init.Alternate     = GPIO_AF6_SAI1;
        GPIO_Init.Pin           = GPIO_PIN_6;
        HAL_GPIO_Init(GPIOE, &GPIO_Init);

        // Configure DMA used for SAI1_A
        // d2.s3.c0 // alternates: 2.1.0, 2.3.0, 2.6.10
        self->hdma_tx_a.Init.Channel                = DMA_CHANNEL_0;
        self->hdma_tx_a.Init.Direction              = DMA_MEMORY_TO_PERIPH;
        self->hdma_tx_a.Init.PeriphInc              = DMA_PINC_DISABLE;
        self->hdma_tx_a.Init.MemInc                 = DMA_MINC_ENABLE;
        self->hdma_tx_a.Init.PeriphDataAlignment    = DMA_PDATAALIGN_HALFWORD;
        self->hdma_tx_a.Init.MemDataAlignment       = DMA_MDATAALIGN_HALFWORD;
        self->hdma_tx_a.Init.Mode                   = DMA_CIRCULAR;
        self->hdma_tx_a.Init.Priority               = DMA_PRIORITY_HIGH;
        self->hdma_tx_a.Init.FIFOMode               = DMA_FIFOMODE_DISABLE;
        self->hdma_tx_a.Init.FIFOThreshold          = DMA_FIFO_THRESHOLD_FULL;
        self->hdma_tx_a.Init.MemBurst               = DMA_MBURST_SINGLE;
        self->hdma_tx_a.Init.PeriphBurst            = DMA_PBURST_SINGLE;

        self->hdma_tx_a.Instance                    = DMA2_Stream3;

        // Bidirectionally link the DMA & SAI handles
        __HAL_LINKDMA(hsai, hdmatx, self->hdma_tx_a);

        // Deinitialize the Stream for new transfer
        HAL_DMA_DeInit(&self->hdma_tx_a);

        // Configure the DMA Stream
        if( HAL_OK != HAL_DMA_Init(&self->hdma_tx_a) ){
            debug(&self->debug, "HAL_DMA_Init failed");
            return;
        }
    } else if(hsai == &selves[0]->hsai_b){ // BLOCK B
        Sai* self = selves[0];
        __HAL_RCC_SAI1_CLK_ENABLE(); // RCC
        __HAL_RCC_GPIOE_CLK_ENABLE();

        GPIO_Init.Alternate     = GPIO_AF6_SAI1;
        GPIO_Init.Pin           = GPIO_PIN_3;
        HAL_GPIO_Init(GPIOE, &GPIO_Init);

        // Configure DMA used for SAI1_B
        // d2.s5.c0 // alt d2.s4.c1, d2.s0.c10
        self->hdma_tx_b.Init.Channel               = DMA_CHANNEL_0;
        self->hdma_tx_b.Init.Direction             = DMA_MEMORY_TO_PERIPH;
        self->hdma_tx_b.Init.PeriphInc             = DMA_PINC_DISABLE;
        self->hdma_tx_b.Init.MemInc                = DMA_MINC_ENABLE;
        self->hdma_tx_b.Init.PeriphDataAlignment   = DMA_PDATAALIGN_HALFWORD;
        self->hdma_tx_b.Init.MemDataAlignment      = DMA_MDATAALIGN_HALFWORD;
        self->hdma_tx_b.Init.Mode                  = DMA_CIRCULAR;
        self->hdma_tx_b.Init.Priority              = DMA_PRIORITY_HIGH;
        self->hdma_tx_b.Init.FIFOMode              = DMA_FIFOMODE_DISABLE;
        self->hdma_tx_b.Init.FIFOThreshold         = DMA_FIFO_THRESHOLD_FULL;
        self->hdma_tx_b.Init.MemBurst              = DMA_MBURST_SINGLE;
        self->hdma_tx_b.Init.PeriphBurst           = DMA_PBURST_SINGLE;

        self->hdma_tx_b.Instance                   = DMA2_Stream5;

        // Bidirectionally link the DMA & SAI handles
        __HAL_LINKDMA(hsai, hdmatx, self->hdma_tx_b);

        // Deinitialize the Stream for new transfer
        HAL_DMA_DeInit(&self->hdma_tx_b);

        // Configure the DMA Stream
        if( HAL_OK != HAL_DMA_Init(&self->hdma_tx_b) ){
            debug(&self->debug, "HAL_DMA_Init failed");
            return;
        }

        // Codec request triggers transfer & new frame calc
        HAL_NVIC_SetPriority( DMA2_Stream5_IRQn
                            , 2
                            , 1
                            );
        HAL_NVIC_EnableIRQ( DMA2_Stream5_IRQn );
    }
}

static uint8_t bs[4] = {1,2,3,4};
void sai_write_hwords( Sai* self, uint16_t* hwords, uint16_t count){
    // if(HAL_SAI_Transmit_DMA(&self->hsai_b, bs, 4))
    // if(HAL_SAI_Transmit_DMA(&self->hsai_b, (uint8_t*)hwords, count))
    //     debug(&self->debug, "SAI block B failed to transmit");
    if(HAL_SAI_Transmit_DMA(&self->hsai_a, (uint8_t*)hwords, count))
        debug(&self->debug, "SAI block A failed to transmit");
}




/*
uint8_t CODEC_Start( block_process_fn_t processor )
{
    uint8_t error = 0;
    // if( !processor ){
    //     error = 3; return error;
    //     error_debug(NULL, "CODEC_Start failed");
    //     return 0;
    // }
    // block_process_fn = processor; // assign dsp process fn

    // Zero the output buffer
    // for( uint16_t i=0; i<2; i++ ){
    //     inBuff[i]  = 0;
    //     outBuff[i] = 0x77777777;
    // }
    // Enable DAC output
    if( HAL_SAI_Transmit_DMA( &sai1
                            , (uint8_t *)outBuff
                            , buffer_size
                            ) != HAL_OK ){ error = 1; }
    // Enable ADC audio input
    if( HAL_SAI_Receive_DMA( &sai2
                           , (uint8_t *)inBuff
                           , buffer_size
                           ) != HAL_OK ){ error = 2; }
    return error;
}
*/

// void CODEC_Stop( void )
// {
//     HAL_SAI_DMAStop( &sai1 );
//     HAL_SAI_DMAStop( &sai2 );
// }

// void HAL_SAI_MspDeInit(SAI_HandleTypeDef *hsai)
// {
//     HAL_NVIC_DisableIRQ( DMA2_Stream5_IRQn );
//     HAL_DMA_DeInit( &hSaiDma2 );
//     HAL_DMA_DeInit( &hSaiDma );
// }

// DMA triggered by codec requesting more ADC!
void DMA2_Stream5_IRQHandler(void){
    // debug(&selves[0]->debug, "sending");
    HAL_DMA_IRQHandler(&selves[0]->hdma_tx_b);
}

/*
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    // HAL_NVIC_DisableIRQ( DMA2_Stream5_IRQn );
    float in[b_size*2];
    float out[b_size*2];
    codec_to_floats( in, &in[b_size], &inBuff[0], b_size );
    (*block_process_fn)( out
                       , in
                       , b_size
                       );
    floats_to_codec( &outBuff[0], out, b_size );
    // HAL_NVIC_EnableIRQ( DMA2_Stream5_IRQn );
}
*/

/*
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
    HAL_NVIC_DisableIRQ( DMA2_Stream5_IRQn );
    float in[b_size*2];
    float out[b_size*2];
    codec_to_floats( in, &in[b_size], &inBuff[b_size*2], b_size );
    (*block_process_fn)( out
                       , in
                       , b_size
                       );
    floats_to_codec( &outBuff[b_size*2], out, b_size );
    HAL_NVIC_EnableIRQ( DMA2_Stream5_IRQn );
}
*/


///////////////////////////
// dsp pickle/unpickle

// #define MAXs32             0x7FFFFFFF
// #define MAXf32             ((float)0x7FFFFFFF)
// #define iMAXf32            (1/ (float)MAXs32)
// #define IO_SCALE           ((float)0.725)
// #define iMAXf32_NORMALIZED (iMAXf32 * IO_SCALE)

// static void codec_to_floats( float* f0, float* f1, __IO uint32_t* codec, int b_size){
//     for( int i=0; i<b_size; i++ ){
//         *f0++ = (float)(int32_t)(*codec++ << 8) * iMAXf32_NORMALIZED;
//         *f1++ = (float)(int32_t)(*codec++ << 8) * iMAXf32_NORMALIZED;
//     }
// }

// static void floats_to_codec( __IO uint32_t* codec, float* f, int b_size ){
//     for( int i=0; i<b_size; i++ ){
//         //*codec++ = (uint32_t)((int32_t)(*f++ * -MAXf32) >> 8);
//         //*codec++ = (uint32_t)((int32_t)(*f++ * -MAXf32) >> 8);
//         *codec++ = (uint32_t)((int32_t)(f[i] * -MAXf32) >> 8);
//         *codec++ = (uint32_t)((int32_t)(f[i+b_size] * -MAXf32) >> 8);
//     }
// }












