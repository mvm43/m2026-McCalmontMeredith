/*
 * GccApplication3.c
 *
 * Created: 2/27/2026 12:29:33 PM
 * Author : MVM43
 */ 

#include <avr/io.h>
void ADC_input(){
	ADMUX = 0b01100101;
	ADCSRA = (1 << ADEN) | (1 << ADPS1) | (1 << ADPS0);
}

int  ADC_con(){
	// Read analog input
	ADCSRA = ADCSRA | 0b01000000; //Alternate code: ADCSRA |= (1<<ADSC); // Start conversion
	while ((ADCSRA & 0b00010011) == 0); // Alternate code: while ((ADCSRA & (1<<ADIF)) ==0); // wait for conversion to finish
	return ADCH;
	
}

int main(void)
{
	PORTC = 0x00;
	PRR = 0x00;
	DDRC = 0b00011111;
	
	//From Clark's code
	DDRD = 1<<PD6 | 1<<PD5;// Make OC0A (PD6) and OC0B (PD5) output bits -- these are the PWM pins;
	//DDRC = 0b00000011; // Make PC0 and PC1 output pins //CHANGE THIS
	
	OCR0A = 0x00;       // Load $00 into OCR0 to set initial duty cycle to 0 (motor off)
	TCCR0A = 0b10000011; //1<<COM0A1 | 0<<COM0A0 | 1<<WGM01 | 1<<WGM00;      // Set non-inverting mode on OC0A pin (COMA1:0 = 10; Fast PWM (WGM1:0 bits = bits 1:0 = 11) (Note that we are not affecting OC0B because COMB0:1 bits stay at default = 00)
	TCCR0B = 0b00000011; //0<<CS02 | 1<<CS01 | 1<<CS00; // Set base PWM frequency (CS02:0 - bits 2-0 = 011 for prescaler of 64, for approximately 1kHz base frequency)
	// PWM is now running on selected pin at selected base frequency.  Duty cycle is set by loading/changing value in OCR0A register.

	PORTD = 0b00000001; // Set forward direction
	
	ADC_input();
	
	
    /* Replace with your application code */
    while (1) 
    {
		char val = ADC_con();
		
		if (val < 51)
		{
			PORTC = 0b00011110;
			PORTD = 0b00000010; //Motor backward
			OCR0A = 85;
		} 
		else if (val < 102)
		{
			PORTC = 0b00011101;
			PORTD = 0b00000010;
			OCR0A = 43;
		}
		else if (val < 153)
		{
			PORTC = 0b00011011;
			PORTD = 0x00;
			OCR0A = 20;
		}
		else if (val < 204)
		{
			PORTC = 0b00010111;
			PORTD = 0b00000001;
			OCR0A = 43;
		}
		else
		{
			PORTC = 0b00001111;
			PORTD = 0b00000001;
			OCR0A = 85; 
		}
    }
}


