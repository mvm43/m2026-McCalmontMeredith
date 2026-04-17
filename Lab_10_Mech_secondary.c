#include <avr/io.h>

// ------------------- SPI INIT (MODE 0, MASTER) -------------------
void SPI_init(void)
{
	// Set MOSI (PB3), SCK (PB5), SS (PB2) as output
	DDRB |= (1 << PB3) | (1 << PB5) | (1 << PB2);

	// Enable SPI, Master mode, fclk/16, Mode 0
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

// ------------------- SEND COMMAND TO MAX7221 -------------------
void send_to_MAX7221(unsigned char command, unsigned char data)
{
	PORTB &= 0b11111011;   // CS LOW (PB2 = 0)

	SPDR = command;        // Send command byte
	while (!(SPSR & (1 << SPIF)));

	SPDR = data;           // Send data byte
	while (!(SPSR & (1 << SPIF)));

	PORTB |= 0b00000100;   // CS HIGH (PB2 = 1)
}

// ------------------- INITIALIZE MAX7221 -------------------
void MAX7221_init(void)
{
	// Step 2: Scan limit (2 digits ? digits 0 and 1)
	send_to_MAX7221(0x0B, 0x01);

	// Step 3: Decode mode (enable BCD for digits 0 and 1)
	send_to_MAX7221(0x09, 0x03);

	// Step 4: Intensity (0x00 to 0x0F)
	send_to_MAX7221(0x0A, 0x08);

	// Step 5: Turn on display
	send_to_MAX7221(0x0C, 0x01);

	// Disable display test
	send_to_MAX7221(0x0F, 0x00);
}

// ------------------- DISPLAY NUMBER (2 DIGITS) -------------------
void MAX7221_display(unsigned char num)
{
	unsigned char tens = num / 10;
	unsigned char ones = num % 10;

	// Digit 0 (rightmost)
	send_to_MAX7221(0x01, ones);

	// Digit 1 (left)
	if (tens > 0)
	send_to_MAX7221(0x02, tens);
	else
	send_to_MAX7221(0x02, 0x0F); // blank
}

// ------------------- CLEAR DISPLAY -------------------
void MAX7221_clear(void)
{
	send_to_MAX7221(0x01, 0x0F);
	send_to_MAX7221(0x02, 0x0F);
}
