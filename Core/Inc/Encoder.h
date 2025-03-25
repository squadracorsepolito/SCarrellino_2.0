#ifndef ENCODER_H
#define ENCODER_H

#include "main.h"

#include "tim.h"
#include "I2C_LCD.h"

/**
 * @brief number of display pages minus 1
 */
#define disp_pages 3-1

/**
 * @brief Stop the timer to snap the encoder knob position
 */
#define EncoderStop(htim)   HAL_TIM_Encoder_Stop_IT(htim, TIM_CHANNEL_ALL);

/**
 * @brief Restart the timer to catch variations in the encoder knob position
 */
#define EncoderStart(htim)   HAL_TIM_Encoder_Start_IT(htim, TIM_CHANNEL_ALL);




struct encoder_struct {
    uint16_t          counter;
    int16_t           count;
    int16_t           position;
    uint8_t           old_position;
    TIM_HandleTypeDef htim;
    uint8_t           number_of_positions;
} EncoderTypedef;

/**
 * @brief Encoder IRQ
 * @param *htim Pointer to the encoder mode timer
 * @return Position of the knob of the encoder (max = disp_pages)
 */
uint8_t encoder_IRQ(TIM_HandleTypeDef *htim);


#endif