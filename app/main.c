//*************************************************************************************
//*************************************************************************************
//File Name: main.c
//Notes:
//Author: wu, zhigang
//Date:   Jul-15-2017
//*************************************************************************************
//*************************************************************************************

//******************************************************************************
//*  INCLUDED HEADERS
//******************************************************************************
#include <defBF533.h>
#include "stdint.h"
#include "Initialise.h"
#include "clocks.h"
#include "sys_status.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "ad1836.h"

#include "i2cTask.h"
#include "ledTask.h"
#include "uartTask.h"
#include "sportTask.h"
#include "interrupt_register.h"




/*******************************************************************************
 *  GLOBALS
 ******************************************************************************/
U32 tics;
U32 boot_diagnostic_code;
 
/* The time use for the run time stats. */
unsigned long ulRunTime = 0UL;
 


void vSetupClockForRunTimeStats( void )
{
}



 /******************************************************************************
 *   FUNCTION:       main
 *   DESCRIPTION:    system and OS initialization.  Task creation and OS startup.
 *   INPUT(S):       
 *   OUTPUT(S):
 *   RETURN VALUE:   
 ******************************************************************************/
void main(void)
{
	SemaphoreHandle_t xsem;
    sysreg_write(reg_SYSCFG, 0x32);	
	Init_PLL();
	Init_EBIU();
	Init_Flags();
    Init_Flash();

	xsem = xSemaphoreCreateBinary();
	interrupt_handler_init(xsem);
		
    Init_AD1836();

	/* Create the tasks defined within this file. */
    vI2CTaskInit();
    vLEDTaskInit();
    vUARTTaskInit();
    vSPORTTaskInit();
    
    vTaskStartScheduler();

    /* Execution will only reach here if there was insufficient heap to start the scheduler. */
}



/******************************************************************************
*   FUNCTION:       Timer_Isr
*   DESCRIPTION:    maintains tic counter
*   INPUT(S):        
*   OUTPUT(S):
*   RETURN VALUE:   
******************************************************************************/
EX_INTERRUPT_HANDLER(Timer_Isr)
{
    // clear the interrupt latch
    *pTIMER_STATUS = TIMIL0;
    tics++;
}


/******************************************************************************
*   FUNCTION:       vApplicationSetupTimerInterrupt
*   DESCRIPTION:    initialize the timers
*   INPUT(S):        
*   OUTPUT(S):
*   RETURN VALUE:   
******************************************************************************/
void vApplicationSetupTimerInterrupt(void)
{
   // periodic interrupt timer
   *pTIMER0_CONFIG   = (PWM_OUT | PERIOD_CNT | IRQ_ENA | OUT_DIS);
   *pTIMER0_PERIOD   = OSCLK_PERIOD;         
   *pTIMER0_WIDTH    = OSCLK_PERIOD / 2;
   *pTIMER_ENABLE    = TIMEN0;

   // REVISIT - SIC_IAR registers are using default values
   //  these should be initialized

   // assign software interrupt to OS context switch routine
   // this interrupt vector also is shared with the I2C port
   register_handler(ik_ivg11, Timer_Isr);     // I2C ISR  -> IVG 11
  
   // peripheral enable Timer 0 
   *pSIC_IMASK |= TIMER0_IRQ;
}



/******************************************************************************
* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
* implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
* used by the Idle task. 
******************************************************************************/
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

	/* Pass out a pointer to the StaticTask_t structure in which the Idle task's
	state will be stored. */
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

	/* Pass out the array that will be used as the Idle task's stack. */
	*ppxIdleTaskStackBuffer = uxIdleTaskStack;

	/* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
	Note that, as the array is necessarily of type StackType_t,
	configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}



/******************************************************************************
* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
* application must provide an implementation of vApplicationGetTimerTaskMemory()
* to provide the memory that is used by the Timer service task.
******************************************************************************/
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

	/* Pass out a pointer to the StaticTask_t structure in which the Timer
	task's state will be stored. */
	*ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

	/* Pass out the array that will be used as the Timer task's stack. */
	*ppxTimerTaskStackBuffer = uxTimerTaskStack;

	/* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
	Note that, as the array is necessarily of type StackType_t,
	configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}


