/*******************************
* IP411-Embedded System Project: Stopwatch on 4 digits 7 segment display
* Author: Bruce Chin Yuan Zhen
* Date: 2019.05.15
*
* Description:
* - each segment (a-g) of the display and the cathode for each digit is connected to a GPIO pin set to output
* - The figures 0-9 are defined by Hex code made up of binary 1 and 0 for the respective IO pins
* - counter is incremented every 100ms for the stopwatch to stop time up to a tenth second
* - 7 segment display is updated constantly in a while loop
*/

#include "LPC802.h"
#include "clock_config.h" // for BOARD_BootClockFRO30M(), etc.

#define DIGIT_1 (0) // Digit 1 = first digit of 7 segment display counting from the left
#define DIGIT_2 (1) // 2nd Digit
#define DIGIT_3 (16) // 3rd Digit
#define DIGIT_4 (17) // 4th Digit
#define BUTTON_USER1 (8) // Button on the board connected to PIO_8

/*
#define ZERO (0x1E90)
#define ONE (0x0280)
#define TWO (0x2C90)
#define THREE (0x2690)
#define FOUR (0x3280)
#define FIVE (0x3610)
#define SIX (0x3E10)
#define SEVEN (0x0290)
#define EIGHT (0x3E90)
#define NINE (0x3690)
*/
const int LUT[10] = { 0x1E90, 0x0280, 0x2C90, 0x2690, 0x3280, 0x3610, 0x3E10, 0x0290, 0x3E90, 0x3690 }; // Look-Up Table
#define DOT (0x8000)

// global variables to allow interaction between ISR and main function
int counter = 0;
int state = 0;
int value_digit1 = 0; // values to be captured and saved when stopwatch is stopped
int value_digit2 = 0;
int value_digit3 = 0;
int value_digit4 = 0;

// declaration of functions
void init();
void wait();
void display(int dp, int ones, int tens);

//======================MAIN==========================
int main(void)
{
	init();			// initialization/configuration of GPIO & ISR
	while (1)		// continuous loop
	{
		switch (state) // checks which state the system is in
		{
		case 0: // standby mode
			counter = 0;
			display(0, 0, 0);
			break;
		case 1: // start -> stopwatch run mode
			display(counter % 10, (counter / 10) % 10, (counter / 100) % 10);
			break;
		case 2: // stop mode
			display(value_digit4, value_digit3, value_digit2); // display stopped time
			break;
		default: // to prevent shadow states that were not defined
			state = 0;
		}
	}
	return 0;
}

// ================INITIALIZATION========================
void init()
{
	__disable_irq();				// turn off globally
	NVIC_DisableIRQ(SysTick_IRQn);	// turn off the SysTick interrupt.
	NVIC_DisableIRQ(PIN_INT0_IRQn); // turn off
	SYSCON->MAINCLKSEL = (0x0 << SYSCON_MAINCLKSEL_SEL_SHIFT);
	SYSCON->MAINCLKUEN &= ~(0x1);	// step 1. (Sec. 6.6.4 of manual)
	SYSCON->MAINCLKUEN |= 0x1;		// step 2. (Sec. 6.6.4 of manual)
	BOARD_BootClockFRO24M();		// 24MHz chosen -> 12 MHz timer
	SysTick_Config(1200000);		// 100ms counter

	// GPIO setup
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_GPIO0_MASK |
		SYSCON_SYSAHBCLKCTRL0_GPIO_INT_MASK);
	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK |
		SYSCON_PRESETCTRL0_GPIOINT_RST_N_MASK);		// reset GPIO and GPIO Interrupt
	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK |
		SYSCON_PRESETCTRL0_GPIOINT_RST_N_MASK);		// clear reset (bit=1)
	GPIO->CLR[0] = 0x3BE93;							// clear bits before setting to output
	GPIO->DIRSET[0] = 0x3BE93;						// set desired bits outputs
	GPIO->SET[0] = (1UL << DIGIT_1 | 1UL << DIGIT_2 | 1UL << DIGIT_3 | 1UL << DIGIT_4); // set PIO_0, PIO_1, PIO_16 & PIO_17 to LOW
	GPIO->SET[0] = LUT[0];							// display 0 (value read from Look Up table)
	GPIO->SET[0] = DOT;

	// Set up GPIO IRQ: interrupt channel 0 (PINTSEL0) to GPIO 8
	SYSCON->PINTSEL[0] = BUTTON_USER1;	// PINTSEL0 is P0_8
	PINT->ISEL = 0x00;					// channel 0 bit is 0: is edge sensitive
	PINT->CIENR = 0b00000001;			// disable channel 0 IRQ for rising edge
	PINT->SIENF = 0b00000001;			// enable channel 0 IRQ for falling edge
	PINT->IST = 0xFF;					// each bit set to 1 to remove any pending flag.

	// Enable IRQs
	NVIC_EnableIRQ(PIN_INT0_IRQn);		// GPIO interrupt channel 0
	NVIC_EnableIRQ(SysTick_IRQn);		// SysTick interrupt
	__enable_irq();						// enable global interrupts
}

void wait() // small delay function
{
	int i = 0;
	while (i != 1024) // dummy program to buy time
	{
		i++;
	}
}

//=========DISPLAY function to display value on the 7-segment============
void display(int dp, int ones, int tens)
{
	GPIO->CLR[0] = 0xBE90;				// clear all segments a-g
	GPIO->SET[0] = (1UL << DIGIT_2 | 1UL << DIGIT_3 | 1UL << DIGIT_4); // turn off display for all 4 digits
	GPIO->CLR[0] = (1UL << DIGIT_4);	// activate only the 4th digit (4th from the left)
	GPIO->SET[0] = LUT[dp];				// display decimal point value [0.1s] -> read from look-up table
	wait();								// short delay time before everything is cleared
	GPIO->CLR[0] = 0xBE90;
	GPIO->SET[0] = (1UL << DIGIT_2 | 1UL << DIGIT_3 | 1UL << DIGIT_4);
	GPIO->CLR[0] = (1UL << DIGIT_3);	// activate only the 3rd digit
	GPIO->SET[0] = LUT[ones]; display value of ones[s]
		GPIO->SET[0] = DOT;				// turn segment "." on for decimal point
	wait();
	GPIO->CLR[0] = 0xBE90;
	GPIO->SET[0] = (1UL << DIGIT_2 | 1UL << DIGIT_3 | 1UL << DIGIT_4);
	GPIO->CLR[0] = (1UL << DIGIT_2);	// activate only the 2nd digit
	GPIO->SET[0] = LUT[tens];			// display value of tens [10s]
	wait();
}

//=========SysTick ISR handler===========
void SysTick_Handler(void) // Interrupt is triggered every 100ms
{
	counter++;
}

//=========GPIO ISR handler=============
void PIN_INT0_IRQHandler(void)
{
	if (PINT->IST & (1 << 0))	// check if interrupt requested for channel 0 of GPIO INT
	{
		PINT->IST = (1 << 0);	// remove interrupt flag
		state++;				// move to the next state
		state %= 3;				// set state to 0 when state==3
		if (state == 2)			// stop mode
		{
			value_digit4 = counter % 10; // capture counter value (latch)
			value_digit3 = (counter / 10) % 10;
			value_digit2 = (counter / 100) % 10;
		}
	}
	return;
}