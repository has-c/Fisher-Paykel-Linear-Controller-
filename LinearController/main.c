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
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <util/delay.h>

#include "main.h"
#include "drivers/adc.h"
#include "drivers/pwm.h"
#include "drivers/uart.h"

#define F_CPU 8000000
#define BAUDRATE 9600
#define UBRR_VALUE F_CPU/16/BAUDRATE - 1

#define PWM_FREQUENCY 1000
#define NUMBER_OF_POSSIBLE_ERRORS 2 //jam, collision
#define NUMBER_OF_SAMPLES 130
#define MAX_JSON_SIZE 

//global variables
volatile uint8_t count = 0;
volatile bool isDead = false; //indication of whether you are in the deadzone
volatile bool isLHS = true; //true = uses the LHS driver, false = uses the RHS driver
volatile bool lowPowerMode = true; //false = high power mode, bidirectional and true = low power mode which is single sided movement 
volatile uint8_t pumpingEffort = 0;
volatile uint16_t timerDutyCycle; 
volatile bool changePumpingEffort = false;
volatile bool pumpingIsOccurring = true;
volatile uint16_t frequency = 65; //note need to divide the freq by half when using the low power mode, also divide everything by 10
volatile uint8_t noOfWaves = 32;
volatile uint32_t dutyCycle = 50;
volatile bool transmitParameters = true;
volatile unsigned char value = 0;


//receive global variable
volatile char receiveBuffer[100]; //100 is just a guess
volatile uint8_t indexCount = 0;
volatile uint8_t reverseCurlyBracketCount = 0;
volatile bool messageReceived = false;

volatile uint8_t current[NUMBER_OF_SAMPLES];
volatile uint8_t currentIndex = 0;

volatile uint32_t voltageLHS[NUMBER_OF_SAMPLES];
volatile uint8_t voltageLHSIndex = 0;

volatile uint32_t voltageRHS[NUMBER_OF_SAMPLES];
volatile uint8_t voltageRHSIndex = 0;

volatile int voltageAcrossTheCoil[NUMBER_OF_SAMPLES];

volatile uint8_t operatingFrequency = 0;
volatile uint8_t appliedVoltage = 0;
volatile uint8_t averagePower = 0;

volatile uint32_t tempParam = 0;

//adc arrays
int usart_putchar_printf(char var, FILE *stream){
	if(var== '\n') UART_Transmit('\r');
	UART_Transmit(var);
	return 0;
}

static FILE mystdout = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);


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
	stdout= &mystdout;//printf

    sei();
	UART_Init(UBRR_VALUE);
	ADC_Init();
	PWM_Init();
	
	//output pin setup
	DDRB |= (1<<PB1)|(1<<PB2);
	DDRD |= (1<<PD5)|(1<<PD6);
	DDRD |= (1<<PD3);

	//UART_SendJson(12, 15, voltage, 20, false, true, true, 120,123);
	
	_delay_ms(5200);

    while (1) 
    {	
		//reset index positions
		voltageLHSIndex = 0;
		voltageRHSIndex = 0;
		currentIndex = 0;
		uint64_t voltageSum = 0;
		uint32_t rmsVoltage = 0;
		uint64_t currentSum = 0;
		uint32_t rmsCurrent = 0;
		uint32_t powerArray[NUMBER_OF_SAMPLES];
		
		//if(changePumpingEffort){
			 ////UART_InterpretPumpingEffort();
		//}
		
		
		/****Voltage and Current*****/
		////get voltage values
		while(voltageLHSIndex < NUMBER_OF_SAMPLES){
			voltageLHS[voltageLHSIndex] = ADC_LHSVoltage();
			voltageRHS[voltageLHSIndex] = ADC_RHSVoltage();
			current[voltageLHSIndex] = ADC_Current();
			//printf("%d\t",voltageRHS[voltageLHSIndex]);
			//printf("%d\n",voltageLHS[voltageLHSIndex]);
			voltageLHSIndex++;
		}
		
		//get voltage across the coil
		for(int i = 0;i<NUMBER_OF_SAMPLES;i++){
			voltageAcrossTheCoil[i] = voltageRHS[i] - voltageLHS[i];
			if(voltageAcrossTheCoil[i]<0){
					voltageAcrossTheCoil[i] = voltageAcrossTheCoil[i]*-1;
			}
			voltageSum += (uint64_t)(voltageAcrossTheCoil[i]*voltageAcrossTheCoil[i]);
			currentSum += (uint64_t)(current[i]*current[i]);
			//printf("%d\n",voltageSum);
		}
		
		
		//calculate rms voltage and current
		rmsVoltage = (uint32_t) (voltageSum / NUMBER_OF_SAMPLES);
		rmsCurrent = (uint32_t) (currentSum / NUMBER_OF_SAMPLES);
		rmsVoltage = sqrt(rmsVoltage);
		rmsCurrent = sqrt(rmsCurrent);
		//printf("%d\n",rmsVoltage);
		//printf("%d\n",rmsCurrent
		
		//calculate average power	
		
		
		
			
    }
	return 0;
}

