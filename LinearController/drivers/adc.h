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
uint32_t ADC_Read();
uint32_t ADC_Calculate();
uint32_t ADC_LHSVoltage();
uint32_t ADC_RHSVoltage();
uint32_t ADC_Current();
void ADC_ChannelSwitching();



#endif /* ADC_H_ */