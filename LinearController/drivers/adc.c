/*
 * adc.c
 *
 * Created: 15/09/2018 5:24:17 PM
 *  Author: hasna
 */ 
  
#include "adc.h"

void ADC_Init(){
	// selection channel is set to internal reference 1.1V
	//ADMUX |= (1<<REFS1);
	//ADMUX |= (1<<REFS0);
	
	//Selects ADC5 as the analog channel selection
	ADMUX |= ((1<<MUX2)|(1<<MUX0));
	
	//set required Prescaler Select Bits
	//Turns on the ADC and sets prescaler to 64
	//enable ADC
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1);
	ADCSRA &= ~(1<<ADPS0);

}

//polling ADC 
uint16_t ADC_Read(){
	while(!(ADCSRA &&(1<<ADIF)));
	return ADC;
}

//note: do not use float as ATMEGA328P does not have a floating point unit. It uses integers to perform float operations thus use integers and multiply them by 1000 to
//get millivolt(mV) accuracy 
uint8_t ADC_Calculate(){
	uint16_t  adcValue = ADC_Read();
	uint8_t measuredVoltage =(adcValue*1000)/1024 * 5;
	return measuredVoltage;
}

void ADC_ChannelSwitching(){
	switch(ADMUX){
		case 0b01000000: //channel 0 Hall sensor 1
		//adcValue = ((float)ADC_read()*5.0)/1024.0;
		//printf("ADC0's value is  : %f\n", adcValue);
		ADMUX = 0b01000001;
		break;
		case 0b01000001: //channel 1 Hall sensor 2
		//adcValue = ((float)ADC_read()*5.0)/1024.0;
		//printf("ADC1's value is : %f\n", adcValue);
		ADMUX = 0b01000010; //change if using all
		break;
		case 0b01000010: //channel 2 Voltage Sensor 1
		//adcValue = ((float)ADC_read()*5.0)/1024.0;
		//printf("ADC2's value is : %f\n", adcValue);
		ADMUX = 0b01000011;
		break;
		case 0b01000011: //channel 3 Voltage sensor 2
		//adcValue = ((float)ADC_read()*5.0)/1024.0;
		//printf("ADC3's value is : %f\n", adcValue);
		ADMUX = 0b01000100;
		break;
		case 0b01000100: //channel 4 Current Sensor
		//adcValue = ((float)ADC_read()*5.0)/1024.0;
		//printf("ADC4's value is : %f\n", adcValue);
		ADMUX = 0b01000000;
		break;

		ADCSRA |= (1<<ADSC); //restart ADC conversion
	}
}


	