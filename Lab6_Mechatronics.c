/*
 * GccApplication8.c
 *
 * Created: 2/25/2026 1:32:32 PM
 * Author : mvmcc
 */ 

#include <avr/io.h>


//** GLOBAL VARIABLES **
char sensorvalue = 0; // value read from analog sensor (0-255 since we'll only read the upper 8 bits of the 10-bit number)

int main(void)
{
// Setup
	DDRC = 0b00011111; // Sets all pins of Port C to output.

	// Set up ADC
	DDRB =  0x00;  // define all Port B bits as input
	PRR = 0x00;  // clear Power Reduction ADC bit (0) in PRR register
	ADCSRA = 0b10000111; //1<<ADEN | 1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0;  // 0x87 // 0b10000111 // Set ADC Enable bit (7) in ADCSRA register, and set ADC prescaler to 128 (bits 2-0 of ADCSRA = ADPS2-ADPS0 = 111)
	ADMUX = 0b01100000; //0<<REFS1 | 1<<REFS0 | 1<<ADLAR; //0x60; // 0b01100000  // select Analog Reference voltage to be AVcc (bits 7-6 of ADMUX = 01),
						//left justification (bit 5 of ADMUX = ADLAR = 1);
						//and select channel 0 (bits 3-0 of ADMUX = MUX3-MUX0 = 000)
    
	while(1)
    {
    
	// Read analog input
	ADCSRA = ADCSRA | 0b01000000; //Alternate code: ADCSRA |= (1<<ADSC); // Start conversion
	while ((ADCSRA & 0b00010000) == 0); // Alternate code: while ((ADCSRA & (1<<ADIF)) ==0); // wait for conversion to finish
	
	sensorvalue = ADCH; // Keep high byte of 10-bit result (throw away lowest two bits)
	
	//Next wire and program the potentiometer. Test it by echoing the binary value measured out to PORTC as is done in the A/D example program. As
	//you adjust the potentiometer, the LED pattern should change as an
	//inverted binary count (because the LEDs are active low, and note that you
	//will be looking at only five of the ten bits).
	PORTB = ~sensorvalue>>3; // echo results back out PORTC (right-shifted for the demo because I only have 5 LEDs wired (PB0-PB4), so I want to display the high 5 bits; also inverted because of LEDs wired as sinks)

    } // end main while
} // end main

