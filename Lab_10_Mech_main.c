/*
 * GccApplication17.c
 *
 * Created: 4/16/2026 11:59:42 PM
 * Author : mvmcc
 */ 

#include <avr/io.h>

// -------- FUNCTION PROTOTYPES (from your secondary code) --------
void SPI_init(void);
void MAX7221_init(void);
void MAX7221_display(unsigned char num);
void MAX7221_clear(void);

// ------------------- SIMPLE DELAY -------------------
void delay_ms(unsigned int ms)
{
	for (unsigned int i = 0; i < ms; i++)
	{
		for (volatile unsigned int j = 0; j < 800; j++);
	}
}

// ------------------- MAIN -------------------
#define MONTH 4   // Change for your demo
#define DAY   17

int main(void)
{
	// -------- SPI INITIALIZATION (FROM NOTES) --------
	// Step 1: Set MOSI, SCK, SS as outputs
	DDRB |= (1 << PB3) | (1 << PB5) | (1 << PB2);

	// Step 2: Set SS HIGH (disable secondary initially)
	PORTB |= (1 << PB2);

	// Step 3: Enable SPI in Main mode, Mode 0, MSB first, Fosc/16
	SPI_init();

	// -------- MAX7221 INITIALIZATION --------
	MAX7221_init();

	// -------- MAIN LOOP --------
	while (1)
	{
		// Step 6: Send digit data (MONTH)
		MAX7221_display(MONTH);
		delay_ms(1000);

		// Send DAY
		MAX7221_display(DAY);
		delay_ms(1000);

		// Blank display (do NOT turn off chip per notes)
		MAX7221_clear();
		delay_ms(2000);
	}
}


