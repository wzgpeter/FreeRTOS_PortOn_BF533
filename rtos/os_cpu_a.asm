/*
*********************************************************************************************************
*                                               uC/OS-II
*                                         The Real-Time Kernel
*
*										O/S specific functions for
*											  Analog Devices
*											Blackfin ADSP-BF53x
*
* File : OS_CPU_ASM.C
* Originally By : Ron Territo   ron@territocomputerservices.com
* Modified By : Deep Bodapati deep.bodapati@analog.com
*********************************************************************************************************

Copyright...

This code is placed in the public domain, and can be distributed freely with no restrictions provided that the heading
of each source module file is not modified to remove the credit to the original, and subsequent, author(s).
  
Disclaimer...

This program code is provided "as is". There is no warranty, either expressed or implied as to its fitness for use in
any application. It is provided only as an example of porting the MicroC/OS operating system to the Blackfin processor.
Its use is strictly at the risk of the user. The author(s) will not be liable for any damages direct or consequential 
related to the use of this software including, but not limited to loss of profit.

*/

#include <def_LPBlackfin.h>

/*
*****************************************************************************
*
*	Global variables
*
*****************************************************************************
*/

.extern _OSTimeTick;
.extern _pxCurrentTCB;
.extern _vTaskSwitchContext;

.section L1_code;


#define CPU_SAVE_CONTEXT()\
	[ --sp ]	= rets;\
	[ --sp ] 	= fp;\
	[ --sp ] 	= astat;\
	[--sp] = (r7:0, p5:0);\
	[--sp] = lc0;\
	[--sp] = lc1;\
	[--sp] = lt0;\
	[--sp] = lt1;\
	[--sp] = lb0;\
	[--sp] = lb1;\
	[--sp] = a0.W;\
	[--sp] = a0.X;\
	[--sp] = a1.W;\
	[--sp] = a1.X;\
	[--sp] = i0;\
	[--sp] = i1;\
	[--sp] = i2;\
	[--sp] = i3;\
	[--sp] = b0;\
	[--sp] = b1;\
	[--sp] = b2;\
	[--sp] = b3;\
	[--sp] = l0;\
	[--sp] = l1;\
	[--sp] = l2;\
	[--sp] = l3;\
	[--sp] = m0;\
	[--sp] = m1;\
	[--sp] = m2;\
	[--sp] = m3;
	
#define CPU_RESTORE_CONTEXT() \
	m3 = [sp++];\
	m2 = [sp++];\
	m1 = [sp++];\
	m0 = [sp++];\
	l3 = [sp++];\
	l2 = [sp++];\
	l1 = [sp++];\
	l0 = [sp++];\
	b3 = [sp++];\
	b2 = [sp++];\
	b1 = [sp++];\
	b0 = [sp++];\
	i3 = [sp++];\
	i2 = [sp++];\
	i1 = [sp++];\
	i0 = [sp++];\
	a1.X = [sp++];\
	a1.W = [sp++];\
	a0.X = [sp++];\
	a0.W = [sp++];\
	lb1 = [sp++];\
	lb0 = [sp++];\
	lt1 = [sp++];\
	lt0 = [sp++];\
	lc1 = [sp++];\
	lc0 = [sp++];\
	(r7:0, p5:0) = [sp++];\
	astat		= [ sp++ ];\
	fp 			= [ sp++ ];\
	rets 		= [ sp++ ];





//==========================================================================================
//  Function:
//      this function OSStartHighRdy() will be called only one time.
//      the OS will select a highest priority task to start for the first time.
//==========================================================================================
.global _OSStartHighRdy;


_OSStartHighRdy:

    // need to save RETS since _OSStartHighRdy never returns	

    // Get the SP for the highest ready task
    p1.L    = _pxCurrentTCB;
    p1.H    = _pxCurrentTCB;
    p2 	    = [ p1 ];
    sp 	    = [ p2 + 0 ];
    nop; nop; nop;

    sp      += 4;
    									
    // Restore CPU context
    CPU_RESTORE_CONTEXT()						

    // Return to high ready task
    rts;

_OSStartHighRdy.end:




//==========================================================================================
// Function:
//   This function is called to switch the context of the current running task
//   This function is registered as the IVG14 handler and will be called to handle both 
//                  interrupt and task level context switches.
//==========================================================================================

.global _OSCtxSw;

_OSCtxSw:

    // Save context, interrupts disabled by IPEND[4] bit
    CPU_SAVE_CONTEXT()

    // The task Program Counter is in RETI, 
    // IPEND[4] is not cleared when it is stored
    // on the stack via an intermediate register like r1
    // so interrupts remain disabled 
    r1        = reti;
    [ --sp ]  = r1;								

    //p1 -> pxCurrentTCB
    p1.L      = _pxCurrentTCB;
    p1.H      = _pxCurrentTCB;
    p2        = [p1];      //p2 -> pxCurrentTCB->pxTopOfStack
    [p2]      = sp;        //store the switched out task's stack	

    // to get the higher priority task to switch in
    call      _vTaskSwitchContext;

    // Sometime the highest priority task is current task,
    // i still proceed here without skipping below code.
    // Get a highest ready task priority
    p3.L      = _pxCurrentTCB;	
    p3.H      = _pxCurrentTCB;
    p2        = [ p3 ];

    sp        = [ p2 + 0 ];	
    nop; nop; nop;
    	
    // Recover the start address of the task
    reti      = [ sp++ ];

    // Restoring CPU context
    CPU_RESTORE_CONTEXT()							

    // Reenable interrupts via IPEND[4] bit and return to task
    rti;													

_OSCtxSw.end:



//==========================================================================================
// Function:
//      this Tick ISR will be called in a fixed tick for each timer interruption.
//==========================================================================================

.global _OSTickISR

_OSTickISR:

    /* Save context, interrupts disabled by IPEND[4] bit */
    CPU_SAVE_CONTEXT()

    call _OSTimeTick;

        /* Restoring CPU context */
    CPU_RESTORE_CONTEXT()
    	
    /* Reenable interrupts via IPEND[4] bit and return to task */
    rti;

_OSTickISR.end:

