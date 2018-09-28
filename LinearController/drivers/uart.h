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
uint8_t UART_Receive(void);
void UART_InterpretPumpingEffort();

extern volatile uint8_t pumpingEffort;

#endif /* UART_H_ */




#endif /* UART_H_ */