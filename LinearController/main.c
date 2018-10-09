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
#include "delay.h"


#include "main.h"
#include "drivers/adc.h"
#include "drivers/pwm.h"
#include "drivers/uart.h"

#define F_CPU 8000000UL
#define BAUDRATE 9600
#define UBRR_VALUE F_CPU/16/BAUDRATE - 1

#define PWM_FREQUENCY 1000
#define NUMBER_OF_POSSIBLE_ERRORS 2 //jam, collision
#define NUMBER_OF_SAMPLES 20

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
volatile uint16_t dutyCycle = 0;
volatile bool transmitParameters = true;
volatile unsigned char value = 0;


//receive global variable
volatile bool finished = false;
volatile unsigned char received;
volatile int rx_count = 0;
volatile unsigned char pumpingEffortArray[40] = {0};
volatile int pumpParam = 0;


volatile uint32_t current[NUMBER_OF_SAMPLES];
volatile uint8_t currentIndex = 0;

volatile uint32_t voltageLHS[NUMBER_OF_SAMPLES];
volatile uint8_t voltageLHSIndex = 0;

volatile uint32_t voltageRHS[NUMBER_OF_SAMPLES];
volatile uint8_t voltageRHSIndex = 0;

volatile uint32_t voltageAcrossTheCoil[NUMBER_OF_SAMPLES];

volatile uint8_t operatingFrequency = 0;
volatile uint8_t appliedVoltage = 0;
volatile uint8_t averagePower = 0;

volatile uint32_t tempParam = 0;

//error detection
volatile bool cmprJammed = true;
volatile bool clearErrorFlag = false;
volatile unsigned char clearErrorArray[20] = {0};
volatile bool cmprCollide = true;


//adc arrays
int usart_putchar_printf(char var, FILE *stream){
	if(var== '\n') UART_Transmit('\r');
	UART_Transmit(var);
	return 0;
}

static FILE mystdout = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);

int concatenate(int a, int b, int c){
	return ((a-48)*100 + (b-48)*10 + (c-48));
}

bool checkForError(unsigned char a, unsigned char b){
	if((a == 101) && (b == 119)){ //101 = e, 119 = w
		return true;
	}else{
		return false;
	}
}

ISR(USART_RX_vect){ //USART receiver ISR
	received = UDR0;
	rx_count++;
	if(rx_count > 20){
		pumpingEffortArray[rx_count - 21] = received;
	}
	if(rx_count > 33){
		clearErrorArray[rx_count - 34] = received;
	}
	if(rx_count>37){
		UCSR0B &= ~(1<<RXCIE0); //turn of receiver after having received
		UCSR0B &= ~(1<<RXEN0);
		finished = true;
		rx_count = 0;
	}
}

//disable recieve during transmission
ISR(USART_TX_vect){ //wait till tx flag is set before ready to receive
	UCSR0B |= (1<<RXEN0);
	UCSR0B |= (1<<RXCIE0);
}


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

void jamDetection(int i){
	if(i>1){
		if(voltageAcrossTheCoil[i]==voltageAcrossTheCoil[i-1] && voltageAcrossTheCoil[i-1]==voltageAcrossTheCoil[i-2] && voltageAcrossTheCoil[i-2]==voltageAcrossTheCoil[i-3] && voltageAcrossTheCoil[i-4]==voltageAcrossTheCoil[i-5]){
			cmprJammed = true;
		}else{
			cmprJammed = false;
		}
	}
}

void safetyShutdown(){
	if(cmprJammed){
		pumpingEffort = 0; //stop the coil if an error has occurred
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



    while (1) 
    {	
		//reset index positions
		voltageLHSIndex = 0;
		voltageRHSIndex = 0;
		currentIndex = 0;
		uint32_t rmsVoltage = 0;
		uint32_t currentSum = 0;
		uint32_t rmsCurrent = 0;
		uint32_t voltageSum = 0;
		uint32_t powerArray[NUMBER_OF_SAMPLES] = {0};
		
		
		//change the pumping effort
		UART_InterpretPumpingEffort();
		
		
		
		/****Voltage and Current*****/
		//get voltage and current values
		//while(voltageLHSIndex < NUMBER_OF_SAMPLES){
			//voltageLHS[voltageLHSIndex] = ADC_LHSVoltage();
			//voltageRHS[voltageLHSIndex] = ADC_RHSVoltage();
			////current[voltageLHSIndex] = ADC_Current();
			//printf("%d\t",voltageRHS[voltageLHSIndex]);
			//printf("%d\n",voltageLHS[voltageLHSIndex]);
			//voltageLHSIndex++;
		//}
		
		//mock arrays
		for(int i = 0; i < NUMBER_OF_SAMPLES;i++){
			voltageRHS[i] = 1132;
			voltageLHS[i] = 40;
			current[i] = 930;
			
		}
		
		
		//get voltage across the coil
		for(int i = 0;i<NUMBER_OF_SAMPLES;i++){
			if(voltageLHS[i]>voltageRHS[i]){
				voltageAcrossTheCoil[i] = voltageLHS[i]-voltageRHS[i];
			}else{
				voltageAcrossTheCoil[i] = voltageRHS[i]-voltageLHS[i];
			}
			voltageSum += voltageAcrossTheCoil[i]*voltageAcrossTheCoil[i];
			currentSum += current[i]*current[i];
			//printf("%d\n", current[i]/10);
			//printf("%d\n",i);
		}
	
		
		

		//printf limitations example
		//uint32_t voltageSum = 23849280;
		//printf("%d\n",voltageSum);
		//UART_Transmit((voltageSum/10000000)+48);
	
		
		
		//calculate rms voltage and current
		rmsVoltage = voltageSum / NUMBER_OF_SAMPLES;
		rmsCurrent = currentSum/NUMBER_OF_SAMPLES;
		
	
		rmsVoltage = sqrt(rmsVoltage);
		rmsCurrent = sqrt(rmsCurrent);
		//printf("%d\n",rmsVoltage);
		//printf("%d\n",rmsCurrent);
		
		//calculate average power	
		for(int j = 0; j < NUMBER_OF_SAMPLES; j++){
			powerArray[j] = (voltageAcrossTheCoil[j] * current[j]);
		}
		uint32_t powerTotal = 0;
		uint32_t rmsPower = 0;
		for (int i = 0; i < NUMBER_OF_SAMPLES-1; i++) {
			powerTotal += (powerArray[i] + powerArray[i+1])/2;	//trapezoidal approx
		}
		
		rmsPower = powerTotal / (NUMBER_OF_SAMPLES-1);
		//printf("%d\t",rmsPower/1000);
		//printf("%d\n",rmsPower%1000);
		
		
		//receive message code
		if(finished){
			pumpingEffort = concatenate(pumpingEffortArray[0],pumpingEffortArray[1],pumpingEffortArray[2]);
				
			for(int i = 0; i < 38; i++){
				pumpingEffortArray[i] = 0;
			}
			clearErrorFlag = checkForError(clearErrorArray[0],clearErrorArray[1]);
			if(clearErrorFlag){
				cmprCollide = false;
				cmprJammed = false;;
			}
			UART_SendJson(rmsPower,frequency,rmsVoltage,rmsCurrent,cmprJammed,cmprCollide, pumpingEffort,pumpingEffort);
			finished = false;
			rx_count = 0;
		}
			
    }
	
	
	return 0;
}

