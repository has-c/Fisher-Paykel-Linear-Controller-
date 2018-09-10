/*
* LC_UART.c
*
* Created: 28-Aug-18 3:20:37 PM
* Author : Abin
*/


#define F_CPU 8000000UL
#define BAUD 9600
#include <avr/io.h>
#include <stdio.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BAUD_INPUT F_CPU/16/BAUD - 1
#include "prototype.h"

//Output MACROS
#define PWM_ON PORTD |= (1<<2)
#define PWM_OFF PORTD &= ~(1<<2)
#define NUMBER_OF_WAVES 250
#define DUTY_CYCLE 360

//global variables
volatile uint8_t count = 0;
volatile bool isDead = false; //indication of whether you are in the deadzone
volatile bool isLHS = true; //true = uses the LHS driver, false = uses the RHS driver
volatile float voltage = 0;


/**************************************************PRINTF FUNCTIONS ***********************************************/

int usart_putchar_printf(char var, FILE *stream){
	if(var== '\n') UART_transmit('\r');
	UART_transmit(var);
	return 0;
}

static FILE mystdout = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);


//************************************ Interrupt Service Routines *******************************//

ISR(TIMER1_COMPA_vect){
	if(isLHS){	//LHS MOTION
		if((count <= NUMBER_OF_WAVES) && (!isDead)){//PRODUCING X NUMBER OF PWM OSCILLATIONS
			PORTB |= (1<<PB1);
			count++;
			}
		else if(count > NUMBER_OF_WAVES){//DEADZONE: leave the port off for 14ms in total
			PORTB &= ~(1<<PB1); //turn the port off
			TCCR1B |= (1 << CS11); //change prescalar to 8
			TCCR1B &= ~(1<<CS10);
			PWM_control(60000,0);
			isLHS = false;
			count = 0;
			isDead = true; //deadzone begins
			}
		else{	//end of deadzone, set the pwm frequency back to normal
			PORTB &= ~(1<<PORTB1);
			TCCR1B |= (1 << CS10);	//re-adjust prescaler
			TCCR1B &= ~(1<<CS11);
			isDead = false;
			PWM_control(400,DUTY_CYCLE);
		}
		}
	else{//RHS MOTION
		if((count <= NUMBER_OF_WAVES) && (!isDead)){
			PORTB |= (1<< PB2);
			count++;
			}
		else if(count > NUMBER_OF_WAVES){//DEADZONE: leave the port off for 14ms in total
			PORTB &= ~(1<< PB2); 
			TCCR1B |= (1 << CS11);	//change prescaler to 8
			TCCR1B &= ~(1<<CS10);
			PWM_control(60000,0); //apply deadzone
			isLHS = true;
			count = 0;
			isDead = true; //deadzone begins
			}
		else{	//end of deadzone, set the pwm frequency back to normal
			PORTB &= ~(1<< PB2);
			TCCR1B |= (1 << CS10);	//re-adjust prescaler
			TCCR1B &= ~(1<<CS11);
			isDead = false;
			PWM_control(400,DUTY_CYCLE);
		}
	}
}

ISR(TIMER1_COMPB_vect){//TRIGGERS ON MATCH WITH OCRB REGISTER (OFF TIME)
	if(isLHS){//LHS MOTION
		if((~isDead) && (count <=NUMBER_OF_WAVES)){
			PORTB &= ~(1 << PB1);
		}
		}
	else{//RHS MOTION
		if((~isDead) && (count <=NUMBER_OF_WAVES)){
			PORTB &= ~(1 << PB2);
		}
	}
}



/**************************************************MAIN FUNCTION ***********************************************/
int main(void)
{
	
	//SETTING UP REGISTERS
	DDRB |= (1<<PB1);
	DDRB |= (1<<PB2);
	PORTB &= ~(1<<PB1);
	PORTB &= ~(1<< PB2);

	
//	stdout= &mystdout;//printf

	//FUNCTION INTIALIZATIONS
	//PWM_init();
	UART_init(BAUD_INPUT);
	//ADC_init();
//	sei();
	
//	PWM_control(400,DUTY_CYCLE); //25kHz with 20% duty cycle
	
	while (1)
	{
		ADCSRA |= (1<<ADSC);
	//	voltage = adc_calculation();
		UART_transmit(48);
		_delay_ms(50);
		UART_transmit(10);
	}
}


