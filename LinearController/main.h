/*
 * main.h
 *
 * Created: 15/09/2018 5:16:31 PM
 *  Author: hasna
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <stdbool.h>
#include <avr/interrupt.h>


#ifndef MAIN_H_
#define MAIN_H_

uint8_t ConvertTimerValueToDutyCycle();
uint16_t CalculateDeadTime();

extern volatile uint8_t pumpingEffort;
extern volatile uint16_t dutyCycle;
extern volatile bool lowPowerMode;
extern volatile bool changePumpingEffort;






#endif /* MAIN_H_ */