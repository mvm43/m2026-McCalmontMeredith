/*
 * GccApplication1.c
 *
 * Created: 2/19/2026 9:47:01 PM
 * Author : MVM43
 */ 

#include <avr/io.h>
#define FREQ_CLK 1000000 //Default Atmel clock frequency

// FUNCTIONS //
void wait(volatile int);
void change_led_state(char);

//Note for myself:
// Within the code have the main section to set up the lab for an indefinite amount of time with switch off and then a void section for the cases of the pattern and to clarify the wait time lengths
//NEED the wait section

int main(void)
{
    //Declare the number of states for the lab4 pattern
	int num_states = 22; // there's 22 states including the 2 second off at end, this is noted as e in the pattern array
	char pattern[] = {'A','C','B','C','A','C','A','D','A','D','B','C','B','C','A','D','B','C','B','C','B','E'}
		
	// Set inputs and outputs
	DDRD = 0b00000000;
	DDRC = 0b00000011;
	PORTC = 0b00000011; 
	
	//while loop for when switch is pressed so the LEDs are off
	while (PIND & 0b00000100)
	{
	}
	
    while (1) 
    {
	for (int i=0; i<num_states; i++)
	{
		change_led_state(pattern[i]);
	}
    }
	
	return(0);
}

void change_led_state(char new_state)
{
	switch(new_state) {
		case 'A': 
			PORTC = 0b00000000;
			wait(200);
			break;
		case 'B':
			PORTC = 0b00000000;
			wait(600);
			break;
		case 'C':
			PORTC = 0b00000011;
			wait(200);
			break;
		case 'D':
			PORTC = 0b00000011;
			wait(600)
			break;
		default: 
			PORTC = 0b00000011;
			wait(2000);
	}
}


