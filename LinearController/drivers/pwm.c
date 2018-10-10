/*
 * pwm.c
 *
 * Created: 15/09/2018 5:17:38 PM
 *  Author: hasna
 */ 

#include "pwm.h"
#include "../main.h"

#define PWM_FREQUENCY 1000

//to do setup in the Fast PWM mode
void PWM_Init(){
	TCCR1B |= (1<<WGM12)|(1<<CS11)|(1<<CS10);//CTC mode and no prescaler
	TIMSK1 |= (1<<OCIE1A)|(1<<OCIE1B); //Enabling interrupts for match on OCR1A and OCR1B
	
}

void PWM_Change(uint16_t topValue, uint16_t bottomValue){
	OCR1A = topValue;
	OCR1B = bottomValue;
	
}

uint8_t PWM_ConvertTimerValueToDutyCycle(){
	return ((dutyCycle*125)/100);
}

uint16_t PWM_CalculateDeadTime(){
	return (((5000/frequency) - (noOfWaves*(1000/PWM_FREQUENCY))))*125; //in ms
}

