/*
 * adc.c
 *
 * Created: 15/09/2018 5:24:17 PM
 *  Author: hasna
 */ 
  
#include "adc.h"

void ADC_Init(){
	// selection channel is set to AREF
	ADMUX &= ~(1<<REFS1);
	ADMUX &= ~(1<<REFS0);
	
	//Selects ADC5 as the analog channel selection
	ADMUX &= ~(1<<MUX3)|(1<<MUX2)|(1<<MUX0);
	ADMUX |= (1<<MUX1);
	
	//set required Prescaler Select Bits
	//Turns on the ADC and sets prescaler to 64
	//enable ADC
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1);
	ADCSRA &= ~(1<<ADPS0);
	
	ADCSRA |= (1<<ADSC);

}

//polling ADC 
uint32_t ADC_Read(){
	while(!(ADCSRA &&(1<<ADIF)));
	return ADC;
}

//note: do not use float as ATMEGA328P does not have a floating point unit. It uses integers to perform float operations thus use integers and multiply them by 1000 to
//get millivolt(mV) accuracy 
uint32_t ADC_Calculate(){
	uint32_t  adcValue = ADC_Read();
	uint32_t measuredVoltage = ((adcValue*1000)/1024) * 5;
	return measuredVoltage;
}

uint32_t ADC_LHSVoltage(){
	ADMUX &= ~((1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0));
	ADCSRA |= (1<<ADSC);
	uint32_t adc_val = ADC_Calculate();
	return (adc_val*(27));
}

uint32_t ADC_RHSVoltage(){
	ADMUX &= ~((1<<MUX3)|(1<<MUX2)|(1<<MUX1));
	ADMUX |= (1<<MUX0);
	ADCSRA |= (1<<ADSC);
	uint32_t adc_val = ADC_Calculate();
	return (adc_val*(2.7));
}

uint32_t ADC_Current(){
	ADMUX &= ~((1<<MUX3)|(1<<MUX2)|(1<<MUX0));
	ADMUX |= (1<<MUX1);
	ADCSRA |= (1<<ADSC);
	uint32_t adc_val = ADC_Calculate();
	return ((adc_val*4.8 - (200*4.8))*4);
}


void ADC_ChannelSwitching(){
	switch(ADMUX){
		case 0b01000000: //channel 0 Hall sensor 1
			ADMUX = 0b01000001;
		case 0b01000001: //channel 1 Hall sensor 2
			ADMUX = 0b01000010; //change if using all	
		case 0b01000010: //channel 2 Voltage Sensor 1
			ADMUX = 0b01000011;
		case 0b01000011: //channel 3 Voltage sensor 2
			ADMUX = 0b01000100;
		case 0b01000100: //channel 4 Current Sensor
			ADMUX = 0b01000000;

		ADCSRA |= (1<<ADSC); //restart ADC conversion
	}
}


	