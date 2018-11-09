/*******************************************************************
Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software
you agree to the terms of the associated Analog Devices License Agreement.

Project Name:  	Power_On_Self_Test

Hardware:		ADSP-21479/21489 EZ-Board

Description:	This examples configures pushbutton and LED tests on the EZ-Board.
*******************************************************************/

#ifdef __ADSP21489__
#include <cdef21489.h>
#include <def21489.h>
#elif __ADSP21479__
#include <cdef21479.h>
#include <def21479.h>
#endif
#include <sru.h>
#include <signal.h>
#include <sysreg.h>
#include <services\int\adi_int.h>

#include "post_common.h"
#include "post_debug.h"
#include "pb_led_test.h"

/*******************************************************************
*  function prototypes
*******************************************************************/
void SW8_IRQ1_handler(int sig, void *);
void SW9_IRQ2_handler(int sig, void *);
void SW10_SW11_DAI_handler(int, void *);

/*******************************************************************
*  global variables
*******************************************************************/
static int g_nNumLEDs = 8;
int g_LEDs[] = {LED1, LED2, LED3, LED4, LED5, LED6, LED7, LED8};

volatile bool gb_sw8_pushed = false;
volatile bool gb_sw9_pushed = false;

extern bool g_bIsTestChosen;
extern int g_nSelectedTest;
extern volatile int g_nPostState;

#if defined(__DEBUG_FILE__)
extern FILE *pDebugFile;				/* debug file */
#endif


/*******************************************************************
*   Function:    Init_LEDs
*   Description: This function configures PORTH for LEDs.
*******************************************************************/
void Init_LEDs(void)
{
	/* setting the SRU and route so that flag pins connect to DPI pin buffers */

	/* use flags 4 to 15 only, flags 0 to 3 not available on the DPI */

	SRU(FLAG6_O,DPI_PB06_I);	/* connect Flag6 output to DPI_PB06 input (LED1) */
	SRU(FLAG7_O,DPI_PB13_I);	/* connect Flag7 output to DPI_PB13 input (LED2) */
	SRU(FLAG4_O,DPI_PB14_I);	/* connect Flag4 output to DPI_PB14 input (LED3) */

	SRU(LOW,DAI_PB03_I);	    /* connect Input LOW to LED4 */
	SRU(LOW,DAI_PB04_I);    	/* connect Input LOW to LED5 */
	SRU(LOW,DAI_PB15_I);		/* connect Input LOW to LED6 */
	SRU(LOW,DAI_PB16_I);		/* connect Input LOW to LED7 */
	SRU(LOW,DAI_PB17_I);		/* connect Input LOW to LED8 */

	/* enabling the buffer using the following sequence: high -> output, low -> input */

	SRU(HIGH,DPI_PBEN06_I);		/* LED1 */
	SRU(HIGH,DPI_PBEN13_I);		/* LED2 */
	SRU(HIGH,DPI_PBEN14_I);		/* LED3 */
	SRU(HIGH,PBEN03_I);			/* LED4 */
	SRU(HIGH,PBEN04_I);			/* LED5 */
	SRU(HIGH,PBEN15_I);			/* LED6 */
	SRU(HIGH,PBEN16_I);			/* LED7 */
	SRU(HIGH,PBEN17_I);			/* LED8 */

	/* setting flag pins as outputs */
	sysreg_bit_set( sysreg_FLAGS, (FLG3O|FLG4O|FLG5O|FLG6O|FLG7O|FLG8O) );

	/* clearing flag pins */
	sysreg_bit_clr( sysreg_FLAGS, (FLG3|FLG4|FLG5|FLG6|FLG7|FLG8) );
}

/*******************************************************************
*   Function:    Init_PushButtons
*   Description: This function configures PORTH and interrupts
*					for push buttons.
*******************************************************************/
void Init_PushButtons(void)
{
	/* register the interrupt handlers */
	adi_int_InstallHandler(/*ADI_CID_DAIHI*/ ADI_CID_P0I , (ADI_INT_HANDLER_PTR)SW10_SW11_DAI_handler, NULL, true);
	adi_int_InstallHandler(ADI_CID_IRQ1I, (ADI_INT_HANDLER_PTR)SW8_IRQ1_handler, NULL, true);
	adi_int_InstallHandler(ADI_CID_IRQ2I, (ADI_INT_HANDLER_PTR)SW9_IRQ2_handler, NULL, true);
	/* pin assignments in SRU_PIN3 (Group D) */

	SRU(LOW,DAI_PB19_I);	/* assign pin buffer 19 low so it is an input */
	SRU(LOW,DAI_PB20_I);	/* assign pin buffer 20 low so it is an input */

	/* route MISCA signals in SRU_EXT_MISCA (Group E) */

    SRU(DAI_PB19_O,MISCA1_I);	/* route so that DAI pin buffer 19 connects to MISCA1 */
    SRU(DAI_PB20_O,MISCA2_I);	/* route so that DAI pin buffer 20 connects to MISCA2 */

    /* pin buffer disable in SRU_PINEN0 (Group F) */

    SRU(LOW,PBEN19_I);	/* assign pin 19 low so it is an input */
	SRU(LOW,PBEN20_I);	/* assign pin 20 low so it is an input */

	/* unmask individual interrupts */
	(*pDAI_IRPTL_PRI) = (SRU_EXTMISCA1_INT | SRU_EXTMISCA2_INT);

	/* make sure interrupts latch on the rising edge */
    (*pDAI_IRPTL_RE) = (SRU_EXTMISCA1_INT | SRU_EXTMISCA2_INT);

    /* clear MSEN bit for this test */
	(*pSYSCTL) &= ~(MSEN);

	/* setup the IRQ pins to use the pushbuttons */
	(*pSYSCTL) |= (IRQ1EN | IRQ2EN);

	sysreg_bit_set(sysreg_MODE2, (IRQ1E | IRQ2E) );
	sysreg_bit_clr(sysreg_IRPTL, (IRQ1I | IRQ2I) );

	/* enable IRQ interrupts and make DAI interrupts high priority */
	sysreg_bit_set(sysreg_IMASK, (IRQ1I | IRQ2I | DAIHI) );

	sysreg_bit_set(sysreg_MODE1, IRPTEN ); /* enable global interrupts */
}


/*******************************************************************
*   Function:    LED_Bar
*   Description: Display a blinking LED bar
*******************************************************************/
void LED_Bar(const int iSpeed)
{
	int n,j;

	for( n = 0; n <= g_nNumLEDs-1; n++ )
	{
		ClearSet_LED(g_LEDs[n], LED_TOGGLE);
		Delay(iSpeed);
	}
}


/*******************************************************************
*   Function:    LED_Bar_Reverse
*   Description: Display a blinking LED bar in reverse
*******************************************************************/
void LED_Bar_Reverse(const int iSpeed)
{
	int n,j;

	for( n = g_nNumLEDs-1; n >= 0; n-- )
	{
		ClearSet_LED(g_LEDs[n], LED_TOGGLE);
		Delay(iSpeed);
	}
}

/*******************************************************************
*   Function:    LED_Bar_BacknForth
*   Description: Display a blinking LED bar that goes back and forth.
*******************************************************************/
void LED_Bar_BacknForth(const int iSpeed)
{
	static unsigned int state = 0; /* remember the state */

	if ( (state % 4) == 0 )
	{
		ClearSet_LED(LED1, 1);
		ClearSet_LED(LED2, 0);
		ClearSet_LED(LED3, 0);
	}
	else if ( (state % 4) == 1 )
	{
		ClearSet_LED(LED1, 0);
		ClearSet_LED(LED2, 1);
		ClearSet_LED(LED3, 0);
	}
	else if ( (state % 4) == 2 )
	{
		ClearSet_LED(LED1, 0);
		ClearSet_LED(LED2, 0);
		ClearSet_LED(LED3, 1);
	}
	else
	{
		ClearSet_LED(LED1, 0);
		ClearSet_LED(LED2, 1);
		ClearSet_LED(LED3, 0);
	}

	Delay(iSpeed);
	state++;
}


/*******************************************************************
*   Function:    Strobe_LEDs
*   Description: This strobes the LEDs based on the pattern provide
*				 with a specified delay until the condition is met.
*				 If the condition is NULL, we strobe once and exit.
*******************************************************************/
void Strobe_LEDs(	unsigned int pattern_a, unsigned int pattern_b,
					unsigned int delay, bool *pbDone)
{
	int cnt = 0;
	volatile bool b = false;

	/* use a copy in case pbDone is NULL */
	if ( NULL != pbDone )
		b = *pbDone;

	while (!b)
	{
		for(cnt = delay; cnt > 0; cnt--)
		{
			ClearSet_LED_Bank( pattern_a );
			Delay(cnt);				/* delay decreasing */

			ClearSet_LED_Bank( pattern_b );
			Delay(delay - cnt);		/* delay increasing */
		}

		/* check it again */
		if ( NULL != pbDone )
		{
			b = *pbDone;
			if (b)
				break;		/* if not NULL see if condition has now been met */
		}
		else
			b = true;	/* after one time break out if NULL */

		for(cnt = 0; cnt < delay; cnt++)
		{
			ClearSet_LED_Bank( pattern_a );
			Delay(cnt);				/* delay increasing */

			ClearSet_LED_Bank( pattern_b );
			Delay(delay - cnt);		/* delay decreasing */
		}
	}
}


/*******************************************************************
*   Function:    Blink_LED
*   Description: Blink various LEDs
*******************************************************************/
void Blink_LED(const int iSpeed)
{
	int n, j;

	while( 1 )
	{
		for( n = 0; n <= g_nNumLEDs-1; n++ )
		{
			ClearSet_LED(g_LEDs[n], LED_TOGGLE);
		}

		Delay(iSpeed);
	}
}


/*******************************************************************
*   Function:    ClearSet_LED_Bank
*   Description: Clear or sets the group of LEDs based on specified state.
*******************************************************************/
void ClearSet_LED_Bank(const int iState)
{
	int n;
	int nTempState = iState;


	for( n = 0; n <= g_nNumLEDs-1; n++ )
	{
		ClearSet_LED(g_LEDs[n], (nTempState & 0x3) );
		nTempState >>= 2;
	}
}


/*******************************************************************
*   Function:    ClearSet_LED
*   Description: Clear or set a particular LED (not a group of LEDs).
*******************************************************************/
void ClearSet_LED(const int led, const int bState)
{
	/* if bState is 0 (LED_OFF) we clear the LED,
	   if bState is 1 (LED_ON) we set the LED,
	   else we toggle the LED */

	switch( led )
	{
		case LED1:
		{
			if( LED_OFF == bState ) 	{ sysreg_bit_clr(sysreg_FLAGS, FLG6); }
			else if( LED_ON == bState )	{ sysreg_bit_set(sysreg_FLAGS, FLG6); }
			else	/* toggle */		{ sysreg_bit_tgl(sysreg_FLAGS, FLG6); }
		}
		break;

		case LED2:
		{
			if( LED_OFF == bState ) 	{ sysreg_bit_clr(sysreg_FLAGS, FLG7); }
			else if( LED_ON == bState )	{ sysreg_bit_set(sysreg_FLAGS, FLG7); }
			else	/* toggle */		{ sysreg_bit_tgl(sysreg_FLAGS, FLG7); }
		}
		break;

		case LED3:
		{
			if( LED_OFF == bState ) 	{ sysreg_bit_clr(sysreg_FLAGS, FLG4); }
			else if( LED_ON == bState )	{ sysreg_bit_set(sysreg_FLAGS, FLG4); }
			else	/* toggle */		{ sysreg_bit_tgl(sysreg_FLAGS, FLG4); }
		}
		break;

		case LED4:
		{
			if( LED_OFF == bState ) 	{ SRU(LOW,DAI_PB03_I); }
			else if( LED_ON == bState )	{ SRU(HIGH,DAI_PB03_I); }
			else /* toggle */ 			{ (*pSRU_PIN0) ^= 0x00004000; }
		}
		break;

		case LED5:
		{
			if( LED_OFF == bState ) 	{ SRU(LOW,DAI_PB04_I); }
			else if( LED_ON == bState )	{ SRU(HIGH,DAI_PB04_I); }
			else /* toggle */ 			{ (*pSRU_PIN0) ^= 0x00200000; }
		}
		break;

		case LED6:
		{
			if( LED_OFF == bState ) 	{ SRU(LOW,DAI_PB15_I); }
			else if( LED_ON == bState )	{ SRU(HIGH,DAI_PB15_I); }
			else /* toggle */ 			{ (*pSRU_PIN3) ^= 0x00004000; }
		}
		break;

		case LED7:
		{
			if( LED_OFF == bState ) 	{ SRU(LOW,DAI_PB16_I); }
			else if( LED_ON == bState )	{ SRU(HIGH,DAI_PB16_I); }
			else /* toggle */ 			{ (*pSRU_PIN3) ^= 0x00200000; }
		}
		break;

		case LED8:
		{
			if( LED_OFF == bState ) 	{ SRU(LOW,DAI_PB17_I); }
			else if( LED_ON == bState )	{ SRU(HIGH,DAI_PB17_I); }
			else /* toggle */ 			{ (*pSRU_PIN4) ^= 0x00000001; }
		}
		break;
	}
}

/*******************************************************************
*   Function:    Test_Pushbuttons_LEDs
*   Description: Test to make sure LEDs and PBs are working correctly.
*******************************************************************/
int Test_Pushbuttons_LEDs(void)
{
	int i = 0; int bit = 0;
	bool bSW8 = false, bSW9 = false;	/* flags to see if button was pushed */

	DEBUG_HEADER( "Pushbutton & LED Test" );

	DEBUG_STATEMENT("\nVisually inspect that LEDs toggle when SW8 and SW9 are pressed");
	DEBUG_STATEMENT("\nWaiting for SW8 and SW9 to be pressed...\n");

    bit = *pSYSCTL & MSEN;
	
	/* enable push buttons */
	Init_PushButtons();

    /* clear MSEN bit for this test */
	(*pSYSCTL) &= ~(MSEN);

	/* clear flags in case test has been run before */
	gb_sw8_pushed = false;
	gb_sw9_pushed = false;

	/* clear all the LEDs to start */
	for (i = LED1; i <= LAST_LED; i++)
	{
		ClearSet_LED( i, LED_OFF);
	}

	/* loop until both pbs are pushed */
	while( !bSW8 || !bSW9 )
	{
		/* PB ISR will update flags when a pb is pushed */

		/* was sw8 pushed? */
		if( gb_sw8_pushed )
		{
			/* sw8 was pressed, toggle all LEDs */
			bSW8 = true;
			gb_sw8_pushed = false;

			/* toggle all the LEDs for this test */
			for (i = LED1; i <= LAST_LED; i++)
			{
				ClearSet_LED( i, LED_TOGGLE);
			}

			DEBUG_STATEMENT("\nSW8 pressed...");
		}

		/* was sw9 pushed? */
		if( gb_sw9_pushed )
		{
			/* sw9 was pressed, toggle all LEDs */
			bSW9 = true;
			gb_sw9_pushed = false;

			/* toggle all the LEDs for this test */
			for (i = LED1; i <= LAST_LED; i++)
			{
				ClearSet_LED( i, LED_TOGGLE);
			}

			DEBUG_STATEMENT("\nSW9 pressed...");
		}
	}
	if(bit)
	(*pSYSCTL) |= (MSEN);

	Delay(0x200);

	/* return status */
	DEBUG_STATEMENT("\n\nTest passed");
	return (int)(bSW8 && bSW9);
}


/*******************************************************************
*   Function:    SW8_IRQ1_handler
*   Description: Called each time SW8 is pressed.
*******************************************************************/
void SW8_IRQ1_handler(int sig_int, void *pCBParam)
{
	static int sw8_ctr = 0;

	sw8_ctr += 1;			/* bump counter */
	gb_sw8_pushed = true;	/* set the flag */
}

/*******************************************************************
*   Function:    SW9_IRQ2_handler
*   Description: Called each time SW9 is pressed.
*******************************************************************/
void SW9_IRQ2_handler(int sig_int, void *pCBParam )
{
	static int sw9_ctr = 0;

	sw9_ctr += 1;			/* bump counter */
	gb_sw9_pushed = true;	/* set the flag */
}


/*******************************************************************
*   Function:    SW10_SW11_DAI_handler
*   Description: Called each time SW10 or SW11 is pressed.
*******************************************************************/
void SW10_SW11_DAI_handler(int sig_int, void *pCBParam)
{
	int iTest;		/* flag to indicate which button was pushed */


	/* read the latch register which also clears the latched interrupt */
	iTest = (*pDAI_IRPTL_H);

    /* if we are running tests already this may be the PB test so just return */
	if ( STATE_SELECTED == g_nPostState )
	{
		return;
	}

	/* update the state machine */
	if ( (g_nPostState >= STATE_START) && (g_nPostState <= STATE_4) )
	{
		/* if sw10 */
		if( SRU_EXTMISCA1_INT == iTest )
		{
			/* pushing a 0 */

			g_nPostState += 1;				/* update the state */
			g_nSelectedTest <<= 1;			/* shift it up */
			DEBUG_STATEMENT("0");
		}

		/* else if sw11 */
		else if( SRU_EXTMISCA2_INT == iTest )
		{
			/* pushing a 1 */

			g_nPostState += 1;				/* update the state */
			g_nSelectedTest <<= 1;			/* shift it up */
			g_nSelectedTest |= 0x1;			/* OR in a 1 */
			DEBUG_STATEMENT("1");
		}

		if ( (g_nPostState >= STATE_1) && (g_nPostState <= STATE_4) )
		{
			DEBUG_STATEMENT("-");
		}
	}

	/* when we get to this state a test has been sucessfully entered */
	if ( STATE_SELECTED == g_nPostState )
	{
		g_bIsTestChosen = true;
	}
}
