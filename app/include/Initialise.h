//*************************************************************************************
//*************************************************************************************
//File Name: ad1836.c
//Notes:
//Author: wu, zhigang
//Date:   Jul-15-2017
//*************************************************************************************
//*************************************************************************************

//******************************************************************************
// *  INCLUDED HEADERS
//******************************************************************************

#ifndef  __INIITALISE_H__
#define __INIITALISE_H__

#include <sys\exception.h>
#include <cdefBF533.h>
#include <ccblkfn.h>
#include <sysreg.h>
#include "ctypes.h"

/*******************************************************************************
 *  DEFINITIONS
 ******************************************************************************/

// Core Timer Registers
#define pTCNTL ((volatile unsigned long *)TCNTL)
#define pTPERIOD ((volatile unsigned long *)TPERIOD)
#define pTSCALE ((volatile unsigned long *)TSCALE)
#define pTCOUNT ((volatile unsigned long *)TCOUNT)

/*******************************************************************************
 *  GLOBALS
 ******************************************************************************/

/*******************************************************************************
 *  PROTOTYPES
 ******************************************************************************/
void Init_PLL(void);
void Init_Timer(void);
void Init_EBIU(void);
void Init_Flash(void);
void Init_Flags(void);
void Disable_Timer(void);
void CoreTimerInit( unsigned int scale, unsigned int counter);


// addresses for Port B in Flash A
#define pFlashA_PortA_Dir	(volatile unsigned char *)0x20270006
#define pFlashA_PortA_Data	(volatile unsigned char *)0x20270004

#define pFlashA_PortB_Dir	(volatile unsigned char *)0x20270007
#define pFlashA_PortB_Data	(volatile unsigned char *)0x20270005


#endif /* __INIITALISE_H__ */
