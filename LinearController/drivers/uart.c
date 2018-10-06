/*
 * uart.c
 *
 * Created: 17/09/2018 11:29:15 AM
 *  Author: hasna
 */ 


#include "uart.h"
#include "../main.h"

#define MAX_LOW_POWER 37885
#define PROPORTIONALITY_CONSTANT 21983

//uart intializer
//initializes UART transmitter
void UART_Init(unsigned int BAUD_RATE){
	
	UBRR0H |= (BAUD_RATE>>8); //sets the baud rate to 9600bps
	UBRR0L |= (BAUD_RATE);
	UCSR0B |= (1<<TXEN0)|(1<RXEN0); //enables UART transmitter
	UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00); //sets character size to 8 data bits

}


////uart transmit
////polling uart
void UART_Transmit(uint8_t myValue){
	while (!((1<<UDRE0)&&(UCSR0A))); //wait until the transmit register is ready
	UDR0 = myValue;//once ready, store next value for transmission
}

//uart recieve 
//interrupt config
uint8_t UART_Receive(){
	uint8_t pumpingEffort = UDR0;
	UART_Transmit(pumpingEffort + 48);
	return pumpingEffort;
}

void UART_InterpretPumpingEffort(){
	pumpingEffort = 1; //mock pumping effort
	if(pumpingEffort==0){ //turn off mode 
		power_all_disable(); //disables all modules on the microcontroller 
		//power_usart_enable(); //enable UART for communication to see when to turn back on
	}else if((pumpingEffort>=1)&&(pumpingEffort<=178)){
		//70% of values - care about efficiency and meeting pumpingEffort
		//efficiency actions turn two switches off
		//disable all unused modules
		//dutyCycle = (PROPORTIONALITY_CONSTANT* MAX_LOW_POWER * (pumpingEffort/178))/(10000*1000);	//10000 and 1000 are because we didnt use floats [integer overflow error here]
		lowPowerMode = true;
						 
	}else if((pumpingEffort>178)&&(pumpingEffort<=254)){
		//30% of values - go ham fam
		lowPowerMode = false;
	}else{ //255 lose your mind
		//change duty cycle and pwm to max out the motor
		lowPowerMode = false;
	}
	changePumpingEffort = false;
}

void UART_SendJson(uint8_t averagePower, uint8_t operatingFrequency, uint32_t appliedVoltage, uint8_t current, bool errorClear,bool jamErrorFlag, bool collisionErrorFlag, uint8_t requiredValue, uint8_t currentValue){
	MFCmodulator(requiredValue,currentValue);
	VERmodulator();
	PARAMmodulator(averagePower,operatingFrequency,appliedVoltage,current);
	ERRORmodulator(errorClear, jamErrorFlag, collisionErrorFlag);

}


void MFCmodulator(uint8_t requiredValue, uint8_t currentValue){
	//temp conversion variables. Used to split 3 digit numbers as only one digit can be sent at a time
	uint8_t firstDigit = 0;
	uint8_t secondDigit = 0;
	uint8_t thirdDigit = 0;
	
	UART_Transmit(13); //carriage return
	UART_Transmit(123); //{
	UART_Transmit(10); //line feed 
	UART_Transmit(13); //carriage return
	
	//LCC Identifier 
	UART_Transmit(34); //"
	UART_Transmit(51);//3
	UART_Transmit(34);//"
	UART_Transmit(58);//:

	//MFC 
	UART_Transmit(10); //Line feed
	UART_Transmit(13); //carriage return
	UART_Transmit(123); //{
	UART_Transmit(10); //Line feed
	UART_Transmit(13); //carriage return
	
	
	//MFC letter setup
	UART_Transmit(34); //'
	UART_Transmit(109);//m
	UART_Transmit(102);//f
	UART_Transmit(99);//c
	UART_Transmit(34);//"
	UART_Transmit(58);//:
	 

	//MFC Values 
	UART_Transmit(123); //{
	UART_Transmit(34); //"
	UART_Transmit(114);//r
	UART_Transmit(101);//e
	UART_Transmit(113);//q
	UART_Transmit(34);//"
	UART_Transmit(58);//:
	UART_Transmit(34);//"
	
	//required value transmission
	firstDigit = requiredValue/100;
	secondDigit = (requiredValue-(firstDigit*100))/10;
	thirdDigit = requiredValue - (firstDigit*100) - (secondDigit*10);
	UART_Transmit(ASCIIConversion(firstDigit));
	UART_Transmit(ASCIIConversion(secondDigit)); 
	UART_Transmit(ASCIIConversion(thirdDigit));
	UART_Transmit(34);//"
	UART_Transmit(44);//,
	

	UART_Transmit(34);//"
	UART_Transmit(99);//c
	UART_Transmit(117);//u
	UART_Transmit(114);//r
	UART_Transmit(34);//"
	UART_Transmit(58);//:
	UART_Transmit(34);//"
	//current value transmission
	firstDigit = currentValue/100;
	secondDigit = (currentValue-(firstDigit*100))/10;
	thirdDigit = currentValue - (firstDigit*100) - (secondDigit*10);
	UART_Transmit(ASCIIConversion(firstDigit));
	UART_Transmit(ASCIIConversion(secondDigit));
	UART_Transmit(ASCIIConversion(thirdDigit));
	
	UART_Transmit(34);//"
	UART_Transmit(125);//}
	UART_Transmit(44);//,
	UART_Transmit(10); //Line feed
	UART_Transmit(13); //carriage return
	 
	
}

void VERmodulator(){
	UART_Transmit(34); //"
	UART_Transmit(118);//v
	UART_Transmit(101);//e
	UART_Transmit(114);//r
	UART_Transmit(34);//"
	UART_Transmit(58);//:
	UART_Transmit(34);//"
	UART_Transmit(49); //1
	UART_Transmit(46);//.
	UART_Transmit(50);//2
	UART_Transmit(46);//.
	UART_Transmit(51);//3
	UART_Transmit(34);//"
	UART_Transmit(44); //,
	UART_Transmit(10); //Line feed 
	UART_Transmit(13); //carriage return
	 
}

void PARAMmodulator(uint8_t averagePower, uint8_t operatingFrequency, uint32_t appliedVoltage, uint8_t current){
	uint8_t firstDigit; 
	uint8_t secondDigit;
	uint8_t thirdDigit;
	uint8_t fourthDigit;
	UART_Transmit(34);//"
	UART_Transmit(112);//p
	UART_Transmit(97);//a
	UART_Transmit(114);//r
	UART_Transmit(97);//a
	UART_Transmit(109);//m
	UART_Transmit(34);//"
	UART_Transmit(58);//:
	UART_Transmit(123);//{
	
	//parameters
	
	UART_Transmit(34); //"
	UART_Transmit(112); //p
	UART_Transmit(119); //w
	UART_Transmit(114);//r 
	UART_Transmit(34); //"
	UART_Transmit(58); //:
	UART_Transmit(34);//"
	
	//power parameters 
	firstDigit = averagePower/100;
	secondDigit = (averagePower-(firstDigit*100))/10;
	thirdDigit = averagePower - (firstDigit*100) - (secondDigit*10);
	UART_Transmit(ASCIIConversion(firstDigit));
	UART_Transmit(ASCIIConversion(secondDigit));
	UART_Transmit(46); //decimal point
	UART_Transmit(ASCIIConversion(thirdDigit));
	UART_Transmit(87); //W
	UART_Transmit(34); //"
	UART_Transmit(44); //,
	 
	 
	
	//frequency
	UART_Transmit(34); //"
	UART_Transmit(102);//f
	UART_Transmit(114);//r
	UART_Transmit(101);//e
	UART_Transmit(113);//q
	UART_Transmit(34);//"
	UART_Transmit(58);//:
	UART_Transmit(34);//"
	firstDigit = operatingFrequency/10;
	secondDigit = operatingFrequency-(firstDigit*10);
	UART_Transmit(ASCIIConversion(firstDigit));
	UART_Transmit(ASCIIConversion(secondDigit));
	UART_Transmit(72); //H
	UART_Transmit(122);//z
	UART_Transmit(34);//"
	UART_Transmit(44);//,
	
	//current 

	UART_Transmit(34); //"
	UART_Transmit(99); //c
	UART_Transmit(117); //u
	UART_Transmit(114);//r
	UART_Transmit(114);//r
	UART_Transmit(34); //"
	UART_Transmit(58); //:
	UART_Transmit(34);//"
	firstDigit = current/100;
	secondDigit = (current-(firstDigit*100))/10;
	thirdDigit = current - (firstDigit*100) - (secondDigit*10);
	UART_Transmit(ASCIIConversion(firstDigit));
	UART_Transmit(ASCIIConversion(secondDigit));
	UART_Transmit(ASCIIConversion(thirdDigit));
	UART_Transmit(109); //m
	UART_Transmit(65); //A
	UART_Transmit(34); //"
	UART_Transmit(44); //,
	
	//applied voltage
	 
	UART_Transmit(34); //"
	UART_Transmit(118); //v
	UART_Transmit(111); //o
	UART_Transmit(108); //l
	UART_Transmit(116); //t
	UART_Transmit(34); //"
	UART_Transmit(58); //:
	UART_Transmit(34); //"
	firstDigit = appliedVoltage/1000;
	secondDigit = (appliedVoltage-(firstDigit*1000))/100;
	thirdDigit = (appliedVoltage - (firstDigit*1000) - (secondDigit*100))/10;
	fourthDigit = appliedVoltage -(firstDigit*1000) - (secondDigit*100) - (thirdDigit*10);
	UART_Transmit(ASCIIConversion(firstDigit));
	UART_Transmit(ASCIIConversion(secondDigit));
	UART_Transmit(46); //decimal point
	UART_Transmit(ASCIIConversion(thirdDigit));
	UART_Transmit(ASCIIConversion(fourthDigit));
	UART_Transmit(86); //V
	UART_Transmit(34); //"
	
	 
	UART_Transmit(125); //}
	UART_Transmit(44);//,
	UART_Transmit(10); //line feed
	UART_Transmit(13); //carriage return
	 
}



void ERRORmodulator(bool errorClear,bool jamErrorFlag, bool collisionErrorFlag){  
	//error clear tells us whether we need to clear the error array
	if(!errorClear){
		UART_Transmit(34); //"
		UART_Transmit(99); //c
		UART_Transmit(108); //l
		UART_Transmit(114); //r
		UART_Transmit(34); //"
		UART_Transmit(58); //:
		UART_Transmit(34); //"
		UART_Transmit(101); //e
		UART_Transmit(119); //w
		UART_Transmit(34); //"
		UART_Transmit(44); //,
		UART_Transmit(10); //line feed
		UART_Transmit(13); //carriage return
	}else{
		UART_Transmit(10); //line feed
		UART_Transmit(13); //carriage return
	}
	
	if(collisionErrorFlag || jamErrorFlag) {
		UART_Transmit(34); //'
		UART_Transmit(101); //e
		UART_Transmit(119); //w
		UART_Transmit(34); // "
		UART_Transmit(58); //:
		UART_Transmit(91); //[
		//print errors
		
		if(jamErrorFlag){ //compressor jammed
			UART_Transmit(34); //"
			UART_Transmit(106); //j
			UART_Transmit(97); //a
			UART_Transmit(109); //m
			UART_Transmit(34); //"
			if(jamErrorFlag && collisionErrorFlag){
				UART_Transmit(44); //,
			}
		}

		if(collisionErrorFlag){ //compressor colliding
			UART_Transmit(34); //"
			UART_Transmit(99); //c
			UART_Transmit(111); //o
			UART_Transmit(108); //l
			UART_Transmit(108); //l
			UART_Transmit(105); //i
			UART_Transmit(115); //s
			UART_Transmit(105); //i
			UART_Transmit(111); //o
			UART_Transmit(110); //n
			UART_Transmit(34); //"
		}
			
			
		UART_Transmit(93); //[
		
	}else{ //no errors present
		UART_Transmit(10); //line feed
		UART_Transmit(13); //carriage return
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