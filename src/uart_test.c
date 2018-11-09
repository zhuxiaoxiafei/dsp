/*******************************************************************
Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software
you agree to the terms of the associated Analog Devices License Agreement.

Project Name:  	Power_On_Self_Test

Hardware:		ADSP-21479/21489 EZ-Board

Description:	This file tests the UART interface on the EZ-Board.
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
#include <sru.h>
#include <signal.h>
#include <sysreg.h>
#include "post_debug.h"

/*******************************************************************
*  global variables and defines
*******************************************************************/
#define BAUD_RATE_115200 	(0x15A2 >> 7)
#define NUM_TEST_CHARS		(1*1024)


/* if we are using the UART to print debug info, define the following */
#ifdef __DEBUG_UART__
#define UART_DEBUG_BUFFER_LINE_SIZE 256
char UART_DEBUG_BUFFER[UART_DEBUG_BUFFER_LINE_SIZE];
#endif

#if defined(__DEBUG_FILE__)
extern FILE *pDebugFile;				/* debug file */
#endif

/*******************************************************************
*  function prototypes
*******************************************************************/
void Init_UART(void);
int PutChar(const char c);
int GetChar(char *const c);

/* if we are using the UART to print debug info, define the following */
#ifdef __DEBUG_UART__
int UART_DEBUG_PRINT(void);
#endif


/*******************************************************************
*   Function:    Init_UART
*   Description: Initialize UART with the appropriate values
*******************************************************************/

void Init_UART(void)
{
	volatile int temp;
	int picr2 = 0x0;

	/* maps the UART0 receive interrupt to P14 using the programmable interrupt controller */

	picr2 = *pPICR2;		/* get PICR2 */
	picr2 &= (~(0x1f<<10));	/* clear P14 bits */
	picr2 |= (0x13<<10);	/* set UART0RX */
	(*pPICR2) = picr2;		/* put it back */

	sysreg_bit_set(sysreg_IMASK, P14I ); /* unmask UART RX interrupt */

/*	(*pUART0IER) = UARTRBFIE;*/		/* enable UART RX interrupt */

	SRU2(UART0_TX_O,DPI_PB09_I);	/* UART TX signal is connected to DPI pin 9 */
	SRU2(HIGH,DPI_PBEN09_I);
	SRU2(DPI_PB10_O,UART0_RX_I);	/* connect the pin buffer output signal to the UART0 RX */
	SRU2(LOW,DPI_PB10_I);
	SRU2(LOW,DPI_PBEN10_I);			/* disable DPI pin10 as input */

	(*pUART0LCR) = UARTDLAB;		/* enables access to divisor register to set baud rate */
#ifdef __ADSP21469__
	(*pUART0DLL) = 0xdc;
	(*pUART0DLH) = 0x2;				/* 0x2dc = 732 for divisor value gives a baud rate of 19200 at 450 Mhz core clock */
#elif (__ADSP21479__)
	(*pUART0DLL) = 0xb0;
	(*pUART0DLH) = 0x1;				/* 0x1B0 = 432 for divisor value gives a baud rate of 19200 at 266 Mhz core clock */
#elif (__ADSP21489__)
	(*pUART0DLL) = 0x8b;
	(*pUART0DLH) = 0x2;				/* 0x28b = 651 for divisor value gives a baud rate of 19200 at 400 Mhz core clock */
#endif
	(*pUART0LCR) = (UARTWLS8 | UARTPEN | UARTSTB);   /* 8 bit word, odd parity, 2 stop bits */

	(*pUART0RXCTL) = UARTEN;        /* enable UART0 RX */
	(*pUART0TXCTL) = UARTEN;        /* enable UART0 TX */
}


/*******************************************************************
*   Function:    PutChar
*   Description: Writes a character to the UART.
*******************************************************************/

int PutChar(const char cVal)
{
	int nStatus = 0;
	unsigned int count = 0;

	do
	{
		if( (*pUART0LSR & UARTTHRE) )
		{
			*pUART0THR = cVal;
			nStatus = 1;
			break;
		}

		count++;

	} while( count < 0x100000 );

	return nStatus;
}


/*******************************************************************
*   Function:    GetChar
*   Description: Reads a character from the UART.
*******************************************************************/

int GetChar(char *const cVal)
{
	int nStatus = 0;
	unsigned int count = 0x0;

	do{
		if( 1 /*UARTDR == (*pUART0LSR & UARTDR)*/ )
		{
			*cVal = (char)*pUART0RBR;
			nStatus = 1;
			break;
		}

		/*count++;*/

	}while( count < 0x100000 );

	return nStatus;
}


/*******************************************************************
*   Function:    Test_UART
*   Description: Performs a test by writing characters to and then
					reading characters from the UART and comparing them.
*******************************************************************/

int Test_UART(void)
{
	int n, i;
	char cTxChar;
	char cRxChar;

	DEBUG_HEADER( "UART Test" );

	/* do this before calling Init_UART() and don't print anything else until
		test is done otherwise we will impact the loopback test if we are
		using the UART for debug info */
	DEBUG_PRINT( "\nLooping %d characters through the UART", NUM_TEST_CHARS );

	Init_UART();

	for(n = 0; n < NUM_TEST_CHARS; n++)
	{
		/* next test char */
		cTxChar = (n & 0xFF);

		/* write a char */
		if( 0 == PutChar(cTxChar) )
		{
			DEBUG_STATEMENT( "\nTest failed" );
			return 0;
		}

		/* wait between writing and reading to give time for data to appear */
		for (i = 0; i < 500000; i++)
		{
			asm("nop;");
		}

		/* read a char */
		if( 0 == GetChar( &cRxChar ) )
		{
			DEBUG_STATEMENT( "\nTest failed" );
			return 0;
		}

		/* it should match */
		if( cTxChar != cRxChar )
		{
			DEBUG_STATEMENT( "\nTest failed" );
			return 0;
		}
	}

	DEBUG_STATEMENT( "\n\n\nTest passed" );
	return 1;
}


#ifdef __DEBUG_UART__
/*******************************************************************
*   Function:    UART_DEBUG_PRINT
*   Description: Prints debug info over the UART using a predefined
*				 buffer.
*******************************************************************/
int UART_DEBUG_PRINT(void)
{
	unsigned int i = 0;		/* index */
	char temp;				/* temp char */


	/* loop through the debug buffer until the end, a NULL, or an error */
	for ( i = 0; i < UART_DEBUG_BUFFER_LINE_SIZE; i++)
	{
		temp = UART_DEBUG_BUFFER[i];

		/* if not NULL then print it */
		if (temp)
		{
			if( 0 == PutChar(temp) )
			{
				/* if error was detected then quit */
				return 0;
			}

			/* if it was a newline we need to add a carriage return */
			if ( 0x0a == temp )
			{
				if( 0 == PutChar(0x0d) )
				{
					/* if error was detected then quit */
					return 0;
				}
			}
		}
		else
		{
			/* else NULL was found */
			return 1;
		}
	}

	return 1;
}
#endif
