/*
 * adc.c
 *
 * Created: 15/09/2018 5:24:17 PM
 *  Author: hasna
 */ 

#include "drivers/adc.h"

void adc_init(){
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

//polling ADC 
uint16_t adc_read(){
	while(!(ADCSRA &&(1<<ADIF)));
	return ADC;
}

//note: do not use float as ATMEGA328P does not have a floating point unit. It uses integers to perform float operations thus use integers and multiply them by 1000 to
//get millivolt(mV) accuracy 
uint8_t adc_calculate(){
	uint16_t  adcValue = adc_read();
	uint8_t measuredVoltage =(adcValue*1000)/1024 * 5;
	return measuredVoltage;
}