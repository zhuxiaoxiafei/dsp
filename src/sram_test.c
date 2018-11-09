/*******************************************************************
Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software
you agree to the terms of the associated Analog Devices License Agreement.

Project Name:  	Power_On_Self_Test

Hardware:		ADSP-21479/21489 EZ-Board

Description:	This file tests the SRAM interface on the EZ-Board.
				
Purpose:	Perform a POST SRAM test on the 21486 EZ-Kit Lite	

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


#if defined(__DEBUG_FILE__)
extern FILE *pDebugFile;				/* debug file */
#endif

#define SRAM_START  		0x0C000000							/* start address of SRAM */
#define SRAM_END			0x0C07ffff							/* end of SRAM (in 32 bit words) */
#define SRAM_SIZE_IN_BYTES	((SRAM_END + 1 -\
							SRAM_START) * 4)					/* SRAM is 2 MB (8 bits wide) */
#define BYTES_PER_DMA		0x800								/* 2 KB in each DMA */
#define NUMBER_OF_DMAS		SRAM_SIZE_IN_BYTES/BYTES_PER_DMA	/* number of DMAs to test all SRAM */
#define NUMBER_OF_DWORDS	BYTES_PER_DMA/4 					/* number of dwords in DMA */
#define DMA_XMODIFY 		2									/* stride using 16-bit elements */
#define DMA_XCOUNT 			NUMBER_OF_DWORDS*DMA_XMODIFY		/* number of 16-bit elements */

/* contains the data we send out to SRAM */
extern unsigned int out_buf[sizeof(unsigned int)*NUMBER_OF_DWORDS];
/* stores the data we receive from SRAM */
extern unsigned int in_buf[sizeof(unsigned int)*NUMBER_OF_DWORDS];
/* stores a backup of the SRAM data we'll overwrite */
extern unsigned int backup_buf[sizeof(unsigned int)*NUMBER_OF_DWORDS];

unsigned long curr_SRAM_address = 0;
int direction_flag_sram = 0;
int dma_done_sram = 0;
bool bFailedVerify_sram = false;
int nNumDMAWrites_sram = 0;
int nNumDMAReads_sram = 0;

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
void Init_DMA_SRAM(void);
void Enable_DMA_Writes_SRAM(void);
void Enable_DMA_Reads_SRAM(void);
void DMA_Writes_ISR_SRAM(int, void *);
void DMA_Reads_ISR_SRAM(int, void *);
void Init_SRAM(void);

/*******************************************************************
*   Function:    Enable_DMA_Writes
*   Description: enable DMA writes to external memory
*******************************************************************/
void Enable_DMA_Writes_SRAM(void)
{
	/* register the interrut handler */
//(BDR)	interrupt(SIG_P9, DMA_Writes_ISR_SRAM);
	adi_int_InstallHandler(ADI_CID_P9I, (ADI_INT_HANDLER_PTR)DMA_Writes_ISR_SRAM, NULL, true);

	*pDMAC0 = 0x0;						/* good idea to clear it out first */
	*pIIEP0 = (unsigned int)out_buf;	/* internal address */
	*pIMEP0 = 1;						/* internal modifier */
	*pCEP0 = NUMBER_OF_DWORDS;			/* DMA count */
	*pEIEP0 = curr_SRAM_address;		/* external address */
	*pEMEP0 = 1;						/* external modifier */
	*pDMAC0 = DEN | TRAN | DFLSH;		/* TRAN set means external write */
}


/*******************************************************************
*   Function:    Enable_DMA_Reads
*   Description: enable DMA reads from external memory
*******************************************************************/
void Enable_DMA_Reads_SRAM(void)
{
	/* register the interrut handler */
//(BDR)	interrupt(SIG_P9, DMA_Reads_ISR_SRAM);
	adi_int_InstallHandler(ADI_CID_P9I, (ADI_INT_HANDLER_PTR)DMA_Reads_ISR_SRAM, NULL, true);

	*pDMAC0 = 0x0;					/* good idea to clear it out first */
	*pIIEP0 = (unsigned int)in_buf;	/* internal address */
	*pIMEP0 = 1;					/* internal modifier */
	*pCEP0 = NUMBER_OF_DWORDS;		/* DMA count */
	*pEIEP0 = curr_SRAM_address;	/* external address */
	*pEMEP0 = 1;					/* external modifier */
	*pDMAC0 = DEN | DFLSH;			/* TRAN clear means external read */
}


/*******************************************************************
*   Function:    Init_DMA
*   Description: initialize DMA registers
*******************************************************************/
void Init_DMA_SRAM(void)
{
	sysreg_bit_set(sysreg_IRPTL, P9IMSK );	/* enable the external port DMA0 interrupt */
	sysreg_bit_set(sysreg_MODE1, IRPTEN );	/* enable global interrupts */

	*pDMAC0 = 0x0;							/* good idea to clear it out first */
}
/*******************************************************************
*   Function:    Test_SRAM
*   Description: Tests the SRAM interface on the EZ-Board.
*******************************************************************/
int Test_SRAM(void)
{
	volatile unsigned int *pSrc16;			/* ptr */
	int nIndex = 0;							/* temp index */
	int bSuccess = 1; 						/* 1 indicates a pass, anything else is a fail */
	static int nDataPattern = INCREMENTING;	/* we use different data patterns each time */

	DEBUG_HEADER( "SRAM Test" );

	DEBUG_PRINT("\n\nTesting %d bytes,\naddress range: 0x%x - 0x%x",
					SRAM_SIZE_IN_BYTES, SRAM_START, SRAM_END);

	/* reset some global regs */
	dma_done_sram = 0;
	nNumDMAWrites_sram = 0;
	nNumDMAReads_sram = 0;
	curr_SRAM_address = SRAM_START;
	direction_flag_sram = 0;

	Init_SRAM();						/* setup SRAM */
	Init_DMA_SRAM();					/* setup DMA */
	srand(*(volatile int*)SRAM_START);	/* seed with a random value */

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

	/* backup the first block of SRAM we'll overwrite using core accesses, subsequent
	   blocks will get backed up in the ISR */
	for(nIndex = 0, pSrc16 = (unsigned int *)SRAM_START; nIndex < NUMBER_OF_DWORDS; pSrc16++, nIndex++ )
	{
		backup_buf[nIndex] = *pSrc16;
	}

	/* enable DMA writes to external memory */
	Enable_DMA_Writes_SRAM();

	/* wait for the test to finish or for a failure */
	while( !dma_done_sram && !bFailedVerify_sram )
	{
		asm("nop;");
		asm("nop;");
		asm("nop;");
	}

	/* setup to ignore this interrupt */
//(BDR)	interrupt(SIG_P9, SIG_IGN);
	adi_int_EnableInt(ADI_CID_P9I, false);
	adi_int_UninstallHandler(ADI_CID_P9I);

	if ( bFailedVerify_sram )
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
void DMA_Writes_ISR_SRAM(int sig_int, void *pCBParam)
{
    volatile unsigned int *pDst16 = (unsigned int *)curr_SRAM_address;
	int nIndex = 0;


	/* increment the number of writes so far */
	nNumDMAWrites_sram += 1;

	if( nNumDMAWrites_sram <= NUMBER_OF_DMAS )
	{
		/* check each DWORD */
	    while( nIndex < NUMBER_OF_DWORDS )
	    {
	        /* verify the DMA writes */
	        if( ((pDst16[nIndex])) != ((out_buf[nIndex])) )
	        {
	        	DEBUG_PRINT( "\n\nFailure at address 0x%x: expected 0x%x, received 0x%x",
	        					curr_SRAM_address + nIndex,
	        					out_buf[nIndex],
	        					pDst16[nIndex] );
	            bFailedVerify_sram = true;
	            break;
	        }

	        nIndex++;
	    }

	    /* enable DMA reads from external memory */
		adi_int_EnableInt(ADI_CID_P9I, false);
		adi_int_UninstallHandler(ADI_CID_P9I);

	    /* enable DMA reads from external memory */
		Enable_DMA_Reads_SRAM();
	}
	else
	{

	}
}


/*******************************************************************
*   Function:    DMA_Reads_ISR
*   Description: DMA read ISR.
*******************************************************************/
void DMA_Reads_ISR_SRAM(int sig_int, void *pCBParam)
{
    int nIndex = 0;
    volatile unsigned int *pSrc16 = (unsigned int *)curr_SRAM_address;


	/* increment the number of reads so far */
	nNumDMAReads_sram += 1;

	/* if we have more DMAs to perform */
	if( nNumDMAReads_sram <= NUMBER_OF_DMAS )
	{
		/* check each DWORD */
	    while( nIndex < NUMBER_OF_DWORDS )
	    {
	    	/* verify the DMA reads */
	        if( ((in_buf[nIndex])) != ((out_buf[nIndex])) )
	        {
	        	DEBUG_PRINT( "\n\nFailure at address 0x%x: expected 0x%x, received 0x%x",
	        					curr_SRAM_address + nIndex,
	        					out_buf[nIndex],
	        					in_buf[nIndex] );
	            bFailedVerify_sram = true;
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
		curr_SRAM_address += BYTES_PER_DMA/4;

		/* enable DMA reads from external memory */
		adi_int_EnableInt(ADI_CID_P9I, false);
		adi_int_UninstallHandler(ADI_CID_P9I);

	    /* enable DMA writes to external memory */
		Enable_DMA_Writes_SRAM();

		/* last one, disable DMA and set flag */
		if (nNumDMAReads_sram == NUMBER_OF_DMAS)
		{
			*pDMAC0 = 0x0;				/* good idea to clear it out */
			dma_done_sram = 1;
		}
	}
}
