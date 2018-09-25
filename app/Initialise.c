/*******************************************************************************
 *  INCLUDED HEADERS
 ******************************************************************************/
#include <defBF533.h>
#include "FreeRTOS.h"
#include "Initialise.h"
#include "clocks.h"
#include "flagio.h"
#include "sys_status.h"

extern U32 tics;


/******************************************************************************
 *   FUNCTION:       Init_Flags
 *   DESCRIPTION:    Initializes necessary GPIO
 *   INPUT(S):        
 *   OUTPUT(S):
 *   RETURN VALUE:   
 ******************************************************************************/
void Init_Flags(void)
{
    /* Set PF9 as output (Watchdog pet pin) */
	*pFIO_DIR = bWDOG_PET;
}


//--------------------------------------------------------------------------//
// Function:	Init_EBIU													//
//																			//
// Parameters:	None														//
//																			//
// Return:		None														//
//																			//
// Description:	This function initialises and enables the asynchronous		//
//				memory banks for the External Bus Interface Unit (EBIU), so	//
//				that access to Flash A is possible.							//
//--------------------------------------------------------------------------//
void Init_EBIU(void)
{
	*pEBIU_AMBCTL0	= 0x7bb07bb0;
	*pEBIU_AMBCTL1	= 0x7bb07bb0;
	*pEBIU_AMGCTL	= 0x000f;
}


//--------------------------------------------------------------------------//
// Function:	Init_Flash													//
//																			//
// Parameters:	None														//
//																			//
// Return:		None														//
//																			//
// Description:	This function sets the pin direction of Port B in Flash A	//
//				to output.													//
//				The LEDs on the ADSP-BF533 EZ-KIT are connected to Port B.	//
//--------------------------------------------------------------------------//
void Init_Flash(void)
{
    *pFlashA_PortA_Dir = 0x01;

	*pFlashA_PortB_Dir = 0x3f;
}



 /******************************************************************************
 *   FUNCTION:       Disable_Timer
 *   DESCRIPTION:    Return timer0 to as close to POR state as possible
 *   INPUT(S):        
 *   OUTPUT(S):
 *   RETURN VALUE:   
 ******************************************************************************/
void Disable_Timer(void)
{
	// Disable timer interrupt
	*pSIC_IMASK &= ~(TIMER0_IRQ);

	// Unregister interrupt handler
	register_handler(ik_ivg11, EX_INT_DEFAULT);
	
	// Reset timer0 to default	
	*pTIMER_DISABLE = TIMEN0;
	*pTIMER_STATUS = TRUN0;

	*pTIMER0_WIDTH = 0x00000000;
	*pTIMER0_PERIOD = 0x00000000;
	*pTIMER0_CONFIG = 0x0000;
}

 /******************************************************************************
 *   FUNCTION:       Init_PLL
 *   DESCRIPTION:    Turns up the PLL frequency
 *   INPUT(S):        
 *   OUTPUT(S):
 *   RETURN VALUE:   
 ******************************************************************************/
void Init_PLL(void)
{
	unsigned int IMASK_reg;

	IMASK_reg = cli();
    *pSIC_IWR = 0x1;    //TIMER0 can be used to wakeup core from idle state
	*pPLL_CTL = SET_MSEL(VCO_MULTIPLIER);
	*pPLL_DIV = CCLK_DIV1 | SET_SSEL(SCLK_DIVIDER);
    //ssync();
	//idle();
	sti(IMASK_reg);
}



void CoreTimerInit( unsigned int scale, unsigned int counter)
{
    *pTCNTL   = 1;                                   // Turn on timer, TMPWR
    *pTSCALE  = scale;                               // Load scale
    *pTCOUNT  = counter;                             // Load counter
    *pTPERIOD = counter;                             // Load counter into period as well

    register_handler_ex (ik_timer, (ex_handler_fn)OSTickISR, EX_INT_ENABLE);
    
    *pTCNTL   = 0x7;                                 // Start Timer and set Auto-reload
}


/*
void Init_PLL(void)
{
	volatile int test=0;

	sysreg_write(reg_SYSCFG, 0x32);		//Initialize System Configuration Register

	*pSIC_IWR = 0x1;
	*pPLL_CTL = 0x2C00;
	ssync();
	idle();
}//end Init_PLL
*/

