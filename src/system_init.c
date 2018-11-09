/*******************************************************************
Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software
you agree to the terms of the associated Analog Devices License Agreement.

Project Name:  	Power_On_Self_Test

Hardware:		ADSP-21479/21489 EZ-Board

Description:	This examples initializes the EZ-Board.
*******************************************************************/

#ifdef __ADSP21489__
#include <cdef21489.h>
#include <def21489.h>
#elif __ADSP21479__
#include <cdef21479.h>
#include <def21479.h>
#endif
#include "post_debug.h"
#include "sysreg.h"

#ifndef BW16
#define BW16	BIT_1
#endif

/* contains the data we send out to S(D)RAM */
section("seg_dmda") unsigned int out_buf[sizeof(unsigned int)*800];
/* stores the data we receive from S(D)RAM */
section("seg_dmda") unsigned int in_buf[sizeof(unsigned int)*800];
/* stores a backup of the S(D)RAM data we'll overwrite */
section("seg_dmda") unsigned int backup_buf[sizeof(unsigned int)*800];

#if defined(__DEBUG_FILE__)
extern FILE *pDebugFile;				/* debug file */
#endif


#ifdef __ADSP21489__
	#define PLL_MULT			PLLM16
	#define RDIV				(0x4D8)
#else	/* 21479 */
	#define PLL_MULT			PLLM16	
	#define RDIV				(0x406)
#endif

/*******************************************************************
*   Function:    Init_PLL
*   Description: initializes the PLL registers
*******************************************************************/
void Init_PLL(void)
{
	int temp,i;

	// Step 1 - change the PLLD to 4
	temp=*pPMCTL;
	temp&=~PLLD16 ;
	temp|=(PLLD4 | DIVEN) ;
	*pPMCTL = temp;

	// Step 2 - wait for dividers to stabilize
	for(i=0;i<16;i++);

	// Step 3 - set INDIV bit now to bring down the VCO speed and enter the bypass mode
	temp&=~DIVEN;
	temp|=(INDIV | PLLBP);
	*pPMCTL = temp;

	// Step 4 - wait for the PLL to lock
	for(i=0;i<4096;i++)

	// Step 5 - come out of the bypass mode
	temp=*pPMCTL;
	temp&=~PLLBP;
	*pPMCTL = temp;

	// Step 6 - wait for dividers to stabilize
	for(i=0;i<16;i++);

	// Step 7 - set the required PLLM and INDIV values here  and enter the bypass mode
	temp = *pPMCTL;
	temp&=~ (INDIV | PLLM63);
	temp|= (PLL_MULT| PLLBP);
	*pPMCTL = temp;

	// Step 8 - wait for the PLL to lock
	for(i=0;i<4096;i++);

	// Step 9 - come out of the bypass mode
	temp = *pPMCTL;
	temp&=~PLLBP;
	*pPMCTL=temp;

	// Step 10 - wait for dividers to stabilize
	for(i=0;i<16;i++);

	// Step 11 - set the required values of PLLD(=2) and SDCKR (=2.5 for ADSP-21489 and 2 for ADSP-21479) here
	temp=*pPMCTL;
	temp&=~(PLLD16 | 0x1C0000 );
	#ifdef __ADSP21489__
	temp|= (SDCKR2_5 | PLLD2 | DIVEN);
	#elif __ADSP21479__
	temp|= (SDCKR2 | PLLD2 | DIVEN);
	#endif
	*pPMCTL=temp;

	// Step 12 - wait for the dividers to stabilize
	for(i=0;i<16;i++);
}


/*******************************************************************
*   Function:    Init_SDRAM
*   Description: initializes the SDRAM registers
*******************************************************************/
void Init_SDRAM(void)
{
	int nValue = 0;
	
    *pSYSCTL |= MSEN;

    // Mapping Bank 0 to SDRAM
    *pEPCTL |= B0SD;
    *pEPCTL &= (~(B1SD|B2SD|B3SD));
    
    nValue = RDIV;
    nValue |= SDROPT | BIT_17;	// Enabling SDRAM read optimization
								// Setting the Modify to 1
	*pSDRRC = nValue;

	// Programming SDRAM control register	
	nValue = 0;
#ifdef __ADSP21489__
	nValue |= (SDCL3|SDTRAS7|SDTRP3|SDCAW9|SDPSS|SDTWR2|SDTRCD3|SDRAW13|X16DE);
#elif __ADSP21479__
	nValue |= (SDCL3|SDTRAS6|SDTRP3|SDCAW9|SDPSS|SDTWR2|SDTRCD3|SDRAW13|X16DE);
#endif	
	*pSDCTL = nValue;
}


/*******************************************************************
*   Function:    Init_SRAM
*   Description: initializes the SRAM registers
*******************************************************************/
void Init_SRAM(void)
{
	int nValue = 0;
	
    *pSYSCTL |= MSEN;

}

/*******************************************************************
*   Function:    Init_AMI
*   Description: initializes the AMI interface
*******************************************************************/
void Init_AMI(void)
{
	int sysctl = 0x0;
	int amictl = 0x0;

	sysctl = *pSYSCTL;
	sysctl |= EPDATA32;
	*pSYSCTL = sysctl;

	// Flash is on Bank1
	// Programming maximum wait states
	amictl = (AMIEN | BW8 | WS31);
	*pAMICTL1 = amictl;

	// SRAM is connected on Bank 3
	// SRAM part used - IS61WV102416BLL
	// As per datasheet access time is 10 ns, 8ns
	// Programming waitstates = 2
	amictl = (AMIEN | BW16 | WS2);
	*pAMICTL3 = amictl;
}


/*******************************************************************
*   Function:    Delay
*   Description: executes a specified number of nops
*******************************************************************/
void Delay(const int n)
{
	int nDelay;
	for( nDelay = 0; nDelay < n; nDelay++)
	{
		asm("nop;");
	}
}
