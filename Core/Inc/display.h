#ifndef DISPLAY_H
#define DISPLAY_H

#include "main.h"

#include "I2C_LCD.h"
#include "Encoder.h"
#include "ECU_level_functions.h"
#include "scarrellino_fsm.h"


/**
 * @note time in milliseconds
 */
#define Display_Timeout 100

/**
 * @brief Max number of timeout to disable the display
 */
#define MAX_TIMEOUT     2 

#define freq 50.0f // Hz

/**
 * @brief Screen refresh rate period, with frequency freq, in millisecond
 */
#define refresh_rate (uint8_t)(((1.0f)/freq)*(1000.0f))


#define idle_state_buff         "IDLE"
#define tson_state_buff         "TSON"
#define charge_state_buff       "CHARGE"
#define stop_charge_state_buff  "STOP CHARGE"


HAL_StatusTypeDef display_routine(TIM_HandleTypeDef *htim, uint8_t knob_position, uint16_t ADC_value, Signals_folderTypedef *signals_handle, FSM_HandleTypeDef *fsm_handle, HAL_StatusTypeDef *error_handle);
uint8_t display_routine_0(uint16_t ADC_value, FSM_HandleTypeDef *fsm_handle, HAL_StatusTypeDef *error_handle, Signals_folderTypedef *signals_handle );
uint8_t display_routine_1(uint16_t ADC_value, FSM_HandleTypeDef *fsm_handle, HAL_StatusTypeDef *error_handle, Signals_folderTypedef *signals_handle );
uint8_t display_routine_2(uint16_t ADC_value, FSM_HandleTypeDef *fsm_handle, HAL_StatusTypeDef *error_handle, Signals_folderTypedef *signals_handle );

#endif