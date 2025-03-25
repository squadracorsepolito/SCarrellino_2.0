#include "main_routine.h"


HAL_StatusTypeDef           herror[number_of_errors];
Signals_folderTypedef       hmessages;
volatile Rx_CAN_Typedef     hcan_rx[can_message_rx_number];
SW_Watchdog_HandleTypedef   hWD[number_of_watchdogs];


extern FSM_HandleTypeDef           hfsm;


void can_routine(){

    extern volatile data_flagTypedef   can_send_flag;

    can_rx_routine(&hcan_rx, &hmessages, &hWD);

    herror[CAN_send_] = can_tx_routine(&can_send_flag);

    if (hmessages.SOC = SOC_Evaluation(hmessages.v_cells.hvb_u_cell_mean) == NaN) {
        herror[SOC_] = HAL_ERROR;
    }

    IMD_AMS_error_handler(&hmessages);

    return;

}



void System_init(){

  I2C_LCD_Init(MyI2C_LCD);
  _FSM_init(&hfsm);
  can_messages_init(&hmessages);
  
  #ifdef Watchdog

  can_WD_init(&hWD);
  can_WD_start(&hWD);
  
  #endif

  return;

}

void charge_control_routine(){

    buzzer_routine();

    if(FSM_routine(&hfsm) != HAL_OK){
        // blocca carica
    }
    
    TSAC_FAN_routine(&hmessages.t_cells.hvb_t_cell_max);

    return;


}



void HMI_routine(){

    extern volatile uint8_t  knob_position;
    extern volatile uint16_t ADC_value;


    static uint16_t          ext_temp = 0;


    if(ext_temp = temp_meas(ADC_value) == NaN){
        herror[Temperature_] = HAL_ERROR; 
    }
    
    
    herror[Display_] = display_routine(&htim3, knob_position, ADC_value, &hmessages, &hfsm, &herror);

    return;


}

void watchdog_routine(){

    herror[Watchdog_] = can_WD_routine(&hWD);

    return;

        
}