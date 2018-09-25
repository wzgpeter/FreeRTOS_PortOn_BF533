//*************************************************************************************
//*************************************************************************************
//File Name: ledTask.c
//Notes:
//Author: wu, zhigang
//Date:   May-6-2017
//*************************************************************************************
//*************************************************************************************

#include "freeRTOS.h"
#include "task.h"
#include "ledTask.h"


void vLEDTask(void *pvParameters)
{
    TickType_t xLastExecutionTime;
    TickType_t xDuration;
    bool bLEDOnOff;

	/* Initialise xLastExecutionTime so the first call to vTaskDelayUntil()
	    works correctly. */
	xLastExecutionTime = xTaskGetTickCount();
	xDuration = 30;		//ms

    bLEDOnOff = 1;
    *(pFlashA_PortB_Dir)  |= (1<<4);
    
    while( 1 )
    {
        /* Wait until it is time for the next cycle. */
		vTaskDelayUntil( &xLastExecutionTime,  xDuration);
/*      
        //LED8 blink
        if(bLEDOnOff == 1)
        {   
            *(pFlashA_PortB_Data) |= (1<<4);
            bLEDOnOff = 0;
        }
        else
        {
            *(pFlashA_PortB_Data) &= ~(1<<4);            
            bLEDOnOff = 1;
        }
*/
    }
}



void vLEDTaskInit(void)
{
    xTaskCreate( vLEDTask, "LED", configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY, NULL );
}



