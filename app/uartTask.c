//*************************************************************************************
//*************************************************************************************
//File Name: uart.c
//Notes:
//Author: wu, zhigang
//Date:   May-6-2017
//*************************************************************************************
//*************************************************************************************

#include "freeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "uartTask.h"
#include "uart_dma.h"


TaskHandle_t xUartTaskHandle = NULL;
static uint8_t wbuf[16], wlen;
static uint8_t rbuf[16], rlen;
void vUARTTask(void *pvParameters)
{
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(500);
    BaseType_t xResult;
    uint32_t ulNotifiedValue;
    UART_DMA_CONTEXT *ctx;
    TickType_t xLastExecutionTime;
    TickType_t xDuration;

    xDuration = 1000;		//ms
    xLastExecutionTime = xTaskGetTickCount();
    ctx = (UART_DMA_CONTEXT *)uart_dma_init();

    //for the internal loop, start uart write first
    wbuf[0] = 0x5A;
    wbuf[1] = 0x5A;
    wbuf[2] = 0x5A;
    wbuf[3] = 0x00;
    wbuf[4] = 0x00;
    wbuf[5] = 0x00;
    wbuf[6] = 0x00;
    wbuf[7] = 0x00;
    wlen = 3;

    rlen = 3;
    uart_dma_write(ctx, wbuf, wlen);
    
    while( 1 )
    {
        //vTaskDelayUntil( &xLastExecutionTime, xDuration );

        uart_dma_read(ctx, rbuf, rlen);

        //if(rbuf[0] == 0xC0)
            uart_dma_write(ctx, wbuf, wlen);
    }
}


void vUARTTaskInit( void )
{
    xTaskCreate( vUARTTask, "UART", configMINIMAL_STACK_SIZE, NULL, mainUART_TASK_PRIORITY, &xUartTaskHandle );
}



