/*
 * GccApplication18.c
 *
 * Created: 4/17/2026 2:09:19 AM
 * Author : mvmcc
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>

// =======================
// MCP9808 DEFINITIONS
// =======================
#define MCP9808_ADDR 0x18
#define REG_TEMP 0x05
#define REG_RES  0x08

// =======================
// FUNCTION PROTOTYPES
// =======================
void i2c_init(void);
void i2c_start(void);
void i2c_repeated_start(void);
void i2c_stop(void);
void i2c_write_to_address(unsigned char address);
void i2c_read_from_address(unsigned char address);
void i2c_write_data(unsigned char data);
unsigned char i2c_read_data(unsigned char ack);

// MAX7221 (you must already have these)
void SPI_init(void);
void MAX7221_init(void);
void MAX7221_display_int(int value);

// =======================
// I2C FUNCTIONS (FIXED)
// =======================
void i2c_init(void) {
	TWSR = 0x00;
	TWBR = 72;
	TWCR = (1<<TWEN);
}

void i2c_start(void) {
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
}

void i2c_repeated_start(void) {
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
}

void i2c_stop(void) {
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}

void i2c_write_to_address(unsigned char address) {
	unsigned char SLA_W = address << 1;
	while (!(TWCR & (1<<TWINT)));
	TWDR = SLA_W;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
}

void i2c_read_from_address(unsigned char address) {
	unsigned char SLA_R = (address << 1) | 1;
	while (!(TWCR & (1<<TWINT)));
	TWDR = SLA_R;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
}

void i2c_write_data(unsigned char data) {
	while (!(TWCR & (1<<TWINT)));
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
}

unsigned char i2c_read_data(unsigned char ACK) {
	while (!(TWCR & (1<<TWINT)));

	if (ACK) {
		TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
		} else {
		TWCR = (1<<TWINT) | (1<<TWEN);
	}

	while (!(TWCR & (1<<TWINT)));
	return TWDR;
}

// =======================
// MCP9808 FUNCTIONS
// =======================

// Set resolution to 0.0625°C
void mcp9808_set_resolution() {
	i2c_start();
	i2c_write_to_address(MCP9808_ADDR);
	i2c_write_data(REG_RES);
	i2c_write_data(0x03);
	i2c_stop();
}

// Read temperature (Celsius)
float mcp9808_read_temp() {
	unsigned char upper, lower;
	float temp;

	i2c_start();
	i2c_write_to_address(MCP9808_ADDR);
	i2c_write_data(REG_TEMP);

	i2c_repeated_start();
	i2c_read_from_address(MCP9808_ADDR);

	upper = i2c_read_data(1); // ACK
	lower = i2c_read_data(0); // NO ACK

	i2c_stop();

	// Convert (from lab notes)
	upper &= 0x1F;

	if (upper & 0x10) {
		upper &= 0x0F;
		temp = 256 - (upper * 16 + lower / 16.0);
		temp *= -1;
		} else {
		temp = (upper * 16 + lower / 16.0);
	}

	return temp;
}

// Convert to Fahrenheit
float c_to_f(float c) {
	return (c * 9.0 / 5.0) + 32.0;
}

// =======================
// MAIN
// =======================
int main(void) {

	float tempC, tempF;
	int display_val;

	i2c_init();
	SPI_init();
	MAX7221_init();

	mcp9808_set_resolution();

	while (1) {

		tempC = mcp9808_read_temp();
		tempF = c_to_f(tempC);

		// Convert to integer for display (e.g., 72.5 ? 725)
		display_val = (int)(tempF * 10);

		MAX7221_display_int(display_val);

		wait(1000);
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

