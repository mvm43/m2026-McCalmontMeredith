/*
 * GccApplication13.c
 *
 * Created: 4/9/2026 11:50:41 PM
 * Author : mvmcc
 */ 

#ifndef MAX7219_H
#define MAX7219_H


#include <avr/io.h>


#include "max7219.h"


void SPI_init(void);
void SPI_send(uint8_t data);

void MAX7219_init(void);
void MAX7219_send(uint8_t address, uint8_t data);
void MAX7219_displayNumber(uint8_t num);
void MAX7219_clear(void);


// Change these to your demo date
#define MONTH 4   // April
#define DAY   10   // 9th

int main(void)
{
	SPI_init();
	MAX7219_init();

	while (1)
	{
		// Display Month (as 0x format ? just number here)
		MAX7219_displayNumber(MONTH);
		wait(1000);

		// Display Day
		MAX7219_displayNumber(DAY);
		wait(1000);

		// Blank display
		MAX7219_clear();
		wait(2000);
	}
}

void wait(volatile int number_of_msec) {
	// This subroutine creates a delay equal to number_of_msec*T, where T is 1 msec
	// It changes depending on the frequency defined by FREQ_CLK
	char register_B_setting;
	char count_limit;
	
	// Some typical clock frequencies:
	switch(FREQ_CLK) {
		case 16000000:
		register_B_setting = 0b00000011; // this will start the timer in Normal mode with prescaler of 64 (CS02 = 0, CS01 = CS00 = 1).
		count_limit = 250; // For prescaler of 64, a count of 250 will require 1 msec
		break;
		case 8000000:
		register_B_setting =  0b00000011; // this will start the timer in Normal mode with prescaler of 64 (CS02 = 0, CS01 = CS00 = 1).
		count_limit = 125; // for prescaler of 64, a count of 125 will require 1 msec
		break;
		case 1000000:
		register_B_setting = 0b00000010; // this will start the timer in Normal mode with prescaler of 8 (CS02 = 0, CS01 = 1, CS00 = 0).
		count_limit = 125; // for prescaler of 8, a count of 125 will require 1 msec
		break;
	}
	
	while (number_of_msec > 0) {
		TCCR0A = 0x00; // clears WGM00 and WGM01 (bits 0 and 1) to ensure Timer/Counter is in normal mode.
		TCNT0 = 0;  // preload value for testing on count = 250
		TCCR0B =  register_B_setting;  // Start TIMER0 with the settings defined above
		while (TCNT0 < count_limit); // exits when count = the required limit for a 1 msec delay
		TCCR0B = 0x00; // Stop TIMER0
		number_of_msec--;
	}
} // end wait()

