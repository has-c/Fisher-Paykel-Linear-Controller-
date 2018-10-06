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
#include <avr/power.h>

#include "main.h"
#include "drivers/adc.h"
#include "drivers/pwm.h"
#include "drivers/uart.h"

#define F_CPU 8000000
#define BAUDRATE 9600
#define UBRR_VALUE F_CPU/16/BAUDRATE - 1

#define PWM_FREQUENCY 1000
#define NUMBER_OF_POSSIBLE_ERRORS 2 //jam, collision
#define NUMBER_OF_SAMPLES 10
#define MAX_JSON_SIZE 

//global variables
volatile uint8_t count = 0;
volatile bool isDead = false; //indication of whether you are in the deadzone
volatile bool isLHS = true; //true = uses the LHS driver, false = uses the RHS driver
volatile bool lowPowerMode = true; //false = high power mode, bidirectional and true = low power mode which is single sided movement 
volatile uint8_t pumpingEffort = 0;
volatile uint16_t timerDutyCycle; 
volatile bool changePumpingEffort = true;
volatile bool pumpingIsOccurring = true;
volatile uint8_t frequency = 65; //note need to divide the freq by half when using the low power mode, also divide everything by 10
volatile uint8_t noOfWaves = 32;
volatile uint32_t dutyCycle = 10;
volatile bool transmitParameters = true;
volatile unsigned char value = 0;


//receive global variable
volatile char receiveBuffer[100]; //100 is just a guess
volatile uint8_t indexCount = 0;
volatile uint8_t reverseCurlyBracketCount = 0;
volatile bool messageReceived = false;

volatile char errorArray[NUMBER_OF_POSSIBLE_ERRORS]; //J = Jam , C = Collision
volatile uint8_t current[NUMBER_OF_SAMPLES];
volatile uint8_t currentIndex = 0;

volatile uint8_t voltageLHS[NUMBER_OF_SAMPLES];
volatile uint8_t voltageLHSIndex = 0;

volatile uint8_t voltageRHS[NUMBER_OF_SAMPLES];
volatile uint8_t voltageRHSIndex = 0;

volatile uint8_t operatingFrequency = 0;
volatile uint8_t appliedVoltage = 0;
volatile uint8_t averagePower = 0;

//adc arrays



ISR(USART_RX_vect){
	//disable transmitter 
	//UCSR0B &= ~(1<<TXEN0);
	//UCSR0B &= ~(1<<TXCIE0);
	
	PORTD |= (1<<PD3);
	
	//put received character inside the receive array
	//receiveBuffer[indexCount] = UDR0;
	//if(receiveBuffer[indexCount]=='}'){
		//reverseCurlyBracketCount++;
	//}
	//indexCount++; //increase indexCount 
	//if(reverseCurlyBracketCount == 3){
		//messageReceived = true;
	//}
}

//disable recieve during transmission
//ISR(USART_TX_vect){
	////enable receive 
	//UCSR0B |= (1<<RXEN0);
	//UCSR0B |= (1<<RXCIE0);
//}

//adc arrays

//on pcb pair one NMOS - PD5, PMOS - PB2
//				  NMOS - PD6, PMOS - PB1
 
ISR(TIMER1_COMPA_vect){
	if(isLHS || lowPowerMode){	//LHS MOTION
		if((count <= noOfWaves) && (!isDead)){//PRODUCING X NUMBER OF PWM OSCILLATIONS
			PORTB |= (1<< PB2); //turn RHS ON
			PORTD |= (1<<PD5); 
			count++;
		}
		else if(count > noOfWaves){//DEADZONE: leave the port off for 14ms in total
			PORTD &= ~(1<<PD5); //turn pmos off
			PORTB &= ~(1<<PB2);//turn nmos off
			PWM_Change(CalculateDeadTime(),65535);
			if(!lowPowerMode){
				isLHS = false;
			}
			count = 0;
			isDead = true; //deadzone begins
		}
		else{	//end of deadzone, set the pwm frequency back to normal
			isDead = false;
			PWM_Change(125,ConvertTimerValueToDutyCycle());
		}
	}
	else{//RHS MOTION
		if((count <= noOfWaves) && (!isDead)){
			PORTD |= (1<< PD6); //NMOS and PMOS on
			PORTB |= (1<<PB1);
			count++;
		}
		else if(count > noOfWaves){//DEADZONE: leave the port off for 14ms in total
			PORTD &= ~(1<< PD6);
			PORTB &= ~(1<<PB1);
			PWM_Change(CalculateDeadTime(),65535); //apply deadzone
			isLHS = true;
			count = 0;
			isDead = true; //deadzone begins
		}
		else{	//end of deadzone, set the pwm frequency back to normal
			isDead = false;
			PWM_Change(125,ConvertTimerValueToDutyCycle());
		}
		 
	}
}

ISR(TIMER1_COMPB_vect){//TRIGGERS ON MATCH WITH OCRB REGISTER (OFF TIME)
	if(isLHS || lowPowerMode){//LHS MOTION
		if((~isDead) && (count <=noOfWaves)){
			PORTB &= ~(1 << PB2);
		}
	}
	else{//RHS MOTION
		if((~isDead) && (count <=noOfWaves)){
			PORTB &= ~(1 << PB1);
		}
	}
}



uint8_t ConvertTimerValueToDutyCycle(){
	return ((dutyCycle*125)/100);
}

uint16_t CalculateDeadTime(){
	return (((5000/frequency) - (noOfWaves*(1000/PWM_FREQUENCY))))*125; //in ms
}

uint8_t ASCIIConversion(uint8_t value){
	uint8_t asciiValue = value + 48;
	return asciiValue;
}

int main(void)
{	
	

	//clock_prescale_set(clock_div_2);
    sei();
	UART_Init(UBRR_VALUE);
	ADC_Init();
	PWM_Init();
	
	//output pin setup
	DDRB |= (1<<PB1)|(1<<PB2);
	DDRD |= (1<<PD5)|(1<<PD6);
	DDRD |= (1<<PD3);

	//UART_SendJson(12, 15, voltage, 20, false, true, true, 120,123);

    while (1) 
    {
	
		if(changePumpingEffort){
			 //UART_InterpretPumpingEffort();
		}
		//UART_SendJson(12, 15, value, 20, false, true, true, 120,123);
		if(currentIndex < NUMBER_OF_SAMPLES){
			current[currentIndex] = ADC_Current();
			UART_SendJson(0, currentIndex, 0, current[currentIndex], false, true, true, 120,123);
			currentIndex++;
		}
		
		if(voltageLHSIndex < NUMBER_OF_SAMPLES){
			voltageLHS[voltageLHSIndex] = ADC_LHSVoltage();
			UART_SendJson(1, voltageLHSIndex, voltageLHS[voltageLHSIndex], 0, false, true, true, 120,123);
			if(voltageLHS[voltageLHSIndex]>100){
			}
			voltageLHSIndex++;
		}
		
		if(voltageRHSIndex < NUMBER_OF_SAMPLES){
			voltageRHS[voltageRHSIndex] = ADC_RHSVoltage();
			UART_SendJson(2, voltageRHSIndex, 0, voltageRHS[voltageRHSIndex], false, true, true, 120,123);
			voltageRHSIndex++;
		}
			
    }
	return 0;
}

