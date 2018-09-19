/*
 * pwm.h
 *
 * Created: 15/09/2018 5:17:26 PM
 *  Author: hasna
 */ 

#include <avr/io.h>
#include <stdio.h>


#ifndef PWM_H_
#define PWM_H_

 
void pwm_init();
void pwm_control(uint16_t topValue, uint16_t bottomValue);




#endif /* PWM_H_ */