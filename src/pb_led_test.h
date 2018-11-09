/*******************************************************************************/
/*                                                                             */
/*   (C) Copyright 2008 - Analog Devices, Inc.  All rights reserved.           */
/*                                                                             */
/*    FILE:     pb_led_test.h ( )					  				                   */
/*                                                                             */
/*    CHANGES:  1.00.0  - initial release    								   */
/*																			   */
/*******************************************************************************/

#ifndef _PB_LED_TEST_H_
#define _PB_LED_TEST_H_



/* LED enum */
typedef enum LEDS_tag
{
	LED1 = 0x01,
	LED2 = 0x02,
	LED3 = 0x04,
	LED4 = 0x08,
	LED5 = 0x10,
	LED6 = 0x20,
	LED7 = 0x40,
	LED8 = 0x80,
	LAST_LED = LED8
}enLED;

/* LED states */
#define LED_OFF		0
#define LED_ON		1
#define LED_TOGGLE	2

#define NUM_STATUS_LEDS	5				/* # of LEDs for showing status, 5 gives 32 tests */

/* with X LEDs you have (2^X-1) different patterns, after that you have to reuse patterns */
#define NUM_STATUS_PATTERNS ((int)(pow(2, NUM_STATUS_LEDS) - 1))
																
#define BLINK_FAST			600000
#define BLINK_SLOW			(BLINK_FAST * 2)


/* function prototypes */
void Init_LEDs(void);
void Init_PushButtons(void);

void ClearSet_LED(const int led, const int bState);
void LED_Bar(const int iSpeed);
void LED_Bar_Reverse(const int iSpeed);
void LED_Bar_BacknForth(const int iSpeed);
void Strobe_LEDs(unsigned int pattern_a, unsigned int pattern_b, unsigned int delay, bool *pbDone);
void Blink_LED(const int iSpeed);
void ClearSet_LED_Bank(const int iState);
void Pushbutton_ISR(int sig_int);

#endif /* _PB_LED_TEST_H_ */
