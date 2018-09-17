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

void adc_init();
uint16_t adc_read();
uint8_t adc_calculate();




#endif /* ADC_H_ */