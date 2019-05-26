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
#include "sport_api.h"
#include "interrupt_register.h"

TaskHandle_t xSportTaskHandle = NULL;

void vSPORTTask(void *pvParameters)
{
    uint32_t ulNotifiedValue;
    BaseType_t ret;
    uint8_t option_flag = SPORT_DMA_I2S_RX_INIT | SPORT_DMA_I2S_TX_INIT;
    uint32_t i;

	register_interrupt_handler(&sport0_dma1_rx_int_cfg);
	register_interrupt_handler(&sport0_dma2_tx_int_cfg);

	//init the Tx and Rx SPORT port0
	struct sport_dev *dev = sport_get(SPORT, SPORT0);

	//config the sport rx registers
	i = 0;
	while(sport0_rx_cfg[i].cmd != SPORT_CFG_END)
	{
		aud_ioctl(dev, sport0_rx_cfg[i].cmd, sport0_rx_cfg[i].val);
		i++;
	}

	//config the sport tx registers
	i = 0;
	while(sport0_tx_cfg[i].cmd != SPORT_CFG_END)
	{
		aud_ioctl(dev, sport0_tx_cfg[i].cmd, sport0_tx_cfg[i].val);
		i++;
	}

	aud_ioctl(dev, SPORT_MODE, SPORT_DMA_MODE_I2S_TX | SPORT_DMA_MODE_I2S_RX);
	aud_ioctl(dev, SPORT_DIRT, SPORT_DMA_WRITE | SPORT_DMA_READ);
	aud_open(dev, SPORT0);

    while(1)
    {
        ret = xTaskNotifyWait(pdFAIL,
                              ULONG_MAX,
                              &ulNotifiedValue,
                              portMAX_DELAY
                              );
        if(ret != pdFALSE)
        {
			sport_proc(dev);
        }
    }
}


void vSPORTTaskInit( void )
{
    xTaskCreate( vSPORTTask, "SPORT", configMINIMAL_STACK_SIZE, NULL, mainSPORT_TASK_PRIORITY, &xSportTaskHandle );
}



