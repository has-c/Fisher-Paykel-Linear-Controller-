
/*
 * uart.h
 *
 * Created: 15/09/2018 5:17:15 PM
 *  Author: hasna
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <avr/power.h>
#include <stdint.h>
#include <stdbool.h>

#define NUMBER_OF_POSSIBLE_ERRORS 2
#define MAX_ARRAY_SIZE 50


#ifndef UART_H_
#define UART_H_

uint8_t UART_ASCIIConversion(uint8_t value);

void UART_Init(unsigned int BAUD_RATE);
void UART_Transmit(uint8_t myValue); 
void UART_InterpretPumpingEffort();

//extern volatile uint8_t pumpingEffort;

//sending parameter information functions 
void UART_SendJson(uint8_t averagePower, uint8_t operatingFrequency, uint32_t appliedVoltage, uint8_t current,bool jamErrorFlag, bool collisionErrorFlag, uint8_t requiredValue, uint8_t currentValue);
void MFCmodulator(uint8_t requiredValue, uint8_t currentValue); //send MFC values 
void VERmodulator(); //sends VER JSON portion
void PARAMmodulator(uint8_t averagePower, uint8_t operatingFrequency, uint32_t appliedVoltage, uint8_t current); //sends parameter json portion
void ERRORmodulator(bool jamErrorFlag, bool collisionErrorFlag); //sends error json portion


#endif /* UART_H_ */
