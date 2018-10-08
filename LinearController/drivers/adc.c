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
	//Turns on the ADC and sets prescaler to 32
	//enable ADC
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS0);	
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
	uint32_t lhsVoltage = ADC_Calculate(); //calculates measured voltage 
	return (lhsVoltage*27/100); //return a 4 digit number that is the LHS voltage
}

uint32_t ADC_RHSVoltage(){
	ADMUX &= ~((1<<MUX3)|(1<<MUX2)|(1<<MUX1));
	ADMUX |= (1<<MUX0);
	ADCSRA |= (1<<ADSC);
	uint32_t rhsVoltage = ADC_Calculate();
	return (rhsVoltage*27/100);
}

uint32_t ADC_Current(){
	ADMUX &= ~((1<<MUX3)|(1<<MUX2)|(1<<MUX0));
	ADMUX |= (1<<MUX1);
	ADCSRA |= (1<<ADSC);
	uint32_t instantaneousCurrent = ADC_Calculate(); //1000
	return ((instantaneousCurrent-200)*10/48*4);
}



	