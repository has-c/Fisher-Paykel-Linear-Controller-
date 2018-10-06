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

void ADC_Init();
int ADC_Read();
uint32_t ADC_Calculate();
int ADC_LHSVoltage();
int ADC_RHSVoltage();
int ADC_Current();
void ADC_ChannelSwitching();



#endif /* ADC_H_ */