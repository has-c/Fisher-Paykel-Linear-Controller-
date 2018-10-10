/*
 * adc.c
 *
 * Created: 15/09/2018 5:24:17 PM
 *  Author: Hasnain
 */ 
  
#include "adc.h"

/**************************************************************Macros**************************************************************/
#define ADC_RESOLUTION 1024
#define UPPER_LIMIT_VOLTAGE 5
#define OPAMP_VOLTAGE_GAIN 27
#define LEVEL_SHIFT

/**************************************************************ADC Intialisation**************************************************************/
void ADC_Init(){
	ADMUX &= ~(1<<REFS1 | ~(1<<REFS0);										//Select Reference Channel to be AREF
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);					//Enables ADC and adds a prescaler of 128 bits
}

/**************************************************************ADC Polling**************************************************************/
uint32_t ADC_Read(){
	while(!(ADCSRA &&(1<<ADIF)));
	return ADC;
}

/**************************************************************ADC Conversion**************************************************************/
/*Note this could've done using floats but our microcontroller does not have a floating point unit and thus computations using floats is more 
computationally extensive. Hence we opted for unsigned integers and are forced to divide by powers of 10 in various places*/ 
uint32_t ADC_Calculate(){
	uint32_t  adcValue = ADC_Read();
	uint32_t measuredVoltage = ((adcValue*1000)/ADC_RESOLUTION) * UPPER_LIMIT_VOLTAGE;						
	return measuredVoltage;
}

/**************************************************************Read LHS Voltage**************************************************************/
uint32_t ADC_LHSVoltage(){
	ADMUX &= ~((1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0));
	ADCSRA |= (1<<ADSC);
	uint32_t lhsVoltage = ADC_Calculate(); //calculates measured voltage 
	return (lhsVoltage*OPAMP_VOLTAGE_GAIN/100); //return a 4 digit number that is the LHS voltage
}

/**************************************************************Read RHS Voltage**************************************************************/
uint32_t ADC_RHSVoltage(){
	ADMUX &= ~((1<<MUX3)|(1<<MUX2)|(1<<MUX1));
	ADMUX |= (1<<MUX0);
	ADCSRA |= (1<<ADSC);
	uint32_t rhsVoltage = ADC_Calculate();
	return (rhsVoltage*OPAMP_VOLTAGE_GAIN/100);
}




	