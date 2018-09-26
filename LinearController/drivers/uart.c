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
#define MAX_LOW_POWER 37885
#define PROPORTIONALITY_CONSTANT 21983

//uart intializer
void UART_Init(unsigned int BAUD_RATE){
	UBRR0H |= (unsigned char)(BAUD_RATE>>8); //sets the baud rate to 9600bps
	UBRR0L |= (unsigned char)(BAUD_RATE);
	UCSR0B |= (1<<TXEN0)|(1<<RXEN0)|(1<<RXCIE0); //enables UART transmitter/reciever and the reciever interrupt
	UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00); //sets character size to 8 data bits
}


//uart transmit
//polling uart
void UART_Transmit(uint8_t myValue){
	while (!((1<<UDRE0)&&(UCSR0A))); //wait until the transmit register is ready
	UDR0 = myValue;//once ready, store next value for transmission
}

//uart recieve 
//interrupt config
uint8_t UART_Receive(){
	uint8_t pumpingEffort = UDR0;
	return pumpingEffort;
}

void UART_InterpretPumpingEffort(){
	if(pumpingEffort==0){ //turn off mode 
		power_all_disable(); //disables all modules on the microcontroller 
		power_usart_enable(); //enable UART for communication to see when to turn back on
	}else if((pumpingEffort>=1)&&(pumpingEffort<=178)){
		//70% of values - care about efficiency and meeting pumpingEffort
		//efficiency actions turn two switches off
		//disable all unused modules
		dutyCycle = (PROPORTIONALITY_CONSTANT* MAX_LOW_POWER * (pumpingEffort/178))/(10000*1000);	//10000 and 1000 are because we didnt use floats 						 
	}else if((pumpingEffort>178)&&(pumpingEffort<=254)){
		//30% of values - go ham fam
	}else{ //255 lose your mind
		//change duty cycle and pwm to max out the motor
	}
}
	