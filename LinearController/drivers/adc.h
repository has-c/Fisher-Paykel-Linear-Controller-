/*
 * adc.h
 *
 * Created: 15/09/2018 5:24:26 PM
 *  Author: hasna
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

void adc_init();
uint16_t adc_read();
float adc_calculate();




#endif /* ADC_H_ */