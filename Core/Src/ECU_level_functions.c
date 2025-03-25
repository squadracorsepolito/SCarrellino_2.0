#include "ECU_level_functions.h"

extern volatile data_flagTypedef can_send_flag;

extern volatile uint8_t ADC_conv_flag;

char state[15];
char buff[30];
extern FSM_HandleTypeDef hfsm;


//value of the timer
extern volatile uint16_t counter;

//value of the timer with sign
extern volatile int16_t count;

uint8_t old_position;

extern uint8_t number_of_positions;

uint8_t volatile extern error_code;






/** 
 * @brief Function to start a software timer
 */
uint32_t MilElapsed(uint8_t reset) {
    uint32_t static start;

    if (reset == 1){
        start = HAL_GetTick();
    }
    
    uint32_t mil = HAL_GetTick() - start;

    return mil;
}

extern uint8_t buzzer_stop_charge_on;


/**
* @brief Controls the buzzer routines
*/
void buzzer_routine(){

    extern bool buzzer_charge_on;
    
    
    uint8_t static flag = 0;
    
    if(buzzer_charge_on == 1){

        if(flag == 0){

            MilElapsed(1);
            buzzer_800hz();

            #ifndef silence
            buzzer_on();
            #endif

            flag = 1;
        }

        if ((MilElapsed(0) >= 300) & (flag == 1)){
            buzzer_1khz();
            flag = 2;
            }

        if((MilElapsed(0) >= 600) & (flag == 2)){
            buzzer_12khz();
            flag = 3;
        }

        if((MilElapsed(0) >= 900) & (flag == 3)){
            buzzer_off();
            buzzer_charge_on = 0;
            flag = 0;
        }    
        
    }

    else if(buzzer_stop_charge_on == 1){

        if(flag == 0){
            MilElapsed(1);
            buzzer_12khz();

            #ifndef silence
            buzzer_on();
            #endif
            
            flag = 1;
        }
        if ((MilElapsed(0) >= 300) & (flag == 1)){
            buzzer_1khz();
            flag = 2;
            }
        if((MilElapsed(0) >= 600) & (flag == 2)){
            buzzer_800hz();
            flag = 3;
        }
        if((MilElapsed(0) >= 900) & (flag == 3)){
            buzzer_off();
            buzzer_stop_charge_on = 0;
            flag = 0;
        }    
    }
}




void TSAC_FAN_routine(uint16_t cell_temp) {
    
    if (cell_temp <= 29){
        TSAC_fan_off();
        return;
    }

    if ((cell_temp > 29) && (cell_temp <= 39)) {
        TSAC_fan_half();
        return;
    }

    if (cell_temp > 39) {
        TSAC_fan_max();
        return;
    }
}







/** 
 * @brief Start the delay to separate the closing of the AIRs and the start of the charge
 */
void start_charge_delay() {
    HAL_TIM_Base_Start_IT(&htim7);
}

uint8_t charge_on = 0;

/**
 * @brief Action to do to start the charge after the delay set for the closing of the AIRs
 */
void end_charge_delay() {
    if (hfsm.current_state == Charge) {
        ChargeENcmdON();
        ChargeBlueLedOn();
        __HAL_TIM_SET_COUNTER(&htim7, 0);
        HAL_TIM_Base_Stop_IT(&htim7);
    }

    else {
        ChargeENcmdOFF();
        ChargeBlueLedOff();
        __HAL_TIM_SET_COUNTER(&htim7, 0);
        HAL_TIM_Base_Stop_IT(&htim7);
    }
}



CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];
extern uint8_t volatile can_rx_flag;





double extern SOC;



void stop_charge_routine() {

    extern uint8_t volatile AIR_CAN_Cmd;

    ChargeENcmdOFF();
    TSAC_fan_off();
    ChargeBlueLedOff();
    buzzer_stop_charge_on = 1;
    AIR_CAN_Cmd           = 0;
}



uint8_t charge_control() {
    if (ChargeENcmdState() == ON) {
        if (charging_curr < 0.5)
            return HAL_ERROR;

        else {
            return HAL_OK;
        }
    }

    else {
        return HAL_OK;
    }
}

/**
 * @brief Function thath sets all the watchdog
 */
/*

void can_WD_set() {
    can_SW_Watchdog_init(&HVCB_HVB_RX_V_CELL_FRAME, HVCB_HVB_RX_V_CELL_CYCLE_TIME_MS*5);
    can_SW_Watchdog_init(&HVCB_HVB_RX_T_CELL_FRAME, HVCB_HVB_RX_T_CELL_CYCLE_TIME_MS*5);
    can_SW_Watchdog_init(&HVCB_HVB_RX_SOC_FRAME, HVCB_HVB_RX_SOC_CYCLE_TIME_MS*5);
    can_SW_Watchdog_init(&HVCB_HVB_RX_MEASURE_FRAME, HVCB_HVB_RX_MEASURE_CYCLE_TIME_MS*5);
    can_SW_Watchdog_init(&MCB_TLB_BAT_SD_CSENSING_STATUS_FRAME, MCB_TLB_BAT_SD_CSENSING_STATUS_CYCLE_TIME_MS*5);
    can_SW_Watchdog_init(&MCB_TLB_BAT_SIGNALS_STATUS_FRAME, MCB_TLB_BAT_SIGNALS_STATUS_CYCLE_TIME_MS*5);
    can_SW_Watchdog_init(&HVCB_HVB_RX_DIAGNOSIS_FRAME,HVCB_HVB_RX_DIAGNOSIS_CYCLE_TIME_MS*5); 
}

*/

void can_WD_init(SW_Watchdog_HandleTypedef *handle_array){
    
    SW_Watchdog_HandleTypedef hWD_HVCB_HVB_RX_V_CELL,
                        hWD_HVCB_HVB_RX_T_CELL,
                        hWD_HVCB_HVB_RX_SOC,
                        hWD_HVCB_HVB_RX_MEASURE,
                        hWD_MCB_TLB_BAT_SD_CSENSING_STATUS,
                        hWD_MCB_TLB_BAT_SIGNALS_STATUS,
                        hWD_HVCB_HVB_RX_DIAGNOSIS;

    
    //handles saving in array of WD handles
    handle_array[HVCB_HVB_RX_V_CELL_]             = hWD_HVCB_HVB_RX_V_CELL;
    handle_array[HVCB_HVB_RX_T_CELL_]             = hWD_HVCB_HVB_RX_T_CELL;
    handle_array[HVCB_HVB_RX_MEASURE_]            = hWD_HVCB_HVB_RX_MEASURE;
    handle_array[MCB_TLB_BAT_SD_CSENSING_STATUS_] = hWD_MCB_TLB_BAT_SD_CSENSING_STATUS;
    handle_array[MCB_TLB_BAT_SIGNALS_STATUS_]     = hWD_MCB_TLB_BAT_SIGNALS_STATUS;
    handle_array[HVCB_HVB_RX_DIAGNOSIS_]          = hWD_HVCB_HVB_RX_DIAGNOSIS;

    //WD values assignment 
    SW_Watchdog_init(&handle_array[HVCB_HVB_RX_V_CELL_],             HVCB_HVB_RX_V_CELL_CYCLE_TIME_MS*5);
    SW_Watchdog_init(&handle_array[HVCB_HVB_RX_T_CELL_],             HVCB_HVB_RX_T_CELL_CYCLE_TIME_MS*5);
    SW_Watchdog_init(&handle_array[HVCB_HVB_RX_MEASURE_],            HVCB_HVB_RX_MEASURE_CYCLE_TIME_MS*5);
    SW_Watchdog_init(&handle_array[MCB_TLB_BAT_SD_CSENSING_STATUS_], MCB_TLB_BAT_SD_CSENSING_STATUS_CYCLE_TIME_MS*5);
    SW_Watchdog_init(&handle_array[MCB_TLB_BAT_SIGNALS_STATUS_],     MCB_TLB_BAT_SIGNALS_STATUS_CYCLE_TIME_MS*5);
    SW_Watchdog_init(&handle_array[HVCB_HVB_RX_DIAGNOSIS_],          HVCB_HVB_RX_DIAGNOSIS_CYCLE_TIME_MS*5); 

    return;
}

/**
 * @brief Starts all the watchdogs
 */
STMLIBS_StatusTypeDef can_WD_start(SW_Watchdog_HandleTypedef *handle_array){

    uint32_t entry_time    = HAL_GetTick();
    uint32_t timeout = 2U; // millisecond 

    for(uint8_t i = 0; i < number_of_watchdogs; i++){

        if(handle_array[i].SW_Watchdog_start(&handle_array[i]) != STMLIBS_OK){
            return STMLIBS_ERROR;
        }

        if( HAL_GetTick() > entry_time + timeout){
            return STMLIBS_TIMEOUT;
        }
    }

    return STMLIBS_OK;

}

/**
 * @brief Checks all the watchdogs
 * @return STMLIBS_ERROR if at least one watchdog has expired, STMLIBS_OK otherwise
 */
STMLIBS_StatusTypeDef can_WD_routine(SW_Watchdog_HandleTypedef *handle_array){

    STMLIBS_StatusTypeDef flag = STMLIBS_OK;

    for(uint8_t i = 0; i < number_of_watchdogs; i++){

        if(handle_array[i].SW_Watchdog_check(&handle_array[i]) != STMLIBS_OK){
            flag = STMLIBS_ERROR;
        }

    }

    return flag;

}


/**
 * @defgroup FSM check functions
 */

HAL_StatusTypeDef HVRelays_isairsclosed(Signals_folderTypedef *messages) {
    if ((messages->tlb_bat_status.air_neg_cmd_is_active == 1) && (messages->tlb_bat_status.air_neg_is_closed == 1) && (messages->tlb_bat_status.air_neg_stg_mech_state_signal_is_active == 0) &&
        (messages->tlb_bat_status.dcbus_prech_rly_cmd_is_active == 0) && (messages->tlb_bat_status.dcbus_prech_rly_is_closed == 0) && (messages->tlb_bat_status.air_pos_cmd_is_active == 1) &&
        (messages->tlb_bat_status.air_pos_is_closed == 1) && (messages->tlb_bat_status.air_pos_stg_mech_state_signal_is_active == 0)) {

        return HAL_OK;
    }
    return HAL_ERROR;
}

HAL_StatusTypeDef HVRelays_IsAnyAirOpen(Signals_folderTypedef *messages) {
    if ((messages->tlb_bat_status.air_neg_cmd_is_active == 0) || (messages->tlb_bat_status.air_neg_is_closed == 0) || (messages->tlb_bat_status.air_neg_stg_mech_state_signal_is_active == 1) ||
        (messages->tlb_bat_status.air_pos_cmd_is_active == 0) || (messages->tlb_bat_status.air_pos_is_closed == 0) || (messages->tlb_bat_status.air_pos_stg_mech_state_signal_is_active == 1)) {

        return HAL_OK;
    }
    return HAL_ERROR;
}

HAL_StatusTypeDef HVRelays_IsAnyClosed(Signals_folderTypedef *messages) {
    if ((messages->tlb_bat_status.air_neg_cmd_is_active == 1) || (messages->tlb_bat_status.air_neg_is_closed == 1) || (messages->tlb_bat_status.air_neg_stg_mech_state_signal_is_active == 1) ||
        (messages->tlb_bat_status.dcbus_prech_rly_cmd_is_active == 1) || (messages->tlb_bat_status.dcbus_prech_rly_is_closed == 1) || (messages->tlb_bat_status.air_pos_cmd_is_active == 1) ||
        (messages->tlb_bat_status.air_pos_is_closed == 1) || (messages->tlb_bat_status.air_pos_stg_mech_state_signal_is_active == 1)) {
            
        return HAL_OK;
    }
    return HAL_ERROR;
}

HAL_StatusTypeDef HVRelays_IsAllOpen(Signals_folderTypedef* messages) {
    if ((messages->tlb_bat_status.air_neg_cmd_is_active == 0) && (messages->tlb_bat_status.air_neg_is_closed == 0) && (messages->tlb_bat_status.air_neg_stg_mech_state_signal_is_active == 0) &&
        (messages->tlb_bat_status.dcbus_prech_rly_cmd_is_active == 0) && (messages->tlb_bat_status.dcbus_prech_rly_is_closed == 0) && (messages->tlb_bat_status.air_pos_cmd_is_active == 0) &&
        (messages->tlb_bat_status.air_pos_is_closed == 0) && (messages->tlb_bat_status.air_pos_stg_mech_state_signal_is_active == 0)) {
        return HAL_OK;
    }
    return HAL_ERROR;
}

HAL_StatusTypeDef SDC_isactive(Signals_folderTypedef* messages) {
    if ((messages->tlb_bat_sdc.sdc_tsac_initial_in_is_active == 1) && (messages->tlb_bat_sdc.sdc_tsac_final_in_is_active == 1) &&
        (messages->tlb_bat_sdc.sdc_post_ams_imd_relay_is_active == 1) && 
        #ifdef IMP_EN
        (messages->tlb_bat_status.imp_any_is_active == 0) &&
        #endif
        (messages->tlb_bat_sdc.sdc_tsac_final_in_voltage >= 20)  &&
        (SDC_FUNGO() == SDC_active)) {
            
        return HAL_OK;
    }
    return HAL_ERROR;
}


