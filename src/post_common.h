/*******************************************************************************/
/*                                                                             */
/*   (C) Copyright 2008 - Analog Devices, Inc.  All rights reserved.           */
/*                                                                             */
/*    FILE:     post_common.h  								                   */
/*                                                                             */
/*    CHANGES:  1.00.0  - initial release    								   */
/*																			   */
/*******************************************************************************/

#ifndef _POST_COMMON_H_
#define _POST_COMMON_H_

#include "pb_led_test.h"

/*******************************************************************
*  function prototypes
*******************************************************************/

void Init_PLL(void);
void Init_SDRAM(void);
void Init_AMI(void);

void Delay(const int n);

int Test_Standard_Loop(void);
int Toggle_Loop_Flag(void);
int Display_Post_Status(void);

int Test_Processor_Version(void);
int Test_Pushbuttons_LEDs(void);

int Test_SDRAM(void);
int Test_SRAM(void);
int Test_SPI_Flash(void);
int Test_Parallel_Flash(void);

int Test_UART(void);
int Test_Analog_Audio_RCA(void);
int Test_Analog_Audio_Diff(void);
int Test_Audio_Extender_RCA(void);
int Test_Audio_Extender_Diff(void);
int Test_Audio_Extender_CodecA(void);

#ifdef __ADSP21479__
int Set_RTC(void);
int Check_RTC(void);
#endif

int Test_Watchdog_Timer(void);    
int Test_SPDIF(void);
int Test_Temp_Sensor(void);

int Check_Config_Data(void);
int Blink_FW_Version(void);

/*******************************************************************
*  global variables and defines
*******************************************************************/

#ifndef NULL
	#define NULL	((void *)0)
#endif

#define NUM_POST_TESTS			32

#define IGNORE_FAILURES			1
#define DO_NOT_IGNORE_FAILURES	0

/* test paramaters */
#define MAX_SAMPLES	 				256
#define REQUIRED_SAMPLES			((MAX_SAMPLES) * 250)
#define DESIRED_FREQ 				((float)3000.0)
#define SAMPLE_RATE 				((float)48000.0)
#define AMPLITUDE					((float)12000)
#define PI							((float)3.141592765309)

#define ACCEPTABLE_DEVIATION_PCT	((float)0.015)
#define ACCEPTABLE_DEVIATION		(DESIRED_FREQ * ACCEPTABLE_DEVIATION_PCT)
#define MAX_DESIRED_FREQ			(DESIRED_FREQ + ACCEPTABLE_DEVIATION)
#define MIN_DESIRED_FREQ			(DESIRED_FREQ - ACCEPTABLE_DEVIATION)
#define MIN_SIGNAL_STRENGTH			(float)35.0
#define MAX_NOISE_THRESHOLD			(float)26

/* names for slots in codec audio frame */
#define INTERNAL_ADC_L0			0
#define INTERNAL_ADC_R0			1

#define INTERNAL_DAC_L0			0
#define INTERNAL_DAC_R0			1

#define INTERNAL_ADC_L1			2
#define INTERNAL_ADC_R1			3

#define INTERNAL_DAC_L1			2
#define INTERNAL_DAC_R1			3


enum _POST_STATE
{
	STATE_START,
	STATE_1,
	STATE_2,
	STATE_3,
	STATE_4,
	STATE_SELECTED
};

typedef enum TESTS_tag								/* LED PATTERN */
{
	TEST_0  = ( 							   0 ),	/* 00000 */
	TEST_1  = ( 							LED1 ),	/* 00001 */
	TEST_2  = ( 					 LED2		 ),	/* 00010 */
	TEST_3  = ( 					 LED2 | LED1 ),	/* 00011 */
	TEST_4  = ( 			  LED3				 ),	/* 00100 */
	TEST_5  = ( 			  LED3 | 		LED1 ),	/* 00101 */
	TEST_6  = ( 			  LED3 | LED2	  	 ),	/* 00110 */
	TEST_7  = ( 			  LED3 | LED2 | LED1 ),	/* 00111 */
	
	TEST_8  = ( 	   LED4 					 ),	/* 01000 */
	TEST_9  = ( 	   LED4 | 				LED1 ),	/* 01001 */
	TEST_10 = ( 	   LED4 | 		 LED2		 ),	/* 01010 */
	TEST_11 = ( 	   LED4 | 		 LED2 | LED1 ),	/* 01011 */
	TEST_12 = ( 	   LED4 | LED3				 ),	/* 01100 */
	TEST_13 = ( 	   LED4 | LED3 | 		LED1 ),	/* 01101 */
	TEST_14 = ( 	   LED4 | LED3 | LED2	  	 ),	/* 01110 */
	TEST_15 = ( 	   LED4 | LED3 | LED2 | LED1 ),	/* 01111 */
	
	TEST_16 = ( LED5				   			 ),	/* 10000 */
	TEST_17 = ( LED5 |						LED1 ),	/* 10001 */
	TEST_18 = ( LED5 |				 LED2		 ),	/* 10010 */
	TEST_19 = ( LED5 |				 LED2 | LED1 ),	/* 10011 */
	TEST_20 = ( LED5 |		  LED3				 ),	/* 10100 */
	TEST_21 = ( LED5 |		  LED3 | 		LED1 ),	/* 10101 */
	TEST_22 = ( LED5 |		  LED3 | LED2	  	 ),	/* 10110 */
	TEST_23 = ( LED5 |		  LED3 | LED2 | LED1 ),	/* 10111 */
	
	TEST_24 = ( LED5 | LED4			   			 ),	/* 11000 */
	TEST_25 = ( LED5 | LED4 |				LED1 ),	/* 11001 */
	TEST_26 = ( LED5 | LED4 |		 LED2		 ),	/* 11010 */
	TEST_27 = ( LED5 | LED4 |		 LED2 | LED1 ),	/* 11011 */
	TEST_28 = ( LED5 | LED4 | LED3				 ),	/* 11100 */
	TEST_29 = ( LED5 | LED4 | LED3 | 		LED1 ),	/* 11101 */
	TEST_30 = ( LED5 | LED4 | LED3 | LED2	  	 ),	/* 11110 */
	TEST_31 = ( LED5 | LED4 | LED3 | LED2 | LED1 ),	/* 11111 */
	
	LAST_TEST = TEST_31

} enTESTS;


/* these states use the convention from pbled_test where each flag
	uses 2 bits, 0 = OFF, 1 = ON, 2 = TOGGLE */
typedef enum TEST_STATES_tag
{
	TEST_0_CLR = 0,
	TEST_0_SET = 0,
	TEST_0_TGL = 0,
	TEST_0_EXT = 0,	

	TEST_1_CLR = 0,
	TEST_1_SET = 1,
	TEST_1_TGL = 2,
	TEST_1_EXT = 3, 

	TEST_2_CLR = (TEST_1_CLR << 2),
	TEST_2_SET = (TEST_1_SET << 2),
	TEST_2_TGL = (TEST_1_TGL << 2),
	TEST_2_EXT = (TEST_1_EXT << 2),
	
	TEST_3_CLR = (TEST_2_CLR | TEST_1_CLR),
	TEST_3_SET = (TEST_2_SET | TEST_1_SET),
	TEST_3_TGL = (TEST_2_TGL | TEST_1_TGL),
	TEST_3_EXT = (TEST_2_EXT | TEST_1_EXT),

	TEST_4_CLR = (TEST_1_CLR << 4),
	TEST_4_SET = (TEST_1_SET << 4),
	TEST_4_TGL = (TEST_1_TGL << 4),
	TEST_4_EXT = (TEST_1_EXT << 4),
	
	TEST_5_CLR = (TEST_4_CLR | TEST_1_CLR),
	TEST_5_SET = (TEST_4_SET | TEST_1_SET),
	TEST_5_TGL = (TEST_4_TGL | TEST_1_TGL),
	TEST_5_EXT = (TEST_4_EXT | TEST_1_EXT),
	
	TEST_6_CLR = (TEST_4_CLR | TEST_2_CLR),
	TEST_6_SET = (TEST_4_SET | TEST_2_SET),
	TEST_6_TGL = (TEST_4_TGL | TEST_2_TGL),
	TEST_6_EXT = (TEST_4_EXT | TEST_2_EXT),
	
	TEST_7_CLR = (TEST_4_CLR | TEST_2_CLR | TEST_1_CLR),
	TEST_7_SET = (TEST_4_SET | TEST_2_SET | TEST_1_SET),
	TEST_7_TGL = (TEST_4_TGL | TEST_2_TGL | TEST_1_TGL),
	TEST_7_EXT = (TEST_4_EXT | TEST_2_EXT | TEST_1_EXT),
	
	TEST_8_CLR = (TEST_1_CLR << 6),
	TEST_8_SET = (TEST_1_SET << 6),
	TEST_8_TGL = (TEST_1_TGL << 6),
	TEST_8_EXT = (TEST_1_EXT << 6),

	TEST_9_CLR = (TEST_8_CLR | TEST_1_CLR),
	TEST_9_SET = (TEST_8_SET | TEST_1_SET),
	TEST_9_TGL = (TEST_8_TGL | TEST_1_TGL),
	TEST_9_EXT = (TEST_8_EXT | TEST_1_EXT),

	TEST_10_CLR = (TEST_8_CLR | TEST_2_CLR),
	TEST_10_SET = (TEST_8_SET | TEST_2_SET),
	TEST_10_TGL = (TEST_8_TGL | TEST_2_TGL),
	TEST_10_EXT = (TEST_8_EXT | TEST_2_EXT),
	
	TEST_11_CLR = (TEST_8_CLR | TEST_2_CLR | TEST_1_CLR),
	TEST_11_SET = (TEST_8_SET | TEST_2_SET | TEST_1_SET),
	TEST_11_TGL = (TEST_8_TGL | TEST_2_TGL | TEST_1_TGL),
	TEST_11_EXT = (TEST_8_EXT | TEST_2_EXT | TEST_1_EXT),
	
	TEST_12_CLR = (TEST_8_CLR | TEST_4_CLR),
	TEST_12_SET = (TEST_8_SET | TEST_4_SET),
	TEST_12_TGL = (TEST_8_TGL | TEST_4_TGL),
	TEST_12_EXT = (TEST_8_EXT | TEST_4_EXT),
	
	TEST_13_CLR = (TEST_8_CLR | TEST_4_CLR | TEST_1_CLR),
	TEST_13_SET = (TEST_8_SET | TEST_4_SET | TEST_1_SET),
	TEST_13_TGL = (TEST_8_TGL | TEST_4_TGL | TEST_1_TGL),
	TEST_13_EXT = (TEST_8_EXT | TEST_4_EXT | TEST_1_EXT),
	
	TEST_14_CLR = (TEST_8_CLR | TEST_4_CLR | TEST_2_CLR),
	TEST_14_SET = (TEST_8_SET | TEST_4_SET | TEST_2_SET),
	TEST_14_TGL = (TEST_8_TGL | TEST_4_TGL | TEST_2_TGL),
	TEST_14_EXT = (TEST_8_EXT | TEST_4_EXT | TEST_2_EXT),
	
	TEST_15_CLR = (TEST_8_CLR | TEST_4_CLR | TEST_2_CLR | TEST_1_CLR),
	TEST_15_SET = (TEST_8_SET | TEST_4_SET | TEST_2_SET | TEST_1_SET),
	TEST_15_TGL = (TEST_8_TGL | TEST_4_TGL | TEST_2_TGL | TEST_1_TGL),
	TEST_15_EXT = (TEST_8_EXT | TEST_4_EXT | TEST_2_EXT | TEST_1_EXT),
	
	TEST_16_CLR = (TEST_1_CLR << 8),
	TEST_16_SET = (TEST_1_SET << 8),
	TEST_16_TGL = (TEST_1_TGL << 8),
	TEST_16_EXT = (TEST_1_EXT << 8),
	
	TEST_17_CLR = (TEST_16_CLR | TEST_1_CLR),
	TEST_17_SET = (TEST_16_SET | TEST_1_SET),
	TEST_17_TGL = (TEST_16_TGL | TEST_1_TGL),
	TEST_17_EXT = (TEST_16_EXT | TEST_1_EXT),

	TEST_18_CLR = (TEST_16_CLR | TEST_2_CLR),
	TEST_18_SET = (TEST_16_SET | TEST_2_SET),
	TEST_18_TGL = (TEST_16_TGL | TEST_2_TGL),
	TEST_18_EXT = (TEST_16_EXT | TEST_2_EXT),
	
	TEST_19_CLR = (TEST_16_CLR | TEST_2_CLR | TEST_1_CLR),
	TEST_19_SET = (TEST_16_SET | TEST_2_SET | TEST_1_SET),
	TEST_19_TGL = (TEST_16_TGL | TEST_2_TGL | TEST_1_TGL),
	TEST_19_EXT = (TEST_16_EXT | TEST_2_EXT | TEST_1_EXT),
	
	TEST_20_CLR = (TEST_16_CLR | TEST_4_CLR),
	TEST_20_SET = (TEST_16_SET | TEST_4_SET),
	TEST_20_TGL = (TEST_16_TGL | TEST_4_TGL),
	TEST_20_EXT = (TEST_16_EXT | TEST_4_EXT),
	
	TEST_21_CLR = (TEST_16_CLR | TEST_4_CLR | TEST_1_CLR),
	TEST_21_SET = (TEST_16_SET | TEST_4_SET | TEST_1_SET),
	TEST_21_TGL = (TEST_16_TGL | TEST_4_TGL | TEST_1_TGL),
	TEST_21_EXT = (TEST_16_EXT | TEST_4_EXT | TEST_1_EXT),
	
	TEST_22_CLR = (TEST_16_CLR | TEST_4_CLR | TEST_2_CLR),
	TEST_22_SET = (TEST_16_SET | TEST_4_SET | TEST_2_SET),
	TEST_22_TGL = (TEST_16_TGL | TEST_4_TGL | TEST_2_TGL),
	TEST_22_EXT = (TEST_16_EXT | TEST_4_EXT | TEST_2_EXT),
	
	TEST_23_CLR = (TEST_16_CLR | TEST_4_CLR | TEST_2_CLR | TEST_1_CLR),
	TEST_23_SET = (TEST_16_SET | TEST_4_SET | TEST_2_SET | TEST_1_SET),
	TEST_23_TGL = (TEST_16_TGL | TEST_4_TGL | TEST_2_TGL | TEST_1_TGL),
	TEST_23_EXT = (TEST_16_EXT | TEST_4_EXT | TEST_2_EXT | TEST_1_EXT),
	
	TEST_24_CLR = (TEST_16_CLR | TEST_8_CLR),
	TEST_24_SET = (TEST_16_SET | TEST_8_SET),
	TEST_24_TGL = (TEST_16_TGL | TEST_8_TGL),
	TEST_24_EXT = (TEST_16_EXT | TEST_8_EXT),
	
	TEST_25_CLR = (TEST_16_CLR | TEST_8_CLR | TEST_1_CLR),
	TEST_25_SET = (TEST_16_SET | TEST_8_SET | TEST_1_SET),
	TEST_25_TGL = (TEST_16_TGL | TEST_8_TGL | TEST_1_TGL),
	TEST_25_EXT = (TEST_16_EXT | TEST_8_EXT | TEST_1_EXT),

	TEST_26_CLR = (TEST_16_CLR | TEST_8_CLR | TEST_2_CLR),
	TEST_26_SET = (TEST_16_SET | TEST_8_SET | TEST_2_SET),
	TEST_26_TGL = (TEST_16_TGL | TEST_8_TGL | TEST_2_TGL),
	TEST_26_EXT = (TEST_16_EXT | TEST_8_EXT | TEST_2_EXT),
	
	TEST_27_CLR = (TEST_16_CLR | TEST_8_CLR | TEST_2_CLR | TEST_1_CLR),
	TEST_27_SET = (TEST_16_SET | TEST_8_SET | TEST_2_SET | TEST_1_SET),
	TEST_27_TGL = (TEST_16_TGL | TEST_8_TGL | TEST_2_TGL | TEST_1_TGL),
	TEST_27_EXT = (TEST_16_EXT | TEST_8_EXT | TEST_2_EXT | TEST_1_EXT),
	
	TEST_28_CLR = (TEST_16_CLR | TEST_8_CLR | TEST_4_CLR),
	TEST_28_SET = (TEST_16_SET | TEST_8_SET | TEST_4_SET),
	TEST_28_TGL = (TEST_16_TGL | TEST_8_TGL | TEST_4_TGL),
	TEST_28_EXT = (TEST_16_EXT | TEST_8_EXT | TEST_4_EXT),
	
	TEST_29_CLR = (TEST_16_CLR | TEST_8_CLR | TEST_4_CLR | TEST_1_CLR),
	TEST_29_SET = (TEST_16_SET | TEST_8_SET | TEST_4_SET | TEST_1_SET),
	TEST_29_TGL = (TEST_16_TGL | TEST_8_TGL | TEST_4_TGL | TEST_1_TGL),
	TEST_29_EXT = (TEST_16_EXT | TEST_8_EXT | TEST_4_EXT | TEST_1_EXT),
	
	TEST_30_CLR = (TEST_16_CLR | TEST_8_CLR | TEST_4_CLR | TEST_2_CLR),
	TEST_30_SET = (TEST_16_SET | TEST_8_SET | TEST_4_SET | TEST_2_SET),
	TEST_30_TGL = (TEST_16_TGL | TEST_8_TGL | TEST_4_TGL | TEST_2_TGL),
	TEST_30_EXT = (TEST_16_EXT | TEST_8_EXT | TEST_4_EXT | TEST_2_EXT),
	
	TEST_31_CLR = (TEST_16_CLR | TEST_8_CLR | TEST_4_CLR | TEST_2_CLR | TEST_1_CLR),
	TEST_31_SET = (TEST_16_SET | TEST_8_SET | TEST_4_SET | TEST_2_SET | TEST_1_SET),
	TEST_31_TGL = (TEST_16_TGL | TEST_8_TGL | TEST_4_TGL | TEST_2_TGL | TEST_1_TGL),
	TEST_31_EXT = (TEST_16_EXT | TEST_8_EXT | TEST_4_EXT | TEST_2_EXT | TEST_1_EXT),
	
	LAST_TEST_STATES
} enTEST_STATES;

#endif /* _POST_COMMON_H_ */
