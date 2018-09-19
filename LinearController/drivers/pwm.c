/*
 * pwm.c
 *
 * Created: 15/09/2018 5:17:38 PM
 *  Author: hasna
 */ 

#include "pwm.h"

//to do setup in the Fast PWM mode
void pwm_init(){
	TCCR1B |= (1<<WGM12) | (1<<CS11)|(1<<CS10);//CTC mode & 64 prescaler
	TIMSK1 |= (1<<OCIE1A)|(1<<OCIE1B); //Enabling interrupts for match on OCR1A and OCR1B
	
}

void pwm_control(uint16_t topValue, uint16_t bottomValue){
	OCR1A = topValue;
	OCR1B = bottomValue;
}


