/*
 * adc.h
 *
 * Created: 15/09/2018 5:24:26 PM
 *  Author: hasna
 */ 

#include <avr/io.h>
#include <stdio.h>


#ifndef ADC_H_
#define ADC_H_

/**************************************************************ADC Declarations**************************************************************/
void ADC_Init();
uint32_t ADC_Read();
uint32_t ADC_Calculate();
uint32_t ADC_LHSVoltage();
uint32_t ADC_RHSVoltage();



#endif /* ADC_H_ */