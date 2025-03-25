#include "Encoder.h"


/**
 * @brief IRQ that handles the rotary encoder 
 */
uint8_t encoder_IRQ(TIM_HandleTypeDef *htim) {

    // position of the knob of the encoder
    uint8_t static position = 0;
 
    // trough the casting at two's complement (int16_t) we obtain a negative value in case of underflow
    // every rotation the timer encreases of 4 ticks, so we divide by 4
    position = (uint8_t)((int16_t)__HAL_TIM_GetCounter(htim)) / 4;


    if (position > disp_pages) {
        __HAL_TIM_SET_COUNTER(htim, disp_pages * 4U);
        position = disp_pages;
    }

    else if (position < 0U) {
        __HAL_TIM_SET_COUNTER(htim, 0U);
        position = 0U;
    }

    // stop the timer to fix the position
    EncoderStop(htim);

    return position;


    

}

/** 
 * @brief Adjust the encoder position if it goes out of the display pages
 */
uint8_t encoder_position_adjustment(TIM_HandleTypeDef *htim) {


    

    

    //if (position != old_position){
    //    I2C_LCD_Clear(I2C_LCD);
    //}

    return;
}