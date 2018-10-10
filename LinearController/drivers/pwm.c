/*
 * pwm.c
 *
 * Created: 15/09/2018 5:17:38 PM
 *  Author: hasna
 */ 

#include "pwm.h"
#include "../main.h"

/**************************************************************Macros**************************************************************/
#define PWM_FREQUENCY 1000

/**************************************************************PWM Intialisation**************************************************************/
void PWM_Init(){
	TCCR1B |= (1<<WGM12)|(1<<CS11)|(1<<CS10);												//CTC mode and no prescaler
	TIMSK1 |= (1<<OCIE1A)|(1<<OCIE1B); 														//Enabling interrupts for match on OCR1A and OCR1B (trigger points)
	
}

/**************************************************************PWM Intialisation**************************************************************/
void PWM_Change(uint16_t topValue, uint16_t bottomValue){									//Allows us to change the PWM Waveform by changing the trigger points OCR1A and OCR1B
	OCR1A = topValue;
	OCR1B = bottomValue;
	
}

/**************************************************************PWM Conversion**************************************************************/
uint8_t PWM_ConvertTimerValueToDutyCycle(){													//Converts the timer value to a more user readable duty-cycle value 
	return ((dutyCycle*125)/100);
}

/**************************************************************PWM Dead-Time**************************************************************/
uint16_t PWM_CalculateDeadTime(){															//Calculates dead-zone to set up the PWM waveform 
	return (((5000/frequency) - (noOfWaves*(1000/PWM_FREQUENCY))))*125; 
}

