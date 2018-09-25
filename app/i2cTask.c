//*************************************************************************************
//*************************************************************************************
//File Name: i2cTask.c
//Notes:
//Author: wu, zhigang
//Date:   May-6-2017
//*************************************************************************************
//*************************************************************************************

#include "freeRTOS.h"
#include "task.h"
#include "i2c.h"
#include "i2cTask.h"


void vI2CTask(void *pvParameters)
{
    TickType_t xLastExecutionTime;
	TickType_t xDuration;
	bool bLEDOnOff;
    unsigned char i2cbuf[2] = {0};
    char nlen = 2;
    
	/* Initialise xLastExecutionTime so the first call to vTaskDelayUntil()
	    works correctly. */
	xLastExecutionTime = xTaskGetTickCount();
	xDuration = 10;		//ms
	
	bLEDOnOff = 1;
	*(pFlashA_PortB_Dir)  |= (1<<3);
	
    while( 1 )
    {
        //read the data from i2c bus
        i2c_read(0x56, i2cbuf, nlen);
    
        /* Wait until it is time for the next cycle. */
		vTaskDelayUntil( &xLastExecutionTime, xDuration );

		//LED7 blink
        if(bLEDOnOff == 1)
        {   
            *(pFlashA_PortB_Data) |= (1<<3);
            bLEDOnOff = 0;
        }
        else
        {
            *(pFlashA_PortB_Data) &= ~(1<<3);            
            bLEDOnOff = 1;
        }

    }
}



void vI2CTaskInit(void)
{
    xTaskCreate( vI2CTask, "I2C", configMINIMAL_STACK_SIZE, NULL, mainI2C_TASK_PRIORITY, NULL );
}

