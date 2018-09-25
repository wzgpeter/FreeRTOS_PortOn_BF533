//*************************************************************************************
//*************************************************************************************
//File Name: sys_status.c
//Notes:
//Author: wu, zhigang
//Date:   Jul-15-2017
//*************************************************************************************
//*************************************************************************************


#ifndef __SYS_STATUS_H__
#define __SYS_STATUS_H__

//******************************************************************************
// *  INCLUDED HEADERS
//******************************************************************************
#include <sys\exception.h>
#include <cdefBF533.h>
#include <ccblkfn.h>
#include <sysreg.h>
#include "ctypes.h"
#include "flagio.h"

//******************************************************************************
//*  DEFINITIONS
//******************************************************************************
/* 
   Defines that data that resides in the variuos flash sectors.  Used to identify
   which sectors need to be erased when performing a flash update.
*/
#ifndef FLASH_SECTORS_BOOTLOADER
#define FLASH_SECTORS_BOOTLOADER   {0,3}
#endif

#ifndef FLASH_SECTORS_DTCP_KEYS
#define FLASH_SECTORS_DTCP_KEYS    {1,2}
#endif

#if FLASH_SIZE==0x100000
#ifndef FLASH_SECTORS_APPLICATION
#define FLASH_SECTORS_APPLICATION  {4,5,6,7,8,9,10,11,12,13,14,15,16}
#endif 

#ifndef FLASH_SECTORS_EQ_TABLE
#define FLASH_SECTORS_EQ_TABLE     {18}
#endif

#ifndef FLASH_SECTORS_DEC
#define FLASH_SECTORS_DEC          {17}
#endif

#else // FLASH_SIZE is default
#ifndef FLASH_SECTORS_APPLICATION
#define FLASH_SECTORS_APPLICATION  {4,5,6,7,8,9}
#endif 

#ifndef FLASH_SECTORS_EQ_TABLE
#define FLASH_SECTORS_EQ_TABLE     {10}
#endif

#ifndef FLASH_SECTORS_DEC
#define FLASH_SECTORS_DEC          {}
#endif

#endif

/*******************************************************************************
 *  EXTERNAL (make visable to others)
 ******************************************************************************/


/*******************************************************************************
 *  MACROS
 ******************************************************************************/
#define MRESET()       (*pFIO_FLAG_D &= ~(bMANUAL_RST))
#define MAX(a,b) (a>b?b:a)
#define MIN(a,b) (a<b?b:a)

/*******************************************************************************
 *  PROTOTYPES
 ******************************************************************************/
void pet_watchdog ( void );


#endif /* __SYS_STATUS_H__ */

