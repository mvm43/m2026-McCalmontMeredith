/*
 * GccApplication3.c
 *
 * Created: 2/27/2026 12:29:33 PM
 * Author : MVM43
 */ 

#include <avr/io.h>
void ADC_input(){
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS2);
}

int  ADC_con(){
	ADCSRA |= (1 << ADSC);
	return ADC;
	
}

void LEDs_off(){
	
	PORTC |= 0b00011111;

}
int main(void)
{
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
		else if (val < 408);
		{
			PORTC |= (1 << PC1);
		}
		else if (val < 612);
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

