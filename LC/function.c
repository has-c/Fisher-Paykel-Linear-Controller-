#include <avr/io.h>
#include "prototype.h"
#include <stdbool.h>
#include <stdio.h>


//**********************************************PWM controller*******************************************//
void PWM_init(){
	TCCR1B |= (1<<WGM12)|(1<<CS10);//CTC mode & no prescaler	
	TIMSK1 |= (1<<OCIE1A)|(1<<OCIE1B); //Enabling interrupts for match on OCR1A and OCR1B
}

void PWM_control(uint16_t topValue, uint16_t bottomValue){
	OCR1A = topValue;
	OCR1B = bottomValue;
}

void DEAD_control(uint16_t time){
	OCR1A = time;
	OCR1B= 0;
}


//**********************************************ADC FUNCTIONS*********************************************//
void ADC_init(){
	// selection channel is set to internal reference 1.1V
	ADMUX |= (1<<REFS1);
	ADMUX |= (1<<REFS0);
	
	//Selects ADC5 as the analog channel selection
	ADMUX |= ((1<<MUX2)|(1<<MUX0));
	
	//set required Prescaler Select Bits
	//Turns on the ADC and sets prescaler to 64
	//enable ADC
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1);
	ADCSRA &= ~(1<<ADPS0);

	//ADCSRA |= (1<<ADIE); //enables ADC interrupt

}

int ADC_read(){
	while(!(ADCSRA &&(1<<ADIF)));
	return ADC;
}

float adc_calculation()
{
	int adcvalue = ADC_read();
	float vin =((float)adcvalue)/1024 * 5;
	return vin;
}
//****************************************COMMUNICATION FUNCTIONS***************************************//

//initializes UART transmitter
void UART_init(unsigned int BAUD_RATE){
	
	UBRR0H |= (unsigned char)(BAUD_RATE>>8); //sets the baud rate to 9600bps
	UBRR0L |= (unsigned char)(BAUD_RATE);
	UCSR0B |= (1<<TXEN0)|(1<<RXEN0); //enables UART transmitter
	UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00); //sets character size to 8 data bits

}


//UART transmitter  function
void UART_transmit(uint8_t myValue){

	while (!((1<<UDRE0)&&(UCSR0A))); //wait until the transmit register is ready
	UDR0 = myValue;//once ready, store next value for transmission
}


//unsigned char USART_receive(void){
	//while(!(UCSR0A)&(1<<RXC));
	//return UDR0;
//}


/************************************************** JSON Modulators ***********************************************/




void MFCmodulator(void){
	UART_transmit(123);
	UART_transmit(10);
	
	UART_transmit(34);
	UART_transmit(51);
	UART_transmit(34);
	UART_transmit(58);
	UART_transmit(10);
	
	UART_transmit(123);
	UART_transmit(10);

	UART_transmit(34);
	UART_transmit(109);
	UART_transmit(102);
	UART_transmit(99);
	UART_transmit(34);
	UART_transmit(58);
	UART_transmit(123);
	UART_transmit(34);
	UART_transmit(114);
	UART_transmit(101);
	UART_transmit(113);
	UART_transmit(34);
	UART_transmit(58);
	UART_transmit(34);
	UART_transmit(49); //first (Req)
	UART_transmit(48); //second (req)
	UART_transmit(48); //Third (req)
	UART_transmit(34);
	UART_transmit(44);
	UART_transmit(34);
	UART_transmit(99);
	UART_transmit(117);
	UART_transmit(114);
	UART_transmit(34);
	UART_transmit(58);
	UART_transmit(34);
	UART_transmit(48); //first (cur)
	UART_transmit(57);//second (cur)
	UART_transmit(53);//third (cur)
	UART_transmit(34);
	UART_transmit(125);
	UART_transmit(44);
	UART_transmit(10);
}

void VERmodulator(void){
	UART_transmit(34);
	UART_transmit(118);
	UART_transmit(101);
	UART_transmit(114);
	UART_transmit(34);
	UART_transmit(58);
	UART_transmit(34);
	UART_transmit(48);//set1 - first(ver)
	UART_transmit(48);//set2 - second(Ver)
	UART_transmit(49);//set 3 - third (ver)
	UART_transmit(46);//dot
	UART_transmit(48);//set 2 - first(ver)
	UART_transmit(48);//set 2 - second (ver)
	UART_transmit(50);//set 2 - thrid (ver)
	UART_transmit(46);//dot
	UART_transmit(48);//set 3 - first(Ver)
	UART_transmit(48);//set 3 - second(ver)
	UART_transmit(51);//set 3 - third(ver)
	UART_transmit(34);
	UART_transmit(44);
	UART_transmit(10);
}


void PARAMmodulator(void){
	UART_transmit(34);
	UART_transmit(112);
	UART_transmit(97);
	UART_transmit(114);
	UART_transmit(97);
	UART_transmit(109);
	UART_transmit(34);
	UART_transmit(58);
	UART_transmit(123);
	UART_transmit(34);
	UART_transmit(112);
	UART_transmit(119);
	UART_transmit(114);
	UART_transmit(34);
	UART_transmit(58);
	UART_transmit(34);
	UART_transmit(49); //first - power
	UART_transmit(46); //second - power
	UART_transmit(50); //third - power
	UART_transmit(48); // fourth - power
	UART_transmit(87);
	UART_transmit(34);
	UART_transmit(44);
	UART_transmit(34);
	UART_transmit(102);
	UART_transmit(114);
	UART_transmit(101);
	UART_transmit(113);
	UART_transmit(34);
	UART_transmit(58);
	UART_transmit(34);
	UART_transmit(49); //first - frequency
	UART_transmit(48); //second - frequency
	UART_transmit(48); //third - frequency
	UART_transmit(72);
	UART_transmit(122);
	UART_transmit(34);
	UART_transmit(44);
	UART_transmit(34);
	UART_transmit(99);
	UART_transmit(117);
	UART_transmit(114);
	UART_transmit(114);
	UART_transmit(34);
	UART_transmit(58);
	UART_transmit(34);
	UART_transmit(48);//first - current
	UART_transmit(57);//second - current
	UART_transmit(55);//third - current
	UART_transmit(109);
	UART_transmit(65);
	UART_transmit(34);
	UART_transmit(44);
	UART_transmit(34);
	UART_transmit(118);
	UART_transmit(111);
	UART_transmit(108);
	UART_transmit(116);
	UART_transmit(34);
	UART_transmit(58);
	UART_transmit(34);
	UART_transmit(49);//first - voltage
	UART_transmit(50);//second - voltage
	UART_transmit(46);//dot
	UART_transmit(51);//third - voltage
	UART_transmit(50);//fourth - voltage
	UART_transmit(86);
	UART_transmit(34);
	UART_transmit(125);
	UART_transmit(44);
	UART_transmit(10);
}


void ERRORmodulator(void){
	UART_transmit(34);
	UART_transmit(99);
	UART_transmit(108);
	UART_transmit(114);
	UART_transmit(34);
	UART_transmit(58);
	UART_transmit(34);
	UART_transmit(101);
	UART_transmit(119);
	UART_transmit(34);
	UART_transmit(44);
	UART_transmit(10);

	UART_transmit(34);
	UART_transmit(101);
	UART_transmit(119);
	UART_transmit(34);
	UART_transmit(58);
	UART_transmit(91);
	UART_transmit(34);
	UART_transmit(99);
	UART_transmit(109);
	UART_transmit(112);
	UART_transmit(114);
	UART_transmit(83);
	UART_transmit(116);
	UART_transmit(97);
	UART_transmit(108);
	UART_transmit(108);
	UART_transmit(101);
	UART_transmit(100);
	UART_transmit(34);
	UART_transmit(44);
	UART_transmit(34);
	UART_transmit(98);
	UART_transmit(108);
	UART_transmit(111);
	UART_transmit(99);
	UART_transmit(107);
	UART_transmit(101);
	UART_transmit(100);
	UART_transmit(68);
	UART_transmit(117);
	UART_transmit(99);
	UART_transmit(116);
	UART_transmit(34);
	UART_transmit(93);
	UART_transmit(10);

	UART_transmit(125);
	UART_transmit(10);
	UART_transmit(125);
}



