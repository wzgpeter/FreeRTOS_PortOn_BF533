//*************************************************************************************
//*************************************************************************************
//File Name: sys_status.c
//Notes:
//Author: wu, zhigang
//Date:   Jul-15-2017
//*************************************************************************************
//*************************************************************************************

//******************************************************************************
// *  INCLUDED HEADERS
//******************************************************************************
#include <stdlib.h>
#include <string.h>
#include "Initialise.h"
#include "sys_status.h"
#include "flashdef.h"
#include "clocks.h"
#include "flagio.h"


/******************************************************************************
*   FUNCTION:       pet_watchdog
*   DESCRIPTION:    toggle the external watchdog timer to avoid reset
*   AUTHOR:         K. Furge
*   INPUT(S):       
*   OUTPUT(S):
*   RETURN VALUE:   always returns 0
******************************************************************************/
void pet_watchdog(void)
{
	U16 flags;
	
	flags = *pFIO_FLAG_D;
	
	if ((flags & bWDOG_PET) == bWDOG_PET)
	{
		*pFIO_FLAG_C = bWDOG_PET;
	}
	else
	{
		*pFIO_FLAG_S = bWDOG_PET;
	}
}


/******************************************************************************
*   FUNCTION:       reset
*   DESCRIPTION:    Pulls the watchdog controller's /MR pin
*   AUTHOR:         K. Furge
*   INPUT(S):       
*   OUTPUT(S):
*   RETURN VALUE:   always returns 0
******************************************************************************/
void reset(void)
{
	/* Set PF10 as output */
	*pFIO_DIR |= bMANUAL_RST;
	
	/* Drive it low and say goodnight... */
	*pFIO_FLAG_C = bMANUAL_RST;
}




