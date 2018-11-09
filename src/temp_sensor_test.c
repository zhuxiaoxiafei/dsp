/*******************************************************************
Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software
you agree to the terms of the associated Analog Devices License Agreement.

Project Name:  	Power_On_Self_Test

Hardware:		ADSP-21489 EZ-Board

Description:	This file tests the ADM1032 Temperature Monitor interface on the EZ-Board.
*******************************************************************/

/*******************************************************************
*  include files
*******************************************************************/
#include <cdef21489.h>
#include <def21489.h>
#include <sru.h>
#include <signal.h>
#include <sysreg.h>
#include <filter.h>
#include "post_debug.h"
#include <services\int\adi_int.h>


/*******************************************************************
*  function prototypes
*******************************************************************/
void Init_TWI_TempSensor(void);
//void TempSensor_ISR(int sig_int);
static void TempSensor_ISR(int, void *);
//void TempSensor_Alert_ISR(int sig_int);
static void TempSensor_Alert_ISR(int, void *);
void Clear_Alert_Latch(void);
int Read_TWI_Register(int address);
void Write_TWI_Register(int address, int data);


/*******************************************************************
*  global variables and defines
*******************************************************************/

#define TWI_INTERRUPT (P0I0|P0I1|P0I2|P0I4)
#define TWIDCNT1 BIT_6
#define TWIDCNT2 BIT_7

#define	MAX_TEMPERATURE_SAMPLES	50	/* number of samples to check */

/* device address */
#define TEMP_SENSOR_ADDRESS		0x4C

/* register addresses for ADM1032 Temperature Monitor */

#define LT_R			0x00  	/* local temperature - READ */
#define LHS_W			0x0B 	/* local temperature HIGH limit - WRITE */
#define LHS_R			0x05 	/* local temperature HIGH limit - READ */
#define LLS_W			0x0C 	/* local temperature LOW limit - WRITE */
#define LLS_R			0x06 	/* local temperature LOW limit - READ */

#define RHSHB_W			0x0D 	/* remote temperature HIGH limit (high byte) - WRITE */
#define RHSHB_R			0x07 	/* remote temperature HIGH limit (high byte) - READ */
#define RHSLB_W			0x13 	/* remote temperature HIGH limit (low byte) - WRITE */
#define RHSLB_R			0x13 	/* remote temperature HIGH limit (low byte) - READ */

#define RLSHB_W			0x0E 	/* remote temperature LOW limit (high byte) - WRITE */
#define RLSHB_R			0x08 	/* remote temperature LOW limit (high byte) - READ */
#define RLSLB_W			0x14 	/* remote temperature LOW limit (low byte) - WRITE */
#define RLSLB_R			0x14 	/* remote temperature LOW limit (low byte) - READ */

#define CONFIG_W		0x09 	/* configuration register - WRITE */
#define CONFIG_R		0x03	/* configuration register - READ */
#define RCS_W			0x19	/* remote THERM limit - WRITE */
#define RCS_R			0x19	/* remote THERM limit - READ */
#define LCS_W			0x20	/* local THERM limit - WRITE */
#define LCS_R			0x20	/* local THERM limit - READ */
#define SR_R			0x02	/* status register - READ */

#define RTHB_R			0x01	/* remote temperature value (high byte) - READ */
#define RTLB_R			0x10	/* remote temperature value (low byte) - READ */
#define RTOHB_R			0x11	/* remote temperature offset (high byte) - READ */
#define RTOHB_W			0x11	/* remote temperature offset (high byte) - WRITE */
#define RTOLB_R			0x12	/* remote temperature offset (low byte) - READ */
#define RTOLB_W			0x12	/* remote temperature offset (low byte) - WRITE */

#define HYSTERISIS_R	0x21	/* therm hysterisis value - READ */
#define HYSTERISIS_W	0x21	/* therm hysterisis value - WRITE */
#define CR_W			0x0A	/* conversion rate - WRITE */
#define CR_R			0x04	/* conversion rate - READ */

/* status register bit definitions */
#define ADC_BUSY 0x80
#define LHIGH    0x40
#define LLOW	 0x20
#define RHIGH    0x10
#define RLOW	 0x08
#define OPEN 	 0x04
#define RTHRM    0x02
#define LTHRM    0x01

/* temperature limit settings */
#define REMOTE_OFFSET 			-8
#define LOCAL_THERM_LIMIT 		40
#define REMOTE_THERM_LIMIT  	45
#define THERM_HYSTERISIS 		2
#define REMOTE_ALERT_HIGH_LIMIT 40
#define REMOTE_ALERT_LOW_LIMIT  20
#define LOCAL_ALERT_HIGH_LIMIT  40
#define LOCAL_ALERT_LOW_LIMIT   20

volatile int g_write_pend = 1;			/* write pending flag */
volatile int g_read_pend = 1;			/* read pending flag */
volatile int g_buff_empty = 1;			/* buffer empty flag */
volatile int g_alert = 0;				/* temperature alert flag */


/*******************************************************************
*   Function:    Init_TWI_TempSensor
*   Description: Initializes the SPDIF.
*******************************************************************/

void Init_TWI_TempSensor(void)
{
	/* setup the interrupt */
	*pPICR0 = TWI_INTERRUPT;
	adi_int_InstallHandler(ADI_CID_P0I,(ADI_INT_HANDLER_PTR)TempSensor_ISR, NULL, true);

	*pTWIMITR = 20 | TWIEN;				/* reference clock = 159.7/16=9.984 MHz */
	*pTWIDIV = 0x6464;					/* SCL clock divider, high and low */
	*pTWIMADDR = TEMP_SENSOR_ADDRESS;	/* device address specific to temp sensor */
	*pTWIFIFOCTL = TWIBHD;				/* read/write happens regardless of FIFO status */
}


/*******************************************************************
*   Function:    Test_Temp_Sensor
*   Description: Tests the temp sensor interface.
*******************************************************************/

int Test_Temp_Sensor(void)
{
	int i = 0, j = 0;				/* indexes */
	int num_samples = 0x0;			/* number of samples taken */
	int	temp;						/* temporary int */
	int picr0_backup = 0x0;			/* backup of PICR0 register */
	int remote_therm_limit = 0;		/* remote therm limit */
	float ftemp = 0;				/* temporary float */
	float hi = 0, low = 0;			/* hi and low temp */
	float init_remote_temp = 0;		/* initial remote temp */
	bool bRemoteThermLimit = false;	/* remote therm limit flag */


	DEBUG_HEADER( "Temp Sensor Test" );

	/* configure SRU */
	SRU(LOW,DPI_PB01_I);
	SRU(LOW,DPI_PB02_I);
	SRU(LOW,DPI_PB07_I);
	SRU(TWI_DATA_PBEN_O,DPI_PBEN07_I);
	SRU(DPI_PB07_O, TWI_DATA_I);
	SRU(LOW,DPI_PB08_I);
	SRU2(TWI_CLK_PBEN_O,DPI_PBEN08_I);

	picr0_backup = *pPICR0;							/* back it up */
	Init_TWI_TempSensor();							/* init TWI interface */

	sysreg_bit_set(sysreg_MODE2, IRQ0E );			/* enable interrupt */
	asm("nop;nop;nop;nop;nop;nop;nop;nop;");
	*pSYSCTL |= IRQ0EN;								/* set FLAG0 to IRQ0 */
	adi_int_InstallHandler(ADI_CID_IRQ0I, (ADI_INT_HANDLER_PTR)TempSensor_Alert_ISR, NULL, true);

	/* check the status register */
	temp = Read_TWI_Register(SR_R);
	DEBUG_PRINT("\nStatus register is: 0x%x", temp);
	temp = Read_TWI_Register(RCS_R);
	DEBUG_PRINT("\nRemote THERM limit set to: %d degrees C", temp);

	/* set conversion rate register, 0xa will allow 64 conversions/second but also
		increases power consumption */
	Write_TWI_Register(CR_W, 0x0A);
	temp = Read_TWI_Register(CR_R);

	Write_TWI_Register(RTOHB_W, REMOTE_OFFSET);
	temp = Read_TWI_Register(RTOHB_R);
	DEBUG_PRINT("\nOffset Register Set to: 0x%x", temp);

	/* set local therm limit */
	Write_TWI_Register(LCS_W, LOCAL_THERM_LIMIT);
	temp = Read_TWI_Register(LCS_W);
	DEBUG_PRINT("\nLocal THERM limit set to: %d degrees C", temp);

	/* set THERM hysterisis value */
	Write_TWI_Register(HYSTERISIS_W, THERM_HYSTERISIS);
	temp = Read_TWI_Register(HYSTERISIS_R);
	DEBUG_PRINT("\nTherm hysterisis set to: %d degrees C", temp);

	/* set remote temperature ALERT limits */
	Write_TWI_Register(RHSHB_W, REMOTE_ALERT_HIGH_LIMIT);
	temp = Read_TWI_Register(RHSHB_R);
	DEBUG_PRINT("\n\nRemote HIGH limit set to: %d degrees C", temp);
	Write_TWI_Register(RLSHB_W, REMOTE_ALERT_LOW_LIMIT);
	temp = Read_TWI_Register(RLSHB_R);
	DEBUG_PRINT("\nRemote LOW limit set to:  %d degrees C", temp);

	/* set local temerature ALERT limits */
	Write_TWI_Register(LHS_W, LOCAL_ALERT_HIGH_LIMIT);
	temp = Read_TWI_Register(LHS_R);
	DEBUG_PRINT("\nLocal HIGH limit set to:  %d degrees C", temp);
	Write_TWI_Register(LLS_W, LOCAL_ALERT_LOW_LIMIT);
	temp = Read_TWI_Register(LLS_R);
	DEBUG_PRINT("\nLocal LOW limit set to:   %d degrees C", temp);

	/* clear any pending ALERT latch */
	Clear_Alert_Latch();

	/* get initial local temp */
	temp = Read_TWI_Register(LT_R);
	DEBUG_PRINT("\n\n\nInit local temperature is: %d degrees C", temp);

	/* get initial remote temp (remote is stored in two bytes) */
	hi = (float)Read_TWI_Register(RTHB_R);
	temp = Read_TWI_Register(RTLB_R);
	temp >>= 5;
	low = 0.125 * temp;
	init_remote_temp = hi + low;
	DEBUG_PRINT("\nInit remote temperature is: %.3f degrees C\n", hi + low);

	/* now set the remote therm limit to X + 3 degrees C, where X is the
		initial remote temp (high byte only) we just read */
	remote_therm_limit = (int)hi + 3;
	Write_TWI_Register(RCS_W, remote_therm_limit);
	temp = Read_TWI_Register(RCS_R);
	DEBUG_PRINT("\nRemote THERM limit set to: %d degrees C", temp);

	DEBUG_PRINT("\n\nRemote temp much exceed %d degrees C for test to pass", temp);

	while( !bRemoteThermLimit && (num_samples < MAX_TEMPERATURE_SAMPLES) )
	{
		/* do some processing between samples to raise temp */
		for ( i = 0; i < 100000; i++)
		{
			complex_float in[2048];
			float out[2048];
			fft_magnitude (in, out, 2048, 2 );
		}

		/* get local temp */
		temp = Read_TWI_Register(LT_R);
		DEBUG_PRINT("\n\n%d) Current local temp is: %d degrees C", num_samples, temp);

		/* get remote temp high and low */
		hi = (float)Read_TWI_Register(RTHB_R);
		temp = Read_TWI_Register(RTLB_R);
		temp >>= 5;
		low = 0.125 * temp;
		DEBUG_PRINT("\n%d) Current remote temp is: %.3f degrees C\n", num_samples, hi + low);

		/* check the status register */
		temp = Read_TWI_Register(SR_R);

		/* check the remote */
		if((temp & RTHRM) != 0)
		{
			/* if this happens our test has passed, set the flag and print status */
			bRemoteThermLimit = true;
			DEBUG_STATEMENT("\nRemote temperature is more than its THERM limit\n");
		}

		/* check the local */
		if((temp & LTHRM) != 0)
		{
			/* if this happens just print status */
			DEBUG_STATEMENT("\nLocal temperature is more than its THERM limit\n");
		}

		/* if it wasn't an alert that tripped, then clear the alert */
		if( (temp & (LHIGH | LLOW | RHIGH | RLOW)) == 0 )
		{
			Clear_Alert_Latch();
			g_alert = 0;
		}

		/* while there's an alert */
		while ( 1 == g_alert )
		{
			/* check status register */
			temp = Read_TWI_Register(SR_R);

			/* is it local high? */
			if ( (temp & LHIGH) != 0 )
			{
				DEBUG_STATEMENT("\nLocal temperature is more than its ALERT HIGH limit\n");
			}

			/* is it local low? */
			if ( (temp & LLOW) != 0 )
			{
				DEBUG_STATEMENT("\nLocal temperature is less than its ALERT LOW limit\n");
			}

			/* is it remote high? */
			if ( (temp & RHIGH) != 0 )
			{
				DEBUG_STATEMENT("\nRemote temperature is more than its ALERT HIGH limit\n");
			}

			/* is it remote low? */
			if ( (temp & RLOW) != 0 )
			{
				DEBUG_STATEMENT("\nRemote temperature is less than its ALERT LOW limit\n");
			}

			/* if a temp limit tripped clear the alert */
			if ( ( temp & (LHIGH | LLOW | RHIGH | RLOW) ) == 0 )
			{
				Clear_Alert_Latch();
				g_alert = 0;
			}
		}

		/* bump sample count */
		num_samples++;
	}

	/* setup to ignore these interrupts */
	adi_int_EnableInt(ADI_CID_IRQ0I, false);
	adi_int_EnableInt(ADI_CID_P0I, false);
	adi_int_UninstallHandler(ADI_CID_IRQ0I);
	adi_int_UninstallHandler(ADI_CID_P0I);

	/* restore PICR0, clear IRQ0 bit */
	sysreg_bit_clr(sysreg_MODE2, IRQ0E );
	(*pPICR0) = picr0_backup;
	*pSYSCTL &= ~IRQ0EN;

	/* if we get here either the remote therm limit was reached (PASS) or we collected
		the max samples without reaching the limit (FAIL), display the result */
	DEBUG_RESULT( bRemoteThermLimit, "\nTest passed", "\nTest failed" );

	/* return status */
	return (int)bRemoteThermLimit;
}


/*******************************************************************
*   Function:    TempSensor_ISR
*   Description: Handler for the temp sensor ISR.
*******************************************************************/

static void TempSensor_ISR(int sig_int, void *pCBParam)
{
	/* read the processor's TWI interrupt source reg */
	int temp = *pTWIIRPTL;

	/* check master TX complete bit */
	if ( (temp & TWIMCOM) == 0 )
	{
		/* if clear that means a transfer not detected, set empty flag */
		g_buff_empty=1;

		/* clear TX FIFO service bit */
		temp |= TWITXINT;
		*pTWIIRPTL=temp;
	}

	else
	{
		/* else it means master TX is complete, clear this bit */
		temp |= TWIMCOM;
		*pTWIIRPTL=temp;

		/* update our flags */
		if( g_write_pend == 1 )
			g_write_pend=0;
		else
			g_read_pend=0;
	}
}


/*******************************************************************
*   Function:    TempSensor_Alert_ISR
*   Description: Handler for the temp sensor alert ISR.
*******************************************************************/

void TempSensor_Alert_ISR(int sig_int, void *pCBParam)
{
	g_alert++;	/* bump alert */
}


/*******************************************************************
*   Function:    Clear_Alert_Latch
*   Description: Clears any alert latch.
*******************************************************************/

void Clear_Alert_Latch(void)
{
	int temp;			/* temporary int */


	/* write device address */
	*pTWIMADDR = 0x0C;

	/* setup flags */
	g_write_pend = 0;
	g_read_pend = 1;

	/* enable master transfer complete interrupt */
	*pTWIIMASK = TWIMCOM;

	/* setup for 1 data byte, master enable, master RX */
	*pTWIMCTL = TWIDCNT1 | TWIMEN | TWIMDIR;

	/* wait for read to complete */
	while( g_read_pend == 1 )
	{
		;
	}

	/* read data from the 8-bit RX fifo */
	temp = *pRXTWI8;

	/* set the device address */
	*pTWIMADDR = TEMP_SENSOR_ADDRESS;
}


/*******************************************************************
*   Function:    Read_TWI_Register
*   Description: Returns value of a TWI register based on specified address.
*******************************************************************/

int Read_TWI_Register(int address)
{
	int temp;			/* temporary int */


	/* setup flags, to read we do both write and read */
	g_write_pend = 1;
	g_read_pend = 1;

	/* write address to the 8-bit TX fifo */
	*pTXTWI8 = address;

	/* enable master transfer complete interrupt */
	*pTWIIMASK = TWIMCOM;

	/* setup for 1 data byte, master enable, master TX */
	*pTWIMCTL = TWIDCNT1 | TWIMEN;

	/* wait for write to complete */
	while( g_write_pend == 1 )
	{
		;
	}

	/* setup for 1 data byte, master enable, master RX */
	*pTWIMCTL = TWIDCNT1 | TWIMEN | TWIMDIR;

	/* wait for read to complete */
	while( g_read_pend == 1 )
	{
		;
	}

	/* read data from the 8-bit RX fifo */
	temp = *pRXTWI8;

	return temp;
}


/*******************************************************************
*   Function:    Write_TWI_Register
*   Description: Writes a TWI register based on specified address and data.
*******************************************************************/

void Write_TWI_Register(int address, int data)
{
	int temp;			/* temporary int */


	/* setup flag */
	g_write_pend = 1;

	/* write address to the 8-bit TX fifo */
	*pTXTWI8 = address;

	/* enable master transfer complete and TX FIFO service interrupts */
	*pTWIIMASK = TWIMCOM | TWITXINT;

	/* setup for 2 data bytes, master enable, master TX */
	*pTWIMCTL = TWIDCNT2 | TWIMEN;

	/* wait for data */
	while ( g_buff_empty != 1 )
	{
		;
	}

	/* write data to the 8-bit TX fifo */
	*pTXTWI8 = data;

	/* reset flag */
	g_buff_empty = 1;

	/* wait for write to complete */
	while( g_write_pend == 1 )
	{
		;
	}
}
