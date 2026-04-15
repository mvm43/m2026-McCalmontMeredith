/*
 * GccApplication1.c
 *
 * Created: 4/10/2026 12:11:38 AM
 * Author : mvmcc
 */ 

#include "max7219.h"

// SPI pins for ATmega328P
#define MOSI PB3
#define SCK  PB5
#define CS   PB2

void SPI_init(void)
{
	DDRB |= (1 << MOSI) | (1 << SCK) | (1 << CS);
	
	// Enable SPI, Master mode, set clock rate fck/16
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

void SPI_send(uint8_t data)
{
	SPDR = data;
	while (!(SPSR & (1 << SPIF)));
}

void MAX7219_send(uint8_t address, uint8_t data)
{
	PORTB &= ~(1 << CS); // CS LOW

	SPI_send(address);
	SPI_send(data);

	PORTB |= (1 << CS);  // CS HIGH
}

void MAX7219_init(void)
{
	MAX7219_send(0x0C, 0x01); // Shutdown register: normal operation
	MAX7219_send(0x09, 0xFF); // Decode mode: BCD for both digits
	MAX7219_send(0x0B, 0x01); // Scan limit: digits 0–1
	MAX7219_send(0x0A, 0x08); // Intensity (brightness)
	MAX7219_send(0x0F, 0x00); // Display test: off
}

void MAX7219_displayNumber(uint8_t num)
{
	uint8_t tens = num / 10;
	uint8_t ones = num % 10;

	// Digit 1 (right)
	MAX7219_send(0x01, ones);

	// Digit 2 (left)
	if (tens > 0)
	MAX7219_send(0x02, tens);
	else
	MAX7219_send(0x02, 0x0F); // blank
}

void MAX7219_clear(void)
{
	MAX7219_send(0x01, 0x0F);
	MAX7219_send(0x02, 0x0F);
}

