/*
 * GccApplication7.c
 *
 * Created: 2/20/2026 2:51:16 AM
 * Author : mvmcc
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define FREQ_CLK 1000000

// Declare Variables
volatile present_led = 0;
volatile led6 = 0;
volatile led7 = 0;
volatile int0_int = 0;
volatile int1_int = 0;

ISR(INT0_vect) {
	// Only valid if LED2 is on and LED6 not active
	if (present_led == 2 && !led6) {
		led6 = 1;
		int0_int = 1;
		PORTB = 0b00000001;  // Turn on LED6, reminder LED6 is connected to Port B and not Port C
	}
}

ISR(INT1_vect) {
	// Only valid if LED6 is active AND LED5 is currently on
	if (led6 && present_led == 5 && !led7) {
		led7 = 1;
		int1_int = 1;
		PORTB |= 0b00000010;  // Turn on LED7
	}
}

int main(void)
{
	DDRD = 0b00000000; 
	DDRC = 0b11111111; // set bits PORTC as output (only need PC2 and PC3
	// as output to control the LEDs)
	PORTC = 0b11111111; // set all bits of PORTC high to turn off LEDs
	// (assumes circuit is wired as active low)
	// Set up Interrupts
	EICRA = 1<<ISC01 | 1<<ISC00 | 1<<ISC11 | 1<<ISC10; // Trigger rising edge
	EIMSK = 1<<INT1 | 1<<INT0; // Enable INT1 and INT0
	sei(); //Enable Global Interrupt

    
    while (1) 
    {
		if (led7) {
			_delay_ms(3000); //Starts the indicated 3 second delay for when led 7 is on

			// Reset everything as LEDs 6 and 7 are on
			PORTC = 0x00;
			led6 = 0; //Reset for LED 6
			led7 = 0; // REset for LED 7
			int0_int = 0;
			int1_int = 0;
			present_led = 0;

    }
	PORTC &= 0xFF //Clear 7 and 6 LEDs (Reset)
	
	if (!led6) {
		// Normal scroll PC0–PC5
		PORTC |= (1 << present_led); //Has LEDs restart

		_delay_ms(200); //delay not longer than 3 seconds

		present_led++; //will start the scroll effect
		if (present_led > 5)
		present_led = 0;
	}
	else {
		// LED6 active so the code will through scroll PC3–PC5 only
		if (present_led < 3)
		present_led = 3; //Sets to LED 3

		PORTC |= (1 << present_led); //Resets PORTC so present_led is reset

		_delay_ms(200);//delay no long than 3 seconds

		present_led++;////will start the scroll by counting up
		if (present_led > 5) //stops at LED 5
		present_led = 3;
	}
	
}

