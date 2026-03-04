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
	while ((ADCSRA & 0b00010000) == 0); // Alternate code: while ((ADCSRA & (1<<ADIF)) ==0); // wait for conversion to finish
	return ADCH;
	
}

void LEDs_off(){
	
	PORTC |= 0b00011111;

}
int main(void)
{
	PRR = 0x00;
	DDRC = 0b00011111;
	LEDs_off();
	ADC_input();
	
    /* Replace with your application code */
    while (1) 
    {
		int val = ADC_con();
		LEDs_off();
		
		if (val < 204)
		{
			PORTC |= (1 << PC0);
		} 
		else if (val < 408)
		{
			PORTC |= (1 << PC1);
		}
		else if (val < 612)
		{
			PORTC |= (1 << PC2);
		}
		else if (val < 816)
		{
			PORTC |= (1 << PC3);
		}
		else
		{
			PORTC |= (1 << PC4);
		}
    }
}
