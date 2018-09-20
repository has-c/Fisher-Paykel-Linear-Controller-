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
uint16_t ADC_Read();
uint8_t ADC_Calculate();




#endif /* ADC_H_ */