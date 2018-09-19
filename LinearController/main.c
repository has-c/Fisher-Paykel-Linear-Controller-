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
#define BAUDRATE 9600
#define UBRRVALUE F_CPU/16/BAUDRATE - 1

#define NUMBER_OF_WAVES 250
#define DUTY_CYCLE 80
#define DEAD_TIME 60000

//global variables
volatile uint8_t count = 0;
volatile bool isDead = false; //indication of whether you are in the deadzone
volatile bool isLHS = true; //true = uses the LHS driver, false = uses the RHS driver
volatile float voltage = 0;
volatile uint8_t strokeLength;

//************************************ Interrupt Service Routines *******************************//

ISR(TIMER1_COMPA_vect){
	if(isLHS){	//LHS MOTION
		if((count <= NUMBER_OF_WAVES) && (!isDead)){//PRODUCING X NUMBER OF PWM OSCILLATIONS
			PORTB |= (1<<PB1) | (1<< PB2); //turn RHS ON
			count++;
		}
		else if(count > NUMBER_OF_WAVES){//DEADZONE: leave the port off for 14ms in total
			PORTB &= ~(1<<PB1); //turn pmos off
			PORTB &= ~(1<<PB2);//turn nmos off
			TCCR1B |= (1 << CS11)|(1<<CS10); //change prescalar to 8
			//TCCR1B &= ~(1<<CS10);
			pwm_control(3000,0);
			isLHS = false;
			count = 0;
			isDead = true; //deadzone begins
		}
		else{	//end of deadzone, set the pwm frequency back to normal
			PORTB &= ~(1<<PORTB1);
			PORTB &= ~(1<<PORTB2);
			//TCCR1B |= (1 << CS10);	//re-adjust prescaler
			TCCR1B &= ~(1<<CS11);
			isDead = false;
			pwm_control(400,DUTY_CYCLE);
		}
	}
	else{//RHS MOTION
		if((count <= NUMBER_OF_WAVES) && (!isDead)){
			PORTD |= (1<< PD6); //PMOS on
			PORTD |= (1<<PD5); //NMOS on
			count++;
		}
		else if(count > NUMBER_OF_WAVES){//DEADZONE: leave the port off for 14ms in total
			PORTD &= ~(1<< PD6);
			PORTD &= ~(1<<PD5);
			TCCR1B |= (1 << CS11)|(1<<CS10);	//change prescaler to 8
			pwm_control(3000,0); //apply deadzone
			isLHS = true;
			count = 0;
			isDead = true; //deadzone begins
		}
		else{	//end of deadzone, set the pwm frequency back to normal
			PORTD &= ~(1<< PD6);
			PORTD &= ~(1<<PD5);
			//TCCR1B |= (1 << CS10);	//re-adjust prescaler
			TCCR1B &= ~(1<<CS11);
			isDead = false;
			pwm_control(400,DUTY_CYCLE);
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
			PORTD &= ~(1 << PD6);
		}
	}
}
 
ISR(USART_RX_vect){
	strokeLength = uart_receive();
}


int main(void)
{	
    sei();
	uart_init(UBRRVALUE);
	adc_init();
	pwm_init();
	
	//output pin setup
	DDRB |= (1<<PB1)|(1<<PB2);
	DDRD |= (1<<PD5)|(1<<PD6);


	
	
    while (1) 
    {
		
    }
}

