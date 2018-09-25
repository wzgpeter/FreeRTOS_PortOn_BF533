/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the RX200 port.
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "Initialise.h"

/* Library includes. */
#include "string.h"

/* Hardware specifics. */
//#include "iodefine.h"

/*-----------------------------------------------------------*/

/* Tasks should start with interrupts enabled and in Supervisor mode, therefore
PSW is set with U and I set, and PM and IPL clear. */
#define portINITIAL_PSW     ( ( StackType_t ) 0x00030000 )

/*-----------------------------------------------------------*/

/* The following lines are to ensure vSoftwareInterruptEntry can be referenced,
 and therefore installed in the vector table, when the FreeRTOS code is built
as a library. */
extern BaseType_t vSoftwareInterruptEntry;
//const BaseType_t * p_vSoftwareInterruptEntry = &vSoftwareInterruptEntry;

/*-----------------------------------------------------------*/

/*
 * Function to start the first task executing - written in asm code as direct
 * access to registers is required.
 */
static void prvStartFirstTask( void );

/*
 * Software interrupt handler.  Performs the actual context switch (saving and
 * restoring of registers).  Written in asm code as direct register access is
 * required.
 */
static void prvYieldHandler( void );

/*
 * The entry point for the software interrupt handler.  This is the function
 * that calls the inline asm function prvYieldHandler().  It is installed in
 * the vector table, but the code that installs it is in prvYieldHandler rather
 * than using a #pragma.
 */
void vSoftwareInterruptISR( void );

/*-----------------------------------------------------------*/

/* This is accessed by the inline assembler functions so is file scope for
convenience. */
extern void *pxCurrentTCB;
//extern void vTaskSwitchContext( void );

#define portINITIAL_CRITICAL_NESTING	( ( uint16_t ) 10 )
volatile uint16_t usCriticalNesting = portINITIAL_CRITICAL_NESTING;

PRIVILEGED_DATA volatile uint32_t uTimeTick = 0;


PRIVILEGED_DATA volatile uint8_t bLEDOnOff = 0;

/*-----------------------------------------------------------*/

void OSTimeTick(void)
{
    uTimeTick++;	
 
    //check task list and scheduling
    vTickISR();

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
}



/*-----------------------------------------------------------*/

//register the OS context switcher ISR into the vector
void vSetupOsCtxSw(void)
{
    register_handler_ex(ik_ivg14, (ex_handler_fn)OSCtxSw, EX_INT_ENABLE);
}



/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
    uint32_t i;
    
    //RETS value -- pushing start address for task
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) pxCode;

    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t )0;   //FP value -- caller's frame pointer -- value irrelevant

    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t )0;   //ASTAT value -- caller's ASTAT value -- value irrelevant

    pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) pvParameters; /* R0 */

    for (i=39; i>0; i--)				// remaining reg values - R7:1, P5:0, LC0, LC1, LT0, LT1, LB0, LB1, 
    {                                   // A0.W, A0.X, A1.W, A1.X, I3:0, B3:0, L3:0, M3:0
        pxTopOfStack--;    
        *pxTopOfStack = (StackType_t) 0;      // All values irrelevant
    }

    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) pxCode;   //RETI value -- pushing start address for task

	return pxTopOfStack;
}
/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void )
{
unsigned int IMASK_reg;

extern void vApplicationSetupTimerInterrupt( void );

	/* Use pxCurrentTCB just so it does not get optimised away. */
	if( pxCurrentTCB != NULL )
	{
        usCriticalNesting = 0;
    
		/* Call an application function to set up the timer that will generate the
		tick interrupt.  This way the application can decide which peripheral to
		use.  A demo application is provided to show a suitable example. */
		vApplicationSetupTimerInterrupt();

#define CTSCALE     0
#define CTCOUNT     ((CLK_VCO)/(OSCLK)/8)  //((CLK_SCLK) / (OSCLK))
    	CoreTimerInit(CTSCALE, CTCOUNT);
		
        //enable the task switching
        vSetupOsCtxSw();
        
		/* Start the first task. */
		prvStartFirstTask();
	}

	/* Just to make sure the function is not optimised away. */
	( void ) vSoftwareInterruptISR();

	/* Should not get here. */
	return pdFAIL;
}
/*-----------------------------------------------------------*/

//#pragma inline_asm prvStartFirstTask
static void prvStartFirstTask( void )
{
    OSStartHighRdy();
}
/*-----------------------------------------------------------*/

//#pragma interrupt ( vTickISR( vect = _VECT( configTICK_VECTOR ), enable ) )
void vTickISR( void )
{
    unsigned int cpu_sr;

	/* Increment the tick, and perform any processing the new tick value
	    necessitates. */
	//set_ipl( configMAX_SYSCALL_INTERRUPT_PRIORITY );
	{
		if( xTaskIncrementTick() != pdFALSE )
		{
			taskYIELD();
		}
	}
	//set_ipl( configKERNEL_INTERRUPT_PRIORITY );
}
/*-----------------------------------------------------------*/

void vSoftwareInterruptISR( void )
{
	prvYieldHandler();
}
/*-----------------------------------------------------------*/

//#pragma inline_asm prvYieldHandler
static void prvYieldHandler( void )
{
#if 0
	/* Re-enable interrupts. */
	SETPSW	I

	/* Move the data that was automatically pushed onto the interrupt stack when
	the interrupt occurred from the interrupt stack to the user stack.

	R15 is saved before it is clobbered. */
	PUSH.L	R15

	/* Read the user stack pointer. */
	MVFC	USP, R15

	/* Move the address down to the data being moved. */
	SUB		#12, R15
	MVTC	R15, USP

	/* Copy the data across. */
	MOV.L	[ R0 ], [ R15 ] ; R15
	MOV.L 	4[ R0 ], 4[ R15 ]  ; PC
	MOV.L	8[ R0 ], 8[ R15 ]  ; PSW

	/* Move the interrupt stack pointer to its new correct position. */
	ADD	#12, R0

	/* All the rest of the registers are saved directly to the user stack. */
	SETPSW	U

	/* Save the rest of the general registers (R15 has been saved already). */
	PUSHM	R1-R14

	/* Save the accumulator. */
	MVFACHI	R15
	PUSH.L	R15
	MVFACMI	R15	; Middle order word.
	SHLL	#16, R15 ; Shifted left as it is restored to the low order word.
	PUSH.L	R15

	/* Save the stack pointer to the TCB. */
	MOV.L	#_pxCurrentTCB, R15
	MOV.L	[ R15 ], R15
	MOV.L	R0, [ R15 ]

	/* Ensure the interrupt mask is set to the syscall priority while the kernel
	structures are being accessed. */
	MVTIPL	#configMAX_SYSCALL_INTERRUPT_PRIORITY

	/* Select the next task to run. */
	BSR.A	_vTaskSwitchContext

	/* Reset the interrupt mask as no more data structure access is required. */
	MVTIPL	#configKERNEL_INTERRUPT_PRIORITY

	/* Load the stack pointer of the task that is now selected as the Running
	state task from its TCB. */
	MOV.L	#_pxCurrentTCB,R15
	MOV.L	[ R15 ], R15
	MOV.L	[ R15 ], R0

	/* Restore the context of the new task.  The PSW (Program Status Word) and
	PC will be popped by the RTE instruction. */
	POP		R15
	MVTACLO	R15
	POP		R15
	MVTACHI	R15
	POPM	R1-R15
	RTE
	NOP
	NOP
#endif 
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
	/* Not implemented in ports where there is nothing to return to.
	Artificially force an assert. */
	configASSERT( pxCurrentTCB == NULL );

	/* The following line is just to prevent the symbol getting optimised away. */
	( void ) vTaskSwitchContext();
}
/*-----------------------------------------------------------*/



