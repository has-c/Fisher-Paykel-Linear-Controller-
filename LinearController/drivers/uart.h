/*
 * uart.h
 *
 * Created: 15/09/2018 5:17:15 PM
 *  Author: hasna
 */ 


#ifndef UART_H_
#define UART_H_

void uart_init(unsigned int BAUD_RATE);
void uart_transmit(uint8_t myValue);
unsigned char USART_receive(void)

#endif /* UART_H_ */