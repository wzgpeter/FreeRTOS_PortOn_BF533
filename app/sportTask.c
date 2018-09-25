//*************************************************************************************
//*************************************************************************************
//File Name: sportTask.c
//Notes:
//Author: wu, zhigang
//Date:   Jul-14-2017
//*************************************************************************************
//*************************************************************************************
#include "freeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "sportTask.h"
#include "sport_dma.h"


TaskHandle_t xSportTaskHandle = NULL;
//int32_t AudioProcBuffer[RX_WORDS_PER_BLOCK];


void vSPORTTask(void *pvParameters)
{
    uint32_t ulNotifiedValue;
    BaseType_t return_code;
    uint8_t option_flag = SPORT_DMA_I2S_RX_INIT | SPORT_DMA_I2S_TX_INIT;
    uint32_t i;

    sport_ctx.mode = SPORT_DMA_MODE_I2S_RX | SPORT_DMA_MODE_I2S_TX;
    sport_dma_init(&sport_ctx, option_flag, SPORT0);

    sport_dma_enable(&sport_ctx, SPORT0);
    
    while(1)
    {
        return_code = xTaskNotifyWait(pdFAIL,
                                      ULONG_MAX,
                                      &ulNotifiedValue,
                                      portMAX_DELAY
                                      );
        if(return_code != pdFALSE)
        {
            for(i=0; i<RX_WORDS_PER_BLOCK; i++)
            {
                AudioTxBuffer[sport_ctx.txblk_ptr][i] = AudioRxBuffer[sport_ctx.rxblk_ptr][i];
            }

            sport_ctx.rxblk_ptr ^= 1;
            sport_ctx.txblk_ptr ^= 1;
        }
    }
}


void vSPORTTaskInit( void )
{
    xTaskCreate( vSPORTTask, "SPORT", configMINIMAL_STACK_SIZE, NULL, mainSPORT_TASK_PRIORITY, &xSportTaskHandle );
}



