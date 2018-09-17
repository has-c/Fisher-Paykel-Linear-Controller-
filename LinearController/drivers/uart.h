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

#ifndef UART_H_
#define UART_H_

void uart_init(unsigned int BAUD_RATE);
void uart_transmit(uint8_t myValue);
uint8_t uart_receive(void)

#endif /* UART_H_ */




#endif /* UART_H_ */