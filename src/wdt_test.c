/*******************************************************************
Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software
you agree to the terms of the associated Analog Devices License Agreement.

Project Name:  	Power_On_Self_Test

Hardware:		ADSP-21479/21489 EZ-Board

Description:	This file tests the watchdog timer interface on the EZ-Board.
*******************************************************************/

/*******************************************************************
*  include files
*******************************************************************/
#ifdef __ADSP21489__
#include <cdef21489.h>
#include <def21489.h>
#elif __ADSP21479__
#include <cdef21479.h>
#include <def21479.h>
#endif
#include "post_debug.h"
#include "pb_led_test.h"

/*******************************************************************
*  global variables and defines
*******************************************************************/
#define CMD_UNLOCK 0xAD21AD21
#define CMD_LOCK	0
#define COUNT_VALUE	0x3FFFFF
#define TRIP_VALUE	0x7

#if defined(__DEBUG_FILE__)
extern FILE *pDebugFile;				/* debug file */
#endif

/*******************************************************************
*  function prototypes
*******************************************************************/
void Init_WDT(void);
void UpdateLEDs(int);


/*******************************************************************
*   Function:    Init_WDT
*   Description: Initialize WDT with the appropriate values
*******************************************************************/
void Init_WDT(void)
{
	int n = 0;

	*pWDTUNLOCK = CMD_UNLOCK;
				
	*pWDTCLKSEL = WDT_CLK_SRC_CER_RESO;

	*pWDTCNT = COUNT_VALUE;
	
	*pWDTTRIP = TRIP_VALUE;
	
	// WDT effect latency - 2.5 WDTCLK cycles at 2 MHz
	/* wait at least 4096 cycles for the pll to lock */
	for(n = 0; n < 500; n++ )
	{
		asm("nop;");
	}
	*pWDTCTL = WDT_EN;
	
	*pWDTUNLOCK = CMD_LOCK;

}


/*******************************************************************
*   Function:    Test_Watchdog_Timer
*   Description: Performs a test on WDT.
*******************************************************************/
int Test_Watchdog_Timer(void)
{
	int n = 0, i = 0;

	Init_WDT();

	// Delay loop, look at WDTTRIP and if change, modify the LED output
	i = (*pWDTTRIP >> 4) & 0xF;
			UpdateLEDs(i);

	if(i)	/* Has WDT test started */
	{
		DEBUG_PRINT("\n\nWDT: Pass %d", i);

		if(TRIP_VALUE == i)
		{
			DEBUG_STATEMENT("\nYou must press reset button");
		}
	}
	else
	{
		DEBUG_HEADER( "Watchdog Timer Test" );
	}
	
	// Cause watch dog timer to time out
	for(n = 0; n < 0xFFFFFFF; n++ )
		{
			asm("nop;");
		}
	return 1;	// If the timer does not time out, something is broken.
}
void UpdateLEDs(int nValue)
{
	int i = 0, nLed = 0;
	for (i = 0; i < 4; ++i)
	{
		nLed = (1 << i);
		if(nValue & (1 << i))
			ClearSet_LED(nLed, LED_ON);
		else
			ClearSet_LED(nLed, LED_OFF);
	}
}


