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



#include "main.h"
#include "drivers/adc.h"
#include "drivers/pwm.h"
#include "drivers/uart.h"

#define F_CPU 8000000UL
#define BAUDRATE 9600
#define UBRR_VALUE F_CPU/16/BAUDRATE - 1

/**************************************************************Macros**************************************************************/
#define PWM_FREQUENCY 1000
#define NUMBER_OF_SAMPLES 120
#define BACKEMF

/**************************************************************Global Variables**************************************************************/
/*******************PWM Global Variables*******************/
volatile uint8_t count = 0;
volatile bool isDead = false;																//Gives indication of whether we are in the dead-zone
volatile bool isLHS = true;																	//Checks whether we are using the left-hand side MOSFET pair
volatile bool lowPowerMode = true;															//Indicates whether we are in the low-power mode
volatile uint8_t pumpingEffort = 0;															//Mass-Flow Control Number 
volatile uint16_t frequency = 65;															//Operating Frequency. Note: Must be halved when in the low power mode to adjust for dead-time 
volatile uint8_t noOfWaves = 32;															//Number of PWM waves being pulsed
volatile uint16_t dutyCycle = 0; 

/*******************UART Receive Global Variables*******************/
volatile bool messageReceived = false;														//Indicates whether the entire message has been characterReceived or not
volatile unsigned char characterReceived;													//
volatile int numberOfCharactersReceived = 0;												//
volatile unsigned char pumpingEffortArray[40] = {0};										//
	
/*******************Power Measurement Global Variables*******************/
volatile uint32_t voltageLHS[NUMBER_OF_SAMPLES];											//Holds left-hand side voltage values
volatile uint8_t measurementIndex = 0;														//Indexing value for power measurement arrays
volatile uint32_t voltageRHS[NUMBER_OF_SAMPLES];											//Holds right-hand side voltage values
volatile uint32_t voltageAverageArray[NUMBER_OF_SAMPLES];									//Holds average voltage values 
volatile uint32_t voltageAcrossTheCoil[NUMBER_OF_SAMPLES];								 

volatile uint8_t operatingFrequency = 0;
volatile uint8_t appliedVoltage = 0;
volatile uint8_t averagePower = 0;

/*******************Error Detection Global Variables*******************/
volatile bool cmprJammed = true;															//Compressor Jammed Flag, True = the compressor is jammed and False = compressor is not jammed
volatile bool clearErrorFlag = false;														//True = clear the errors within the system, False = errors persist
volatile unsigned char clearErrorArray[20] = {0};					
volatile bool cmprCollide = true;															//Compressor Collide Flag, True = the compressor is piston is colliding with the cylinder, False = compressor is not colliding





/**************************************************************Interrupt Service Routines (ISR)**************************************************************/

/*******************UART Receive Complete ISR*******************/
/*Purpose: Provides functionality for the controller to receive commands from the master*/
ISR(USART_RX_vect){ 
	characterReceived = UDR0;															//Collect the character received from the master
	numberOfCharactersReceived++;
	if(numberOfCharactersReceived > 20){
		pumpingEffortArray[numberOfCharactersReceived - 21] = characterReceived;		//
	}
	if(numberOfCharactersReceived > 33){
		clearErrorArray[numberOfCharactersReceived - 34] = characterReceived;
	}
	if(numberOfCharactersReceived>37){
		UCSR0B &= ~(1<<RXCIE0);															//Turn off the receiver
		UCSR0B &= ~(1<<RXEN0);
		messageReceived = true;
		numberOfCharactersReceived = 0;
	}
}

/*******************UART Transmit Complete ISR*******************/
/*Purpose: Receiving commands is disabled during transmission from the controller. After 
transmission is completed, re-enable the receive functionality*/
ISR(USART_TX_vect){ //wait till tx flag is set before ready to receive
	UCSR0B |= (1<<RXEN0);
	UCSR0B |= (1<<RXCIE0);
}

/*******************Timer 1 ISR COMPA*******************/
/*Purpose: Provides functionality to create the PWM wave to control the motor*/
ISR(TIMER1_COMPA_vect){
	if(isLHS || lowPowerMode){															//This conditional block ensures that if the controller is in low power mode or the isLHS flag is True, then use the left-hand side (LHS) motor driver pair to drive the motor
		if((count <= noOfWaves) && (!isDead)){											//Produce a certain number of PWM waves 
			PORTB |= (1<< PB2);															//Turn on LHS PMOS
			PORTD |= (1<<PD5);															//Turn on LHS NMOS
			count++;
		}
		else if(count > noOfWaves){														//Once the correct number of PWM waves have been generated, initiate a dead-zone
			PORTB &= ~(1<<PB2);															//Turn PMOS off
			PORTD &= ~(1<<PD5);															//Turn NMOS off
			PWM_Change(PWM_CalculateDeadTime(),65535);									//Set appropriate Dead-zone depending on frequency of operation 
			if(!lowPowerMode){															//If the controller is not in low power mode then set the isLHS flag to false so that next cycle the right-hand side driver wil be used 
				isLHS = false;
			}
			count = 0;
			isDead = true;																//Activate dead-zone flag
		}
		else{																			//End of Dead-zone, set the PWM operating frequency back to normal operation (1khz)
			isDead = false;
			PWM_Change(125,PWM_ConvertTimerValueToDutyCycle());
		}
	}
	else{																				//This conditional block ensures that if the isLHS is False, then use the right-hand side (RHS) driver to  drive the motor
		if((count <= noOfWaves) && (!isDead)){											//Produce a certain number of PWM waves 
			PORTD |= (1<< PD6);															//Turn RHS NMOS on
			PORTB |= (1<<PB1);															//Turn RHS PMOS on
			count++;
		}
		else if(count > noOfWaves){														//Once the correct number of PWM waves have been generated, initiate a dead-zone
			PORTB &= ~(1<<PB1);															//Turn RHS PMOS off
			PORTD &= ~(1<< PD6);														//Turn RHS NMOS off
			PWM_Change(PWM_CalculateDeadTime(),65535);									//Set appropriate Dead-zone depending on frequency of operation 
			isLHS = true;																//Set the isLHS flag to true, to ensure that next round the LHS drivers are used
			count = 0;
			isDead = true;																//Activate dead-zone flag
		}
		else{																			//End of Dead-zone, set the PWM operating frequency back to normal operation (1khz)
			isDead = false;
			PWM_Change(125,PWM_ConvertTimerValueToDutyCycle());
		}
		
	}
}

/*******************Timer 1 ISR COMPB*******************/
/*Purpose: Provides functionality to create the PWM wave for the PMOS to control the motor movement*/
ISR(TIMER1_COMPB_vect){																	
	if(isLHS || lowPowerMode){																									
		if((!isDead) && (count <=noOfWaves)){
			PORTB &= ~(1 << PB2);
		}
	}
	else{
		if((!isDead) && (count <=noOfWaves)){
			PORTB &= ~(1 << PB1);
		}
	}
}

/**************************************************************Error Detection Helper Functions**************************************************************/

/*******************Jam Detection*******************/

//void jamDetection(int i){
	//if(i>1){
		//if(BACKEMF == 0){
			//cmprJammed = true;
			//safetyShutdown();
		//}else{
			//cmprJammed = false;
		//}
	//}
//}

/*******************Safety Shutdown*******************/
/*Purpose: Turn the coil off if any errors have occurred*/
void safetyShutdown(){
	if(cmprJammed){
		pumpingEffort = 0; 
	}
}

/**************************************************************UART Receive Helper Functions**************************************************************/
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
/**************************************************************Main Function**************************************************************/
int main(void)
{	

	/*******************Intialisations*******************/
    sei();
	UART_Init(UBRR_VALUE);
	ADC_Init();
	PWM_Init();
	
	/*******************Local Variable Declaration*******************/
	uint8_t voltageAverageIndex = 0;
	uint16_t voltageAverageFinal = 0;
	uint16_t currentAverageFinal = 0;
	uint32_t averagePower = 0;
	
	/*******************Output Pin Setup*******************/
	DDRB |= (1<<PB1)|(1<<PB2);
	DDRD |= (1<<PD5)|(1<<PD6);
	
	/*******************Coil Impedence*******************/
	uint32_t coilImpedence;

    while (1) 
    {	
		
		/*******************Local Variable Declaration*******************/
		measurementIndex = 0;
		uint32_t averageVoltage = 0;
		uint32_t voltageSum = 0;
		uint32_t powerArray[NUMBER_OF_SAMPLES];

		/*******************Voltage Measurement*******************/
			while(measurementIndex < NUMBER_OF_SAMPLES){									//Keep on sampling the LHS and RHS of the coil until you reach the total number of samples
				voltageLHS[measurementIndex] = ADC_LHSVoltage();							//Sample LHS voltage
				voltageRHS[measurementIndex] = ADC_RHSVoltage();							//Sample RHS voltage
				measurementIndex++;											
			}
		
		for(int i = 0;i<NUMBER_OF_SAMPLES;i++){												//This for-loop calculates the voltage across the coil
			if(voltageLHS[i]>voltageRHS[i]){												
				voltageAcrossTheCoil[i] = voltageLHS[i]-voltageRHS[i];
				}else{
				voltageAcrossTheCoil[i] = voltageRHS[i]-voltageLHS[i];
			}		
			voltageSum += voltageAcrossTheCoil[i];											
			
		}
		
		averageVoltage = voltageSum / NUMBER_OF_SAMPLES;									//This is the average voltage per sample					

		voltageAverageArray[voltageAverageIndex] = averageVoltage;							
		voltageAverageIndex++;
				
		/*******************Calculate Average Voltage and Current*******************/		
		if(voltageAverageIndex==NUMBER_OF_SAMPLES){											//The averageVoltage per sample is then averaged across multiple samples to get a a more accurate result	
			for(int k = 0;k < NUMBER_OF_SAMPLES;k++){
				voltageAverageFinal += voltageAverageArray[k];
			}
			voltageAverageFinal /= NUMBER_OF_SAMPLES;										//This the average voltage across multiple samples taken 
			currentAverageFinal = (voltageAverageFinal*100)/415;							//From average voltage, average current through the coil is calculated using Ohms law			
			voltageAverageIndex = 0;
		}
				
		/*******************Power Calculation*******************/	
		uint32_t powerTotal = 0;												
		for(int j = 0; j < NUMBER_OF_SAMPLES; j++){											//This for-loop calculates the average power dissipated over the coil
			powerArray[j] = (voltageAcrossTheCoil[j] *voltageAcrossTheCoil[j])/415;
			powerTotal += powerArray[j];
		}
		
		averagePower = powerTotal / NUMBER_OF_SAMPLES;										
					
		/*******************Receive Message Protocol*******************/
		if(messageReceived){
			pumpingEffort = concatenate(pumpingEffortArray[0],pumpingEffortArray[1],pumpingEffortArray[2]);
				
			for(int i = 0; i < 38; i++){
				pumpingEffortArray[i] = 0;
			}
			clearErrorFlag = checkForError(clearErrorArray[0],clearErrorArray[1]);
			if(clearErrorFlag){
				cmprCollide = false;
				cmprJammed = false;;
			}
			if(lowPowerMode){
				frequency = frequency*2/10;
			}else{
				frequency = frequency/10;
			}
			UART_SendJson(averagePower,frequency,voltageAverageFinal,currentAverageFinal,cmprJammed,cmprCollide, pumpingEffort,pumpingEffort);
			messageReceived = false;
			numberOfCharactersReceived = 0;
		}
		UART_InterpretPumpingEffort();														
    }
	
	
	return 0;
}


