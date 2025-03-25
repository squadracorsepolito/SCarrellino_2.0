#include "ntc.h"


extern bool flag_ntc;

float adc_conv_bit(ADC_HandleTypeDef *handle){

    switch (handle->Init.Resolution)   
    {                       
    case ADC_RESOLUTION12b:    
        
        return 12.0f;
        break;
    
    case ADC_RESOLUTION10b:
        
        return 10.0f;
        break;

    case ADC_RESOLUTION8b:

        return 8.0f;
        break;
    
    case ADC_RESOLUTION6b:

        return 6.0f;
        break;
    }
}


uint16_t get_resistance(uint16_t ADC_value){
    
    float ntc_voltage = ADC_Val*(ADC_value/(pow(2,adc_conv_bit(&hadc1))));
    float resistance = (ntc_voltage*Rpu)/(Val-ntc_voltage) + 3000;
    return resistance;
}


uint16_t get_temperature(float resistance){
    float Temperature = 1/(A + B*(log(resistance/R25)) + C*(pow(log(resistance/R25),2)) + D*(pow(log(resistance/R25), 3)) ) - 273.15;
    
    return (uint16_t) Temperature; 
}


uint16_t get_ntc_temperature(uint16_t ADC_value){

    uint16_t resistance = get_resistance(ADC_value);
    uint16_t  temperature = get_temperature(resistance);
    
    return temperature;

}



uint16_t temp_meas(ADC_value){

    if((ADC_value < 0U) || (ADC_value > 100U)){
        return NaN;
    }

    static uint16_t prev_value = 0U;

    if(ADC_value != prev_value){

        prev_value = ADC_value;
        return get_ntc_temperature(ADC_value);
    }

    return prev_value;

}

