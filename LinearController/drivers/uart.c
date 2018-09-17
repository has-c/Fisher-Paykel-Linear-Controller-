/*
 * uart.c
 *
 * Created: 17/09/2018 11:29:15 AM
 *  Author: hasna
 */ 

/*
 * uart.c
 *
 * Created: 15/09/2018 5:16:55 PM
 *  Author: hasna
 */ 

#include "uart.h"

//uart intializer
void uart_init(unsigned int BAUD_RATE){
	UBRR0H |= (unsigned char)(BAUD_RATE>>8); //sets the baud rate to 9600bps
	UBRR0L |= (unsigned char)(BAUD_RATE);
	UCSR0B |= (1<<TXEN0)|(1<<RXEN0)|(1<<RXCIE0); //enables UART transmitter/reciever and the reciever interrupt
	UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00); //sets character size to 8 data bits
}


//uart transmit
//polling uart
void uart_transmit(uint8_t myValue){
	while (!((1<<UDRE0)&&(UCSR0A))); //wait until the transmit register is ready
	UDR0 = myValue;//once ready, store next value for transmission
}

//uart recieve 
//interrupt config
uint8_t uart_receive(){
	uint8_t uartStrokeLength = UDR0;
	return uartStrokeLength;
}



