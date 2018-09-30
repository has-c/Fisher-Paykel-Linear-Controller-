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

#define NUMBER_OF_POSSIBLE_ERRORS 2

extern volatile uint8_t pumpingEffort;
extern volatile uint32_t dutyCycle;
extern volatile bool lowPowerMode;
extern volatile bool changePumpingEffort;

extern volatile char errorArray[NUMBER_OF_POSSIBLE_ERRORS]; //J = Jam , C = Collision
extern volatile uint8_t current;
extern volatile uint8_t operatingFrequency;
extern volatile uint8_t appliedVoltage;
extern volatile uint8_t averagePower;



#endif /* MAIN_H_ */