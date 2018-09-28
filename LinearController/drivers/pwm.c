/*
 * pwm.c
 *
 * Created: 15/09/2018 5:17:38 PM
 *  Author: hasna
 */ 

#include "pwm.h"

//to do setup in the Fast PWM mode
void PWM_Init(){
	TCCR1B |= (1<<WGM12)|(1<<CS12);//CTC mode and no prescaler
	TIMSK1 |= (1<<OCIE1A)|(1<<OCIE1B); //Enabling interrupts for match on OCR1A and OCR1B
	
}

void PWM_Change(uint16_t topValue, uint16_t bottomValue){
	OCR1A = topValue;
	OCR1B = bottomValue;
	
}


