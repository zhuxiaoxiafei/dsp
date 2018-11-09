/*******************************************************************
Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software
you agree to the terms of the associated Analog Devices License Agreement.

Project Name:  	Power_On_Self_Test

Hardware:		ADSP-21479/21489 EZ-Board

Description:	This examples performs a processor version check on the EZ-Board.
*******************************************************************/
#ifdef __ADSP21489__
#include <cdef21489.h>
#include <def21489.h>
#elif __ADSP21479__
#include <cdef21479.h>
#include <def21479.h>
#endif
#include "post_debug.h"

#if defined(__DEBUG_FILE__)
extern FILE *pDebugFile;				/* debug file */
#endif

/*******************************************************************
*   Function:    Test_Processor_Version
*   Description: This function compares the version of the processor
* 				 being run on with the version that the software was
*				 built against.  This will catch a different version
*				 of silicon than what it was built for.
*******************************************************************/
int Test_Processor_Version(void)
{
#if !defined (__SILICON_REVISION__)
	return 0; // failed
#else
	int running_on = *pREVPID>>4;			/* check the part */
	int built_for = __SILICON_REVISION__;	/* check what we built against */

	DEBUG_HEADER( "Processor Version Test" );

#if ((__SILICON_REVISION__ == 0x0001) && __ADSP21489__)	/* if building for rev 0.1 */
	/* anomaly 15000017 - Incorrect Revision Number in REVPID Register */
	if(*pREVPID == 0x6)
	{
		running_on = 1;
	}
#endif

	DEBUG_PRINT("\nBuilt for version %d, running on version %d", built_for, running_on);

	if( built_for != running_on )
	{
		/* if different, fail */
		DEBUG_STATEMENT("\nTest failed");
		return 0;
	}

	/* if same, pass */
	DEBUG_STATEMENT("\nTest passed");
	return 1;
#endif
}
