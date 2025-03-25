
#include "display.h"


/** @brief Display pages manager function */
HAL_StatusTypeDef display_routine(TIM_HandleTypeDef *htim, uint8_t knob_position, 
                                  uint16_t ADC_value, Signals_folderTypedef *signals_handle, 
                                  FSM_HandleTypeDef *fsm_handle, HAL_StatusTypeDef *error_handle){

    static uint32_t entry_time = 0;
    static uint32_t exit_time = 0;
    static uint8_t timeout_counter = 0;
    static uint8_t old_knob_pos = 0;

    //if timeout occurred in the previous run of the function reinitialize the display
    if (exit_time - entry_time > Display_Timeout){

        if(timeout_counter >= MAX_TIMEOUT){
            return HAL_ERROR;
        }

        timeout_counter ++;

        I2C_LCD_Init(MyI2C_LCD);
        entry_time = 0;
        exit_time = 0;
        return HAL_TIMEOUT;
    }

    entry_time = HAL_GetTick();

    if (entry_time - exit_time < refresh_rate) {

        return HAL_OK;
    }


    

    switch (knob_position) {
        case 0:
            old_knob_pos = display_routine_0(ADC_value, fsm_handle, error_handle, signals_handle);
            break;

        case 1:
            old_knob_pos = display_routine_1();
            break;

        case 2:
            old_knob_pos = display_routine_2();
            break;

        default:
            old_knob_pos = display_routine_0();
            break;
    }

    EncoderStart(htim);

    exit_time = HAL_GetTick();

    return;
    
}



/** @brief display page: temperature, state of charge */
uint8_t display_routine_0(uint16_t ADC_value, FSM_HandleTypeDef *fsm_handle,
                          HAL_StatusTypeDef *error_handle, Signals_folderTypedef *signals_handle ) {
    
    bool volatile extern fungo_pressed;

    static char buffer[30U] = "";

    //temperature extern
    I2C_LCD_Home(I2C_LCD);
    sprintf(buffer, "Ext Temp    : %d C ", ADC_value);
    I2C_LCD_WriteString(I2C_LCD, (char *)&buffer);

    //State
    I2C_LCD_ACapo(I2C_LCD);
    switch (fsm_handle->current_state) {
        case FSM_SCARRELLINO_FSM_IDLE:
            sprintf(buffer, "State : %s   ", idle_state_buff);
            break;

        case FSM_SCARRELLINO_FSM_TSON:
            sprintf(buffer, "State : %s   ", tson_state_buff);
            break;

        case FSM_SCARRELLINO_FSM_CHARGE:
            sprintf(buffer, "State : %s   ", charge_state_buff);
            break;
            
        case FSM_SCARRELLINO_FSM_STOP_CHARGE:
            sprintf(buffer, "State : %s   ", stop_charge_state_buff);
            break;
        
    }

    I2C_LCD_WriteString(I2C_LCD, (char *)&buffer);

    // state of charge
    I2C_LCD_ACapo(I2C_LCD);
    sprintf(buffer, "HV Batt SOC : %.1f%% ", signals_handle->SOC);

    I2C_LCD_WriteString(I2C_LCD, (char *)&buffer);
    I2C_LCD_ACapo(I2C_LCD);

    //if fungo has not been pressed
    if(fungo_pressed == 0){

        //charging temperature if it is in CHARGE state
        if (hfsm.current_state == FSM_SCARRELLINO_FSM_CHARGE) {
            sprintf(buff, "Charge Temp : %.0f C ", charge_temp);
            I2C_LCD_WriteString(I2C_LCD, (char *)&buff);

        }

        //Error code if it isn't in CHARGE state and there is an error
        else if (error_code != 30) {
            sprintf(buff, "Error Code : %i    ", error_code);
            I2C_LCD_WriteString(I2C_LCD, (char *)&buff);

        }

        //if it isn't neither in error neither in CHARGE state
        else {
            char static void_buffer[21] = "                   ";
            I2C_LCD_WriteString(I2C_LCD, (char *)&void_buffer);
        }

    }

    else{
        sprintf(buff, "Fungo Pressed, reset");
        I2C_LCD_WriteString(I2C_LCD, (char *)&buff);
    }

    /*
    I2C_LCD_ACapo(I2C_LCD);
    sprintf(buff, "position = %d  ", position);
    I2C_LCD_WriteString(I2C_LCD,(char *) &buff);
*/

    //set the last page displayed
    old_position = 0;
}


/** @brief display page 1: min e max */
uint8_t display_routine_1(uint16_t ADC_value, FSM_HandleTypeDef *fsm_handle, HAL_StatusTypeDef *error_handle, Signals_folderTypedef *signals_handle ) {
    I2C_LCD_Home(I2C_LCD);

    char static cell_info_buffer[15] = "CELLS INFO";

    I2C_LCD_WriteString(I2C_LCD, (char *)&cell_info_buffer);
    I2C_LCD_ACapo(I2C_LCD);

    sprintf(buff, "v max  =%.2f ID=%.0f", v_max_rx, v_max_id_rx);
    I2C_LCD_WriteString(I2C_LCD, (char *)&buff);
    I2C_LCD_ACapo(I2C_LCD);

    sprintf(buff, "v min  =%.2f ID=%.0f", v_min_rx, v_min_id_rx);
    I2C_LCD_WriteString(I2C_LCD, (char *)&buff);
    I2C_LCD_ACapo(I2C_LCD);

    sprintf(buff, "v mean =%.2f", v_mean_rx);
    I2C_LCD_WriteString(I2C_LCD, (char *)&buff);
    I2C_LCD_ACapo(I2C_LCD);

    /*
    sprintf(buff, "position = %d  ", position);
    I2C_LCD_WriteString(I2C_LCD,(char *) &buff);
*/

    //set the last page displayed
    old_position = 1;
}



/** @brief display page 2  */
uint8_t display_routine_2(uint16_t ADC_value, FSM_HandleTypeDef *fsm_handle, HAL_StatusTypeDef *error_handle, Signals_folderTypedef *signals_handle ) {
    I2C_LCD_Home(I2C_LCD);

    sprintf(buff, "CHARGE CURRENT");
    I2C_LCD_WriteString(I2C_LCD, (char *)&buff);
    I2C_LCD_ACapo(I2C_LCD);

    sprintf(buff, "I = %.2f", charging_curr);
    I2C_LCD_WriteString(I2C_LCD, (char *)&buff);
    I2C_LCD_ACapo(I2C_LCD);

    //sprintf(buff, "MAINS");
    //I2C_LCD_WriteString(I2C_LCD,(char *) &buff);
    //I2C_LCD_ACapo(I2C_LCD);
    //
    //sprintf(buff, "V = %.2f I = %.2f", mains_v_rx, mains_i_rx);
    //I2C_LCD_WriteString(I2C_LCD,(char *) &buff);
    //I2C_LCD_ACapo(I2C_LCD);

    /*   
    sprintf(buff, "position = %d  ", position);
    I2C_LCD_WriteString(I2C_LCD,(char *) &buff);
*/

    //set the last page displayed
    old_position = 2;
}

/** @brief display errors  */
void error_display() {
    char error_buffer[40] = "";

    //aggiungi errore sconosciuto
#define display_error                                             \
    do {                                                          \
        I2C_LCD_Clear(I2C_LCD);                                   \
        I2C_LCD_Home(I2C_LCD);                                    \
        if (error_code != 30)                                     \
            sprintf(error_buffer, "error code: %d ", error_code); \
        else                                                      \
            sprintf(error_buffer, "error code: unknown ");        \
        I2C_LCD_WriteString(I2C_LCD, (char *)&error_buffer);      \
                                                                  \
        I2C_LCD_ACapo(I2C_LCD);                                   \
        sprintf(error_buffer, "CHARGE STOPPED ");                 \
        I2C_LCD_WriteString(I2C_LCD, (char *)&error_buffer);      \
    } while (0)

    switch (error_code) {
        case 0:
            strcpy(error_buffer, "init fsm error");
            break;

        case 1:
            strcpy(error_buffer, "start fsm error");
            break;

        case 2:
            strcpy(error_buffer, "CAN start error");
            break;

        case 3:
            strcpy(error_buffer, "CAN IT activation error");
            break;

        case 4:
            strcpy(error_buffer, "CAN generic error");
            break;

        case 5:
            strcpy(error_buffer, "CAN Rx error");
            break;

        case 6:
            strcpy(error_buffer, "CAN 2 start error");
            break;

        case 7:
            strcpy(error_buffer, "watchdog error");
            break;

        case 8:
            strcpy(error_buffer, "can send error");
            break;
    }

    display_error;
    //HAL_UART_Transmit(&LOG_UART, (uint8_t *)&error_buffer, strlen(error_buffer), 10);
}