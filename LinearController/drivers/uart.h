/*
 * uart.h
 *
 * Created: 17/09/2018 11:28:13 AM
 *  Author: hasna
 */ 


#ifndef UART_H_
#define UART_H_

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


void UART_Init(unsigned int BAUD_RATE);
void UART_Transmit(uint8_t myValue);
uint8_t UART_Receive();
void UART_InterpretPumpingEffort();
extern volatile uint8_t pumpingEffort;

void UART_SendJson(uint8_t averagePower, uint8_t operatingFrequency, uint8_t appliedVoltage, uint8_t current, uint8_t errorArray[NUMBER_OF_POSSIBLE_ERRORS]);
void MFCmodulator(); //sends MFC JSON portion
void VERmodulator(); //sends VER JSON portion
void PARAMmodulator(uint8_t averagePower, uint8_t operatingFrequency, uint8_t appliedVoltage, uint8_t current); //sends parameter json portion
void ERRORmodulator(uint8_t errorArray[NUMBER_OF_POSSIBLE_ERRORS]); //sends error json portion


}

#endif /* UART_H_ */




#endif /* UART_H_ */