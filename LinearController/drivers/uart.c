/*
 * uart.c
 *
 * Created: 17/09/2018 11:29:15 AM
 *  Author: Hasnain
 */ 


#include "uart.h"
#include "../main.h"
#include <avr/power.h>
#include <string.h>


/**************************************************************Macros**************************************************************/
#define LOW_POWER_PROPORTIONALITY_CONSTANT 30
#define LOW_POWER_INTERCEPT 1300
#define HIGH_POWER_PROPORTIONALITY_CONSTANT 37

/**************************************************************UART Intialisation**************************************************************/
void UART_Init(unsigned int BAUD_RATE){
	
	UBRR0H = BAUD_RATE >>8;
	UBRR0L = BAUD_RATE;
	UCSR0B = (1<<RXEN0)|(1<<RXCIE0);
	UCSR0B |= (1<<TXEN0)|(1<<TXCIE0);
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
}

/**************************************************************UART Transmit**************************************************************/

void UART_Transmit(uint8_t myValue){
	//disable receive
	UCSR0B &= ~(1 << RXEN0);
	UCSR0B &= ~(1 << RXCIE0);
	while (!((1<<UDRE0)&&(UCSR0A))); //wait until the transmit register is ready
	UDR0 = myValue;//once ready, store next value for transmission
}

/**************************************************************Decimal to ASCII Conversion**************************************************************/
uint8_t UART_ASCIIConversion(uint8_t value){
	uint8_t asciiValue = value + 48;
	return asciiValue;
}


/**************************************************************Mass-Flow Control Value Interpretation**************************************************************/
/*Puropose: This function allows the controller to interpret the mass-flow control (MFC) value sent by the master. This function changes duty cyle in an effort 
to adjust stroke length which is proportionally related to exerted pumping effort*/
void UART_InterpretPumpingEffort(){
	if(pumpingEffort==255){																					//MFC at maximum
		if(!isOn){
			power_all_enable();
			isOn = true;
		}
		dutyCycle = 99;
		lowPowerMode = false;
	}else if((pumpingEffort>=1)&&(pumpingEffort<=178)){														
		if(!isOn){
			power_all_enable();
			isOn = true;
		}
		if(!lowPowerMode){
			frequency /= 2;																					//Frequency is halved to maintain the current operating frequency
		}
		lowPowerMode = true; 																				//Turns on low power mode meaning we are only using one pair of drivers
		dutyCycle = (LOW_POWER_PROPORTIONALITY_CONSTANT*pumpingEffort + LOW_POWER_INTERCEPT)/100;			//Low power relationship between duty cycle and pumping effort - found through analysis
	}else if((pumpingEffort>178)&&(pumpingEffort<=254)){													//Higher MFC range, 
		if(!isOn){
			power_all_enable();
			isOn = true;
		}
		if(lowPowerMode){
			frequency *= 2;																					//Frequency is doubled to maintain the current operating frequency
		}
		lowPowerMode = false;
		dutyCycle = HIGH_POWER_PROPORTIONALITY_CONSTANT*pumpingEffort/100;									//High power relationship between duty cycle and pumping effort - again found through analysis
	}else{ 																									//When a zero or other undefined character is received through the master turn off the coil
		power_all_disable(); 																				//Disable all units																			
		power_usart0_enable();
		isOn = false;																				//Enable UART so that communication can still occur
		dutyCycle = 0;																						
	}
}

/**************************************************************JSON Structure**************************************************************/

/**************************************************************Send JSON Structure**************************************************************/
/*The functions encapsulated within UART_SendJSON create the JSON Structure that is send to the master from the controller*/
void UART_SendJson(uint8_t averagePower, uint8_t operatingFrequency, uint32_t appliedVoltage, uint8_t current,bool jamErrorFlag, bool collisionErrorFlag, uint8_t requiredValue, uint8_t currentValue){
	MFCmodulator(requiredValue,currentValue);
	VERmodulator();
	PARAMmodulator(averagePower,operatingFrequency,appliedVoltage,current);
	ERRORmodulator(jamErrorFlag, collisionErrorFlag);

}

/**************************************************************Mass Flow Control**************************************************************/

void MFCmodulator(uint8_t requiredValue, uint8_t currentValue){
	uint8_t firstDigit = 0;
	uint8_t secondDigit = 0;
	uint8_t thirdDigit = 0;
																											//Shown below are the ASCII conversions
	UART_Transmit(13); 																						//carriage return
	UART_Transmit(123); 																					//{
	UART_Transmit(10); 																						//line feed 
	UART_Transmit(13); 																						//carriage return
	
																											//LCC Identifier 
	UART_Transmit(34); 																						//"
	UART_Transmit(51);																						//3
	UART_Transmit(34);																						//"
	UART_Transmit(58);																						//:

																											//MFC 
	UART_Transmit(10); 																						//Line feed
	UART_Transmit(13); 																						//carriage return
	UART_Transmit(123);																						//{
	UART_Transmit(10);																						//Line feed
	UART_Transmit(13); 																						//carriage return
	
																											//MFC letter setup
	UART_Transmit(34); 																						//'
	UART_Transmit(109);																						//m
	UART_Transmit(102);																						//f
	UART_Transmit(99);																						//c
	UART_Transmit(34);																						//"
	UART_Transmit(58);																						//:
	 
																											//MFC Values 
	UART_Transmit(123); 																					//{
	UART_Transmit(34); 																						//"
	UART_Transmit(114);																						//r
	UART_Transmit(101);																						//e
	UART_Transmit(113);																						//q
	UART_Transmit(34);																						//"
	UART_Transmit(58);																						//:
	UART_Transmit(34);																						//"
	
																											//Transmit the required MFC value 
	firstDigit = requiredValue/100;																			//The dividing by powers of 10 and multiplying by powers of 10 is necessary to extract a digit from a number 
	secondDigit = (requiredValue-(firstDigit*100))/10;
	thirdDigit = requiredValue - (firstDigit*100) - (secondDigit*10);
	UART_Transmit(UART_ASCIIConversion(firstDigit));
	UART_Transmit(UART_ASCIIConversion(secondDigit)); 
	UART_Transmit(UART_ASCIIConversion(thirdDigit));
	UART_Transmit(34);																						//"
	UART_Transmit(44);																						//,
	

	UART_Transmit(34);																						//"
	UART_Transmit(99);																						//c
	UART_Transmit(117);																						//u
	UART_Transmit(114);																						//r
	UART_Transmit(34);																						//"
	UART_Transmit(58);																						//:
	UART_Transmit(34);																						//"
	
																											//Transmit current MFC value
	firstDigit = currentValue/100;																			//The dividing by powers of 10 and multiplying by powers of 10 is necessary to extract a digit from a number 
	secondDigit = (currentValue-(firstDigit*100))/10;
	thirdDigit = currentValue - (firstDigit*100) - (secondDigit*10);
	UART_Transmit(UART_ASCIIConversion(firstDigit));
	UART_Transmit(UART_ASCIIConversion(secondDigit));
	UART_Transmit(UART_ASCIIConversion(thirdDigit));
	
	UART_Transmit(34);																						//"
	UART_Transmit(125);																						//}
	UART_Transmit(44);																						//,
	UART_Transmit(10); 																						//Line feed
	UART_Transmit(13); 																						//carriage return
	 
	
}

/**************************************************************Version Number**************************************************************/
void VERmodulator(){
	UART_Transmit(34); 																						//"
	UART_Transmit(118);																						//v
	UART_Transmit(101);																						//e
	UART_Transmit(114);																						//r
	UART_Transmit(34);																						//"
	UART_Transmit(58);																						//:
	UART_Transmit(34);																						//"
	UART_Transmit(49);																						//1
	UART_Transmit(46);																						//.
	UART_Transmit(50);																						//2
	UART_Transmit(46);																						//.
	UART_Transmit(51);																						//3
	UART_Transmit(34);																						//"
	UART_Transmit(44);																						//,
	UART_Transmit(10); 																						//Line feed 
	UART_Transmit(13); 																						//carriage return
	 
}

/**************************************************************Parameter Modulator**************************************************************/
void PARAMmodulator(uint8_t averagePower, uint8_t operatingFrequency, uint32_t appliedVoltage, uint8_t current){
	uint8_t firstDigit; 
	uint8_t secondDigit;
	uint8_t thirdDigit;
	uint8_t fourthDigit;

	UART_Transmit(34);																						//"
	UART_Transmit(112);																						//p
	UART_Transmit(97);																						//a
	UART_Transmit(114);																						//r
	UART_Transmit(97);																						//a
	UART_Transmit(109);																						//m
	UART_Transmit(34);																						//"
	UART_Transmit(58);																						//:
	UART_Transmit(123);																						//{

																											//Transmit Parameters
	UART_Transmit(34); 																						//"
	UART_Transmit(112); 																					//p
	UART_Transmit(119); 																					//w
	UART_Transmit(114);																						//r 
	UART_Transmit(34); 																						//"
	UART_Transmit(58); 																						//:
	UART_Transmit(34);																						//"
	
																											//Transmit Power 
	firstDigit = averagePower/100;																			//The dividing by powers of 10 and multiplying by powers of 10 is necessary to extract a digit from a number 
	secondDigit = (averagePower-(firstDigit*100))/10;
	thirdDigit = averagePower - (firstDigit*100) - (secondDigit*10);
	UART_Transmit(UART_ASCIIConversion(firstDigit));
	UART_Transmit(UART_ASCIIConversion(secondDigit));
	UART_Transmit(46); 																						//Decimal point
	UART_Transmit(UART_ASCIIConversion(thirdDigit));
	UART_Transmit(87); 																						//W
	UART_Transmit(34); 																						//"
	UART_Transmit(44); 																						//,
	 
	 
	
																											//Transmit Frequency
	UART_Transmit(34); 																						//"
	UART_Transmit(102);																						//f
	UART_Transmit(114);																						//r
	UART_Transmit(101);																						//e
	UART_Transmit(113);																						//q
	UART_Transmit(34);																						//"
	UART_Transmit(58);																						//:
	UART_Transmit(34);																						//"
	firstDigit = operatingFrequency/10;																		//The dividing by powers of 10 and multiplying by powers of 10 is necessary to extract a digit from a number 
	secondDigit = operatingFrequency-(firstDigit*10);
	UART_Transmit(UART_ASCIIConversion(firstDigit));
	UART_Transmit(UART_ASCIIConversion(secondDigit));
	UART_Transmit(72);																						//H
	UART_Transmit(122);																						//z
	UART_Transmit(34);																						//"
	UART_Transmit(44);																						//,
	
																											//Transmit Current
	UART_Transmit(34); 																						//"
	UART_Transmit(99); 																						//c
	UART_Transmit(117); 																					//u
	UART_Transmit(114);																						//r
	UART_Transmit(114);																						//r
	UART_Transmit(34); 																						//"
	UART_Transmit(58); 																						//:
	UART_Transmit(34);																						//"
	firstDigit = current/100;																				//The dividing and multiplying by powers of 10 below is necessary to extract a digit from a number 
	thirdDigit = current - (firstDigit*100) - (secondDigit*10);
	UART_Transmit(UART_ASCIIConversion(firstDigit));
	UART_Transmit(46); 																						//decimal point												
	UART_Transmit(UART_ASCIIConversion(secondDigit));
	UART_Transmit(UART_ASCIIConversion(thirdDigit));
	UART_Transmit(65); 																						//A
	UART_Transmit(34); 																						//"
	UART_Transmit(44); 																						//,
	
																											//Transmit average voltage across the coil
	 
	UART_Transmit(34); 																						//"
	UART_Transmit(118); 																					//v
	UART_Transmit(111); 																					//o
	UART_Transmit(108); 																					//l
	UART_Transmit(116); 																					//t
	UART_Transmit(34); 																						//"
	UART_Transmit(58); 																						//:
	UART_Transmit(34); 																						//"
	firstDigit = appliedVoltage/1000;																		//The dividing and multiplying by powers of 10 below is necessary to extract a digit from a number 
	secondDigit = (appliedVoltage-(firstDigit*1000))/100;												
	thirdDigit = (appliedVoltage - (firstDigit*1000) - (secondDigit*100))/10;
	fourthDigit = appliedVoltage -(firstDigit*1000) - (secondDigit*100) - (thirdDigit*10);
	UART_Transmit(UART_ASCIIConversion(firstDigit));
	UART_Transmit(UART_ASCIIConversion(secondDigit));
	UART_Transmit(46); 																						//decimal point
	UART_Transmit(UART_ASCIIConversion(thirdDigit));
	UART_Transmit(UART_ASCIIConversion(fourthDigit));
	UART_Transmit(86); 																						//V
	UART_Transmit(34); 																						//"
	
	 
	UART_Transmit(125); 																					//}
	UART_Transmit(44);																						//,
	UART_Transmit(10); 																						//line feed
	UART_Transmit(13); 																						//carriage return
	 
}

/**************************************************************Error Modulator**************************************************************/
void ERRORmodulator(bool jamErrorFlag, bool collisionErrorFlag){  
	if(clearErrorFlag){																						//acknowledge that the 'clear error' command was received from the user and obeyed by the controller
		UART_Transmit(34); 																					//"
		UART_Transmit(99); 																					//c
		UART_Transmit(108); 																				//l
		UART_Transmit(114); 																				//r
		UART_Transmit(34); 																					//"
		UART_Transmit(58); 																					//:
		UART_Transmit(34); 																					//"
		UART_Transmit(101); 																				//e
		UART_Transmit(119); 																				//w
		UART_Transmit(34); 																					//"
		UART_Transmit(44); 																					//,
		UART_Transmit(10); 																					//line feed
		UART_Transmit(13); 																					//carriage return
		clearErrorFlag = false;
	}else{
		UART_Transmit(10); 																					//line feed
		UART_Transmit(13); 																					//carriage return
		
		if(collisionErrorFlag || jamErrorFlag) {															//if either error occurs then send 
			UART_Transmit(34); 																				//'
			UART_Transmit(101); 																			//e
			UART_Transmit(119); 																			//w
			UART_Transmit(34); 																				// "
			UART_Transmit(58); 																				//:
			UART_Transmit(91); 																				//[
																											
																											//Print the various errors
			if(jamErrorFlag){ 																				//Compressor Jammed 
				UART_Transmit(34); 																			//"
				UART_Transmit(106); 																		//j
				UART_Transmit(97); 																			//a
				UART_Transmit(109); 																		//m
				UART_Transmit(34); 																			//"
				if(jamErrorFlag && collisionErrorFlag){
					UART_Transmit(44); 																		//,
				}
			}

			if(collisionErrorFlag){ 																		//Compressor Colliding
				UART_Transmit(34); 																			//"
				UART_Transmit(99); 																			//c
				UART_Transmit(111); 																		//o
				UART_Transmit(108); 																		//l
				UART_Transmit(108); 																		//l
				UART_Transmit(105); 																		//i
				UART_Transmit(115); 																		//s
				UART_Transmit(105); 																		//i
				UART_Transmit(111); 																		//o
				UART_Transmit(110); 																		//n
				UART_Transmit(34); 																			//"
			}
			
			
			UART_Transmit(93); //[
			
			}else{ //no errors present
			UART_Transmit(10); //line feed
			UART_Transmit(13); //carriage return
		}
	}
	
	
	//print final curly brackets
	
	UART_Transmit(10); //line feed
	UART_Transmit(13); //carriage return
	UART_Transmit(125); // }
	UART_Transmit(10); //line feed
	UART_Transmit(13); //carriage return
	UART_Transmit(125); // }
	UART_Transmit(10); //line feed
	UART_Transmit(13); //carriage return


}