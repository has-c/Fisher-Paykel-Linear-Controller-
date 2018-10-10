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

 /**************************************************************PWM Declarations**************************************************************/
void PWM_Init();
void PWM_Change(uint16_t topValue, uint16_t bottomValue);
extern uint16_t PWM_CalculateDeadTime();
extern uint8_t PWM_ConvertTimerValueToDutyCycle();




#endif /* PWM_H_ */