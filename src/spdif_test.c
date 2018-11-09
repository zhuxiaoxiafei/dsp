/*******************************************************************
Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software
you agree to the terms of the associated Analog Devices License Agreement.

Project Name:  	Power_On_Self_Test

Hardware:		ADSP-21479/21489 EZ-Board

Description:	This file tests the SPDIF interface on the EZ-Board.
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
#include <math.h>
#include <stdlib.h>
#include <filter.h>
#include <sru.h>
#include <signal.h>
#include <sysreg.h>
#include "post_debug.h"
#include <services\int\adi_int.h>


/*******************************************************************
*  function prototypes
*******************************************************************/
void Init_Sport_SPDIF(void);
void Init_SPDIF(void);
void SPDIF_Sport0_ISR(int sig_int, void *);
int Test_Channel(float* pfRealIn, const int iMaxSamples, const int iSampleRate,
					const float fExpFreq, const float fFreqTol, const float fExpAmp,
					const float fAmpTol);


/*******************************************************************
*  global variables and defines
*******************************************************************/
#define MAX_SAMPLES	 					256
#define PI								((float)3.141592765309)

#define SPDIF_REQUIRED_SAMPLES			((MAX_SAMPLES) * 100)
#define SPDIF_DESIRED_FREQ 				((float)3000.0)
#define SPDIF_SAMPLE_RATE 				((float)48000.0)
#define SPDIF_AMPLITUDE					((float)8388607.5)
#define SPDIF_ACCEPTABLE_DEVIATION_PCT	((float)0.015)
#define SPDIF_MAX_SIGNAL_STRENGTH		(float)1000000.0

volatile bool g_bLeftRight = 1;
volatile int  g_iSampleIndex = 1;
volatile int  g_iSampleCount = 0;

volatile float g_fSineWaveOut[MAX_SAMPLES];
volatile float g_fSineWaveIn_Left[MAX_SAMPLES];
volatile float g_fSineWaveIn_Right[MAX_SAMPLES];


/*******************************************************************
*   Function:    Init_Sport_SPDIF
*   Description: Initializes the SPORT interface to communicate with
*					the SPDIF.
*******************************************************************/

void Init_Sport_SPDIF(void)
{
	/* clear the multichannel mode registers */
	*pSPMCTL1 = 0;
/* 	*pSPMCTL23 = 0; */
    *pSPCTL0 = 0;
    *pSPCTL1 = 0;
    *pSPCTL2 = 0;
	*pSPCTL2 = 0;


	/* Configure SPORT 0 for input from SPDIF

		OPMODE = I2S mode
		L_FIRST = I2S sends left channel first
		SLEN24 = 24 bit of data in each 32-bit word
		SPEN_A = Enable data channel A
	*/

	*pDIV0 = 0;
	*pSPCTL0 = (OPMODE | L_FIRST | SLEN24 | SPEN_A);


	/* Configure SPORT 3 for output to SPDIF

		SPTRAN = Transmit on serial port
		OPMODE = I2S mode
		L_FIRST = I2S sends left channel first
		SLEN24 = 24 bit of data in each 32-bit word
		SPEN_A = Enable data channel A
	*/

	*pDIV3 = 0;
	*pSPCTL3 = (SPTRAN | OPMODE | L_FIRST | SLEN32 | SPEN_A);
}


/*******************************************************************
*   Function:    Init_SPDIF
*   Description: Initializes the SPDIF.
*******************************************************************/

void Init_SPDIF(void)
{
	/* use default SPDIF Receiver mode */
    *pDIRCTL = 0;

	/* init SPDIF transmitter in I2S mode */
    *pDITCTL = ( DIT_EN | DIT_IN_I2S );
}


/*******************************************************************
*   Function:    Config_SRU_SPDIF
*   Description: Configure the SRU for SPDIF.
*******************************************************************/

void Config_SRU_SPDIF(void)
{

	/* connect the SPDIF receiver */
    SRU(DAI_PB18_O, DIR_I);

	/* connect the SPDIF receiver outputs */
    SRU(DIR_DAT_O, SPORT0_DA_I);
    SRU(DIR_FS_O, SPORT0_FS_I);
    SRU(DIR_CLK_O, SPORT0_CLK_I);

	/* connect the SPDIF transmitter inputs */
    SRU(SPORT3_DA_O, DIT_DAT_I);
    SRU(DIR_FS_O, DIT_FS_I);
    SRU(DIR_CLK_O, DIT_CLK_I);
    SRU(DIR_CLK_O, SPORT3_CLK_I);

    SRU(DIR_FS_O, SPORT3_FS_I );

    SRU(DIR_TDMCLK_O, DIT_HFCLK_I); /* also used in external pll code */

	/* connect the SPDIF transmitter output */
    SRU(DIT_O, DAI_PB01_I);

	/* Tie the pin buffer inputs LOW for DAI pin 18.  Even though
		these pins are inputs to the SHARC, tying unused pin buffer inputs
		LOW is "good coding style" to eliminate the possibility of
		termination artifacts internal to the IC.  Note that signal
		integrity is degraded only with a few specific SRU combinations.
		In practice, this occurs VERY rarely, and these connections are
		typically unnecessary. */
    SRU(LOW, DAI_PB18_I);


	/* tie the pin buffer enable inputs LOW for DAI pin 18 so that they are
		always input pins */
    SRU(LOW, PBEN18_I);

	/* tie the pin buffer enable inputs HIGH to make DAI pin 1 an output */
    SRU(HIGH,PBEN01_I);
}


/*******************************************************************
*   Function:    Test_SPDIF
*   Description: Tests the SPDIF interface.
*******************************************************************/

int Test_SPDIF(void)
{
	volatile int bError = 0; 	/* returning 1 indicates a pass, anything else is a fail */

	/* reset the buffer indexes */
    g_iSampleIndex = 0;
	g_iSampleCount = 0;

	DEBUG_HEADER( "SPDIF Test" );
	DEBUG_PRINT( "\nCollecting 0x%x samples for processing...", SPDIF_REQUIRED_SAMPLES );

	Config_SRU_SPDIF();		/* inits the SRU & DAI/DPI pins */
	Init_Sport_SPDIF();		/* inits the transmit and receive serial ports (SPORTS) */
	Init_SPDIF();    		/* inits the SPDIF transmitter & receiver */

	/* setup interrupt */
	adi_int_InstallHandler(ADI_CID_P6I, (ADI_INT_HANDLER_PTR)SPDIF_Sport0_ISR, NULL, true);

    /* loop until done */
    while( g_iSampleCount < SPDIF_REQUIRED_SAMPLES )
    {
	    /* once the required number of samples has been collected, we'll process the signal */
    };

	/* turn off interrupts so that the data is stable */
	adi_int_EnableInt(ADI_CID_P6I, false);
	adi_int_UninstallHandler(ADI_CID_P6I);

	DEBUG_STATEMENT( "done" );
    DEBUG_STATEMENT( "\n\nProcessing signal..." );

	/* test the left channel */
	bError = Test_Channel((float*)g_fSineWaveIn_Left, MAX_SAMPLES, SPDIF_SAMPLE_RATE,
    						SPDIF_DESIRED_FREQ, SPDIF_ACCEPTABLE_DEVIATION_PCT,
    						SPDIF_MAX_SIGNAL_STRENGTH, SPDIF_MAX_SIGNAL_STRENGTH);

	DEBUG_STATEMENT( "done\n\n" );

	DEBUG_RESULT( bError, "Test passed", "Test failed" );

	/* return status */
	return bError;
}


/*******************************************************************
*   Function:    SPDIF_Sport0_ISR
*   Description: Tests the SPDIF interface.
*******************************************************************/

void SPDIF_Sport0_ISR(int sig_int, void *pCBParam)
{
	/* read the value */
	int nInValue = (int)*pRXSP0A;

	if( nInValue & 0x00800000)
	{
		/* negative sign extend */
		nInValue |= 0xFF000000;
	}

	/* store the value */
	g_fSineWaveIn_Left[g_iSampleIndex] = (float)nInValue;

	*pTXSP3A = (int)(SPDIF_AMPLITUDE * sin( (2.0 * PI * SPDIF_DESIRED_FREQ * ( ((float)(g_iSampleIndex+1)) / SPDIF_SAMPLE_RATE))) );

	if( g_bLeftRight )
	{
		/* right side */
	}
	else
	{
		/* left side */

		g_iSampleIndex++;	/* increment the index */
	}

	/* toggle */
	g_bLeftRight = !g_bLeftRight;

	/* check if we've reached MAX_SAMPLES, if so reset index */
	if( g_iSampleIndex > MAX_SAMPLES-1 )
		g_iSampleIndex = 0;

	/* bump the counter */
	g_iSampleCount++;
}


/*******************************************************************
*   Function:    Test_Channel
*   Description: Tests the supplied signal.
*******************************************************************/

int Test_Channel(float* pfRealIn, const int iMaxSamples, const int iSampleRate,
					const float fExpFreq, const float fFreqTol, const float fExpAmp,
					const float fAmpTol)
{

	complex_float *cfFFTOut; //[(MAX_SAMPLES/2)];
	float fTempFreq = 0;
	int nSampleNumber = 0;
	int nHighestFreqIndex = 0;
	float fSampledFrequency = 0;
	float fSlope = 0.0;
	int   iSlopeY1 = 0;
	int iError=1; 	//default set to pass

	const float fMaxFreq = (fExpFreq + (fExpFreq * fFreqTol));
	const float fMinFreq = (fExpFreq - (fExpFreq * fFreqTol));

	cfFFTOut = (complex_float *) malloc(iMaxSamples/2);
   	/* real input array fills from a converter or other source */
   	if(iMaxSamples==256)
   	{

		rfft256( pfRealIn, (complex_float*)cfFFTOut);
   	}else if(iMaxSamples==4096)
   	{


		rfft4096( pfRealIn, cfFFTOut);
   	}

   	/* arrays are filled with FFT data, scan the output array for the highest real value */
   	fTempFreq = abs(cfFFTOut->re);
 	for (nSampleNumber = 1; nSampleNumber < (iMaxSamples / 2); nSampleNumber++)
    {
		if( abs((cfFFTOut + nSampleNumber)->re) > fTempFreq )
		{
			fTempFreq = abs((cfFFTOut + nSampleNumber)->re);
			nHighestFreqIndex = nSampleNumber;
		}
    }

   /* multiply the index of the array of the highest value with the sample rate value */
   fSampledFrequency = nHighestFreqIndex * (iSampleRate / iMaxSamples);

   /* make sure frequency is within acceptable ranges */
   if( (fSampledFrequency < fMaxFreq) && (fSampledFrequency > fMinFreq) )
   {
   		/* for now, take the point before, and after the max value
   			average them, then find the distance between them */

	    /* the slope is given by b = (y2 - y1) / (x2 - x1) or b = y2-y1 */
	    if( 0 == nSampleNumber )
	    {	/* roll around to the end of the array */
	    	iSlopeY1 = iMaxSamples;
	    }
	    else if( iMaxSamples == nSampleNumber )
	    {
	    	iSlopeY1 = 0;
	    }
	    else
	    {
	    	iSlopeY1 = nSampleNumber - 1;
	    }

	    fSlope = ((cfFFTOut+nHighestFreqIndex)->re - (cfFFTOut+iSlopeY1)->re);
	    fSlope = abs(fSlope);

	    if( fSlope < fExpAmp )
	    {
	    	/* test failed */
	    	iError = 0;
	    }

   }else
   {
    	/* test failed */
    	iError = 0;

   }
	free(cfFFTOut);
	/* return test result */
	return iError;
}


