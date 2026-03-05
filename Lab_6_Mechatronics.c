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
	ADC_input();
	
    /* Replace with your application code */
    while (1) 
    {
		char val = ADC_con();
		
		if (val < 51)
		{
			PORTC = 0b00011110;
		} 
		else if (val < 102)
		{
			PORTC = 0b00011101;
		}
		else if (val < 153)
		{
			PORTC = 0b00011011;
		}
		else if (val < 204)
		{
			PORTC = 0b00010111;
		}
		else
		{
			PORTC = 0b00001111;
		}
    }
}

