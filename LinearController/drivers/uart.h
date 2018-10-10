
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


#ifndef UART_H_
#define UART_H_


/**************************************************************UART Helper Functions**************************************************************/
uint8_t UART_ASCIIConversion(uint8_t value);

/**************************************************************UART Functions**************************************************************/
void UART_Init(unsigned int BAUD_RATE);
void UART_Transmit(uint8_t myValue); 
void UART_InterpretPumpingEffort();

/**************************************************************UART JSON Functions**************************************************************/
void UART_SendJson(uint8_t averagePower, uint8_t operatingFrequency, uint32_t appliedVoltage, uint8_t current,bool jamErrorFlag, bool collisionErrorFlag, uint8_t requiredValue, uint8_t currentValue);
void MFCmodulator(uint8_t requiredValue, uint8_t currentValue); 
void VERmodulator(); 
void PARAMmodulator(uint8_t averagePower, uint8_t operatingFrequency, uint32_t appliedVoltage, uint8_t current); 
void ERRORmodulator(bool jamErrorFlag, bool collisionErrorFlag); 


#endif /* UART_H_ */
