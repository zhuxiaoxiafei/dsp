/*******************************************************************
Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software
you agree to the terms of the associated Analog Devices License Agreement.

Project Name:  	Power_On_Self_Test

Hardware:		ADSP-21479/21489 EZ-Board

Description:	This file tests the SDRAM interface on the EZ-Board.
				This EZ-Board contains 16Mb x 16 (32MB) of SDRAM which
				resides in the range from 0x200000 to 0x9fffff.

				The test DMAs blocks of 2KB at a time between internal
				memory and SDRAM, then compares the data.  It also takes
				care to back up the SDRAM blocks before overwriting them
				using core accesses to preserve the SDRAM contents.

				Additionally each time the test is called, the data
				pattern is changed between incrementing, random, all 0x00,
				all 0x55, all 0xaa, and all 0xff.

				The test makes use of these 2KB data blocks:


					INTERNAL						SDRAM


				    					(7) core
				+----------+ -----------------------+
				|backup_buf| <-----+				|
				+----------+ 	   |				|
				                   |				|
				                   | (2) core	   \|/
				+----------+ 	   +----------- +----------+
    (1) fill ->	|  out_buf | --+   				|  block_N |
				+----------+   +-------------->	+----------+
									(3) DMA			|
							   		(4) compare   	|
				+----------+ 						|
				|  in_buf  | <----------------------+
				+----------+       (5) DMA
								   (6) compare


				The test performs these repeating steps:

				1) fills out_buf with a known pattern
				2) copies block_N to backup_buf using core acceses
				3) DMAs out_buf to block_N
				4) compares out_buf with block_N
				5) DMAs block N to in_buf
				6) compares in_buf to block_N
				7) restores backup_buf to block_N using core acceses

				Note, the contents of this file cannot live in SDRAM or
				else unexpected results may occur.  This could be taken care
				of in the LDF by making sure the input sections for sdram_test
				object are placed in L1.  Also needed to be placed in L1
				are any other objects used by this test which may include
				uart_test (if UART debug is used), and other libs.

*******************************************************************/

/*******************************************************************
*  include files
*******************************************************************/
#ifdef __ADSP21489__
#include <cdef21489.h>
#include <def21489.h>
#else
#include <cdef21479.h>
#include <def21479.h>
#endif
#include <stdlib.h>
#include <sru.h>
#include <signal.h>
#include <sysreg.h>

#include "post_common.h"
#include "post_debug.h"
#include <services\int\adi_int.h>

/*******************************************************************
*  global variables and defines
*******************************************************************/
#define SDRAM_START  		0x00200000							/* start address of SDRAM */
#define SDRAM_END			0x009fffff							/* end of SDRAM (in 32 bit words) */
#define SDRAM_SIZE_IN_BYTES	((SDRAM_END + 1 -\
							 SDRAM_START) * 4 )					/* SDRAM is 32 MB (8 bits wide) */
#define BYTES_PER_DMA		0x800								/* 2 KB in each DMA */
#define NUMBER_OF_DMAS		SDRAM_SIZE_IN_BYTES/BYTES_PER_DMA	/* number of DMAs to test all SDRAM */
#define NUMBER_OF_DWORDS	BYTES_PER_DMA/4 					/* number of dwords in DMA */
#define DMA_XMODIFY 		2									/* stride using 16-bit elements */
#define DMA_XCOUNT 			NUMBER_OF_DWORDS*DMA_XMODIFY		/* number of 16-bit elements */

/* contains the data we send out to SDRAM */
extern unsigned int out_buf[sizeof(unsigned int)*NUMBER_OF_DWORDS];
/* stores the data we receive from SDRAM */
extern unsigned int in_buf[sizeof(unsigned int)*NUMBER_OF_DWORDS];
/* stores a backup of the SDRAM data we'll overwrite */
extern unsigned int backup_buf[sizeof(unsigned int)*NUMBER_OF_DWORDS];

unsigned long curr_SDRAM_address = 0;
int direction_flag = 0;
int dma_done = 0;
bool bFailedVerify = false;
int nNumDMAWrites = 0;
int nNumDMAReads = 0;

enum			/* data pattern */
{
	INCREMENTING,
	RANDOM,
	ALL_0S,
	ALL_5S,
	ALL_AS,
	ALL_FS,
};

#if defined(__DEBUG_FILE__)
extern FILE *pDebugFile;				/* debug file */
#endif

/*******************************************************************
*  function prototypes
*******************************************************************/
void Init_DMA(void);
void Enable_DMA_Writes(void);
void Enable_DMA_Reads(void);
void DMA_Writes_ISR(int, void *);
void DMA_Reads_ISR(int, void *);


/*******************************************************************
*   Function:    Enable_DMA_Writes
*   Description: enable DMA writes to external memory
*******************************************************************/
void Enable_DMA_Writes(void)
{
	/* register the interrut handler */
//(BDR)	interrupt(SIG_P9, DMA_Writes_ISR);
	adi_int_InstallHandler(ADI_CID_P9I, (ADI_INT_HANDLER_PTR)DMA_Writes_ISR, NULL, true);

	*pDMAC0 = 0x0;						/* good idea to clear it out first */
	*pIIEP0 = (unsigned int)out_buf;	/* internal address */
	*pIMEP0 = 1;						/* internal modifier */
	*pCEP0 = NUMBER_OF_DWORDS;			/* DMA count */
	*pEIEP0 = curr_SDRAM_address;		/* external address */
	*pEMEP0 = 1;						/* external modifier */
	*pDMAC0 = DEN | TRAN | DFLSH;		/* TRAN set means external write */
}


/*******************************************************************
*   Function:    Enable_DMA_Reads
*   Description: enable DMA reads from external memory
*******************************************************************/
void Enable_DMA_Reads(void)
{
	/* register the interrut handler */
//(BDR)	interrupt(SIG_P9, DMA_Reads_ISR);
	adi_int_InstallHandler(ADI_CID_P9I, (ADI_INT_HANDLER_PTR)DMA_Reads_ISR, NULL, true);

	*pDMAC0 = 0x0;					/* good idea to clear it out first */
	*pIIEP0 = (unsigned int)in_buf;	/* internal address */
	*pIMEP0 = 1;					/* internal modifier */
	*pCEP0 = NUMBER_OF_DWORDS;		/* DMA count */
	*pEIEP0 = curr_SDRAM_address;	/* external address */
	*pEMEP0 = 1;					/* external modifier */
	*pDMAC0 = DEN | DFLSH;			/* TRAN clear means external read */
}


/*******************************************************************
*   Function:    Init_DMA
*   Description: initialize DMA registers
*******************************************************************/
void Init_DMA(void)
{
	sysreg_bit_set(sysreg_IRPTL, P9IMSK );	/* enable the external port DMA0 interrupt */
	sysreg_bit_set(sysreg_MODE1, IRPTEN );	/* enable global interrupts */

	*pDMAC0 = 0x0;							/* good idea to clear it out first */
}


/*******************************************************************
*   Function:    Test_SDRAM
*   Description: Tests the SDRAM interface on the EZ-Board.
*******************************************************************/
int Test_SDRAM(void)
{
	volatile unsigned int *pSrc16;			/* ptr */
	int nIndex = 0;							/* temp index */
	int bSuccess = 1; 						/* 1 indicates a pass, anything else is a fail */
	static int nDataPattern = INCREMENTING;	/* we use different data patterns each time */

	DEBUG_HEADER( "SDRAM Test" );

	DEBUG_PRINT("\n\nTesting %d bytes,\naddress range: 0x%x - 0x%x",
					SDRAM_SIZE_IN_BYTES, SDRAM_START, SDRAM_END);

	/* reset some global regs */
	dma_done = 0;
	nNumDMAWrites = 0;
	nNumDMAReads = 0;
	curr_SDRAM_address = SDRAM_START;
	direction_flag = 0;

	Init_SDRAM();						/* setup SDRAM */
	Init_DMA();							/* setup DMA */
	srand(*(volatile int*)SDRAM_START);	/* seed with a random value */

	/* fill our out_buf with some data */
	for(nIndex = 0; nIndex < NUMBER_OF_DWORDS; nIndex++ )
	{
		switch (nDataPattern)
		{
			case INCREMENTING:
				out_buf[nIndex] = nIndex;
				break;
			case RANDOM:
				out_buf[nIndex] = rand();
				break;
			case ALL_0S:
				out_buf[nIndex] = 0x00000000;
				break;
			case ALL_5S:
				out_buf[nIndex] = 0x55555555;
				break;
			case ALL_AS:
				out_buf[nIndex] = 0xaaaaaaaa;
				break;
			case ALL_FS:
				out_buf[nIndex] = 0xffffffff;
				break;
		}
	}

	/* print some debug info */
	switch (nDataPattern)
	{
		case INCREMENTING:
			DEBUG_STATEMENT("\nTest pattern is \"incrementing\"");
			break;
		case RANDOM:
			DEBUG_STATEMENT("\nTest pattern is \"random\"");
			break;
		case ALL_0S:
			DEBUG_STATEMENT("\nTest pattern is \"all 0x00\"");
			break;
		case ALL_5S:
			DEBUG_STATEMENT("\nTest pattern is \"all 0x55\"");
			break;
		case ALL_AS:
			DEBUG_STATEMENT("\nTest pattern is \"all 0xaa\"");
			break;
		case ALL_FS:
			DEBUG_STATEMENT("\nTest pattern is \"all 0xff\"");
			break;
	}

	/* update type of data for next run */
	nDataPattern++;
	if (nDataPattern > ALL_FS)
		nDataPattern = RANDOM;

	/* backup the first block of SDRAM we'll overwrite using core accesses, subsequent
	   blocks will get backed up in the ISR */
	for(nIndex = 0, pSrc16 = (unsigned int *)SDRAM_START; nIndex < NUMBER_OF_DWORDS; pSrc16++, nIndex++ )
	{
		backup_buf[nIndex] = *pSrc16;
	}

	/* enable DMA writes to external memory */
	Enable_DMA_Writes();

	/* wait for the test to finish or for a failure */
	while( !dma_done && !bFailedVerify )
	{
		asm("nop;");
		asm("nop;");
		asm("nop;");
	}

	/* setup to ignore this interrupt */
//(BDR)	interrupt(SIG_P9, SIG_IGN);
	adi_int_EnableInt(ADI_CID_P9I, false);
	adi_int_UninstallHandler(ADI_CID_P9I);

	if ( bFailedVerify )
	{
		DEBUG_STATEMENT("\n\nTest failed");
		return 0;			/* return failure */
	}
	else
	{
		DEBUG_STATEMENT("\n\nTest passed");
		return 1;			/* return pass */
	}
}


/*******************************************************************
*   Function:    DMA_Writes_ISR
*   Description: DMA write ISR.
*******************************************************************/
void DMA_Writes_ISR(int sig_int, void *pCBParam)
{
    volatile unsigned int *pDst16 = (unsigned int *)curr_SDRAM_address;
	int nIndex = 0;


	/* increment the number of writes so far */
	nNumDMAWrites += 1;

	if( nNumDMAWrites <= NUMBER_OF_DMAS )
	{
		/* check each DWORD */
	    while( nIndex < NUMBER_OF_DWORDS )
	    {
	        /* verify the DMA writes */
	        if( ((pDst16[nIndex])) != ((out_buf[nIndex])) )
	        {
	        	DEBUG_PRINT( "\n\nFailure at address 0x%x: expected 0x%x, received 0x%x",
	        					curr_SDRAM_address + nIndex,
	        					out_buf[nIndex],
	        					pDst16[nIndex] );
	            bFailedVerify = true;
	            break;
	        }

	        nIndex++;
	    }

		/* setup to ignore this interrupt */
		adi_int_EnableInt(ADI_CID_P9I, false);
		adi_int_UninstallHandler(ADI_CID_P9I);

	    /* enable DMA reads from external memory */
		Enable_DMA_Reads();
	}
	else
	{

	}
}


/*******************************************************************
*   Function:    DMA_Reads_ISR
*   Description: DMA read ISR.
*******************************************************************/
void DMA_Reads_ISR(int sig_int, void *pCBParam)
{
    int nIndex = 0;
    volatile unsigned int *pSrc16 = (unsigned int *)curr_SDRAM_address;


	/* increment the number of reads so far */
	nNumDMAReads += 1;

	/* if we have more DMAs to perform */
	if( nNumDMAReads <= NUMBER_OF_DMAS )
	{
		/* check each DWORD */
	    while( nIndex < NUMBER_OF_DWORDS )
	    {
	    	/* verify the DMA reads */
	        if( ((in_buf[nIndex])) != ((out_buf[nIndex])) )
	        {
	        	DEBUG_PRINT( "\n\nFailure at address 0x%x: expected 0x%x, received 0x%x",
	        					curr_SDRAM_address + nIndex,
	        					out_buf[nIndex],
	        					in_buf[nIndex] );
	            bFailedVerify = true;
	            break;
	        }
	        nIndex++;
	    }

	    /* restore the last block of SDRAM we overwrote using core accesses */
		for(nIndex = 0; nIndex < NUMBER_OF_DWORDS; pSrc16++, nIndex++ )
		{
			*pSrc16 = backup_buf[nIndex];
		}

	    /* backup the next block of SDRAM we'll overwrite using core accesses */
		for(nIndex = 0; nIndex < NUMBER_OF_DWORDS; pSrc16++, nIndex++ )
		{
			backup_buf[nIndex] = *pSrc16;
		}

		/* setup for next block (divide by 4 for SHARC 32-bit addressing) */
		curr_SDRAM_address += BYTES_PER_DMA/4;

		/* setup to ignore this interrupt */
		adi_int_EnableInt(ADI_CID_P9I, false);
		adi_int_UninstallHandler(ADI_CID_P9I);

	    /* enable DMA writes to external memory */
		Enable_DMA_Writes();

		/* last one, disable DMA and set flag */
		if (nNumDMAReads == NUMBER_OF_DMAS)
		{
			*pDMAC0 = 0x0;				/* good idea to clear it out */
			dma_done = 1;
		}
	}
}
