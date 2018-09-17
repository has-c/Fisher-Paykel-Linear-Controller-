/*
 * LinearController.c
 *
 * Created: 15/09/2018 5:15:32 PM
 * Author : hasnain
 */ 


#include <avr/io.h>
#include <stdio.h>
#include <stdbool.h>
#include <avr/interrupt.h>

#include "main.h"
#include "drivers/uart.h"
#include "drivers/adc.h"
#include "drivers/pwm.h"

#define F_CPU 8000000UL
#define BAUD 9600
#define BAUD_INPUT F_CPU/16/BAUD - 1


volatile uint8_t strokeLength;

ISR(USART_RX_vect){
	strokeLength = uart_receive();
}


int main(void)
{
    sei();
	
    while (1) 
    {
		
    }
}

