#ifndef  NTC_H
#define  NTC_H


#include "main.h"
#include "stdbool.h"
#include "adc.h"

//costants for the temperature measure
#define A       0.00335401643468053f
#define B       0.000256523550896126f
#define C       0.00000260597012072052f
#define D       0.000000063292612648746f
#define R25     10000.0f

#define ADC_Val     3.6f
#define Val         5.0f
#define Rpu         10000.0f 

#ifndef NaN
#define NaN -1
#endif


//ntc related functions

/** @brief get temp from res */
uint16_t get_temperature(float resistance);

/** @brief get res from ntc_voltage */
uint16_t get_resistance(uint16_t ADC_value);

/**
 * @brief Function to measure the external temperature through the NTC sensor
 * @param ADC_voltage ADC voltage measure
 * @return Temperature value
 */
uint16_t temp_meas(ADC_voltage);



/** @brief higher level function that measures the temperature from the ntc voltage */
uint16_t get_ntc_temperature(uint16_t ADC_value);

#endif