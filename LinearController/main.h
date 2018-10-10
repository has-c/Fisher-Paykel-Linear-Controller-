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
#include "drivers/uart.h"



#ifndef MAIN_H_
#define MAIN_H_


extern volatile uint8_t pumpingEffort;
extern volatile uint16_t dutyCycle;
extern volatile bool lowPowerMode;
extern volatile bool changePumpingEffort;
extern volatile uint16_t frequency;
extern volatile uint8_t noOfWaves;

extern volatile uint8_t operatingFrequency;
extern volatile uint8_t appliedVoltage;
extern volatile uint8_t averagePower;

extern volatile char receiveBuffer[100]; //message buffer
extern volatile uint8_t indexCount; //receive buffer index count
extern volatile uint8_t reverseCurlyBracketCount; 
extern volatile bool messageReceived;

extern volatile bool clearErrorFlag;



#endif /* MAIN_H_ */