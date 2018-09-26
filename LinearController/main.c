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
#include "drivers/ADC.h"
#include "drivers/PWM.h"
#include "drivers/UART.h"

#define F_CPU 8000000UL
#define BAUDRATE 9600
#define UBRRVALUE F_CPU/16/BAUDRATE - 1

#define NUMBER_OF_WAVES 250
#define DEAD_TIME 3000

//global variables
volatile uint8_t count = 0;
volatile bool isDead = false; //indication of whether you are in the deadzone
volatile bool isLHS = true; //true = uses the LHS driver, false = uses the RHS driver
volatile float voltage = 0;
volatile uint8_t pumpingEffort;
volatile uint8_t dutyCycle; 
volatile uint16_t timerDutyCycle; 

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
			TCCR1B |= (1 << CS11); //change prescalar to 8
			PWM_Change(DEAD_TIME,0);
			isLHS = false;
			count = 0;
			isDead = true; //deadzone begins
		}
		else{	//end of deadzone, set the pwm frequency back to normal
			TCCR1B &= ~(1<<CS11);
			isDead = false;
			PWM_Change(400,timerDutyCycle);
		}
	}
	else{//RHS MOTION
		if((count <= NUMBER_OF_WAVES) && (!isDead)){
			PORTD |= (1<< PD6)|(1<<PD5); //NMOS and PMOS on
			count++;
		}
		else if(count > NUMBER_OF_WAVES){//DEADZONE: leave the port off for 14ms in total
			PORTD &= ~(1<< PD6);
			PORTD &= ~(1<<PD5);
			TCCR1B |= (1 << CS11);	//change prescaler to 8
			PWM_Change(DEAD_TIME,0); //apply deadzone
			isLHS = true;
			count = 0;
			isDead = true; //deadzone begins
		}
		else{	//end of deadzone, set the pwm frequency back to normal
			TCCR1B &= ~(1<<CS11);
			isDead = false;
			PWM_Change(400,timerDutyCycle);
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
	pumpingEffort = UART_Receive();
}

 


int main(void)
{	
    sei();
	UART_Init(UBRRVALUE);
	ADC_Init();
	PWM_Init();
	
	//output pin setup
	DDRB |= (1<<PB1)|(1<<PB2);
	DDRD |= (1<<PD5)|(1<<PD6);

    while (1) 
    {
		 UART_InterpretPumpingEffort();
		 ConvertTimerValueToDutyCycle();
    }
	
	return 0;
}

void ConvertTimerValueToDutyCycle(){
	timerDutyCycle = dutyCycle*400;
}