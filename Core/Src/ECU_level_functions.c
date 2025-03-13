#include "ECU_level_functions.h"
#include "I2C_LCD.h"
#include "SW_Watchdog_V2.0.h"
#include "adc.h"
#include "can.h"
#include "can_functions.h"
#include "fsm.h"
#include "hvcb.h"
#include "interrupt.h"
#include "main.h"
#include "mcb.h"
#include "ntc.h"
#include "scarrellino_fsm.h"
#include "stdio.h"
#include "string.h"
#include "tim.h"
#include "usart.h"

//flag to know when to send the signal via can to reach the 100Hz frequency
extern volatile data_flagTypedef can_send_flag;

extern volatile uint8_t ntc_temp;
extern volatile uint8_t ADC_conv_flag;

char state[15];
char buff[30];
extern FSM_HandleTypeDef hfsm;

//Struct for the CAN Software Watchdog
SW_Watchdog_HandleTypedef HVCB_HVB_RX_V_CELL_FRAME, HVCB_HVB_RX_T_CELL_FRAME, HVCB_HVB_RX_SOC_FRAME,
    MCB_TLB_BAT_SD_CSENSING_STATUS_FRAME, HVCB_HVB_RX_MEASURE_FRAME, HVCB_HVB_RX_DIAGNOSIS_FRAME, MCB_TLB_BAT_SIGNALS_STATUS_FRAME;

//value of the timer
extern volatile uint16_t counter;

//value of the timer with sign
extern volatile int16_t count;

//real value of the position of the rotary encoder
extern volatile int16_t position;
uint8_t old_position;

extern uint8_t number_of_positions;

uint8_t volatile extern error_code;


uint8_t volatile     hvb_diag_bat_vlt_sna  ,
                     hvb_diag_inv_vlt_sna  ,
                     hvb_diag_bat_curr_sna ,
                     hvb_diag_vcu_can_sna  ,
                     hvb_diag_cell_sna     ,
                     hvb_diag_bat_uv       ,
                     hvb_diag_cell_ov      ,
                     hvb_diag_cell_uv      ,
                     hvb_diag_cell_ot      ,
                     hvb_diag_cell_ut      ,
                     hvb_diag_inv_vlt_ov   ,
                     hvb_diag_bat_curr_oc  ;

/** @brief Display pages manager function */
void display_routine() {

    static uint32_t Display_routine_entry_time = 0;
    static uint32_t Display_routine_exit_time = 0;

    //if timeout occurred in the previously run of the function reinitialize the display
    if (Display_routine_exit_time - Display_routine_entry_time > Display_Timeout){

        I2C_LCD_Init(MyI2C_LCD);
        Display_routine_entry_time = 0;
        Display_routine_exit_time = 0;
        return;
    }

    Display_routine_entry_time = HAL_GetTick();

    //we use the adc conversion to take a frequency to refresh the display
    if (ADC_conv_flag == 0) {

        Display_routine_exit_time = HAL_GetTick();
        return;
    }

    encoder_position_adjustment();

    switch (position) {
        case 0:
            display_routine_0();
            break;

        case 1:
            display_routine_1();
            break;

        case 2:
            display_routine_2();
            break;

        default:
            display_routine_0();
            break;
    }

    Display_routine_exit_time = HAL_GetTick();

    return;
    
}



/** @brief display page: temperature, state of charge */
void display_routine_0() {
    

    double extern charge_temp, SOC;
    bool volatile extern fungo_pressed;


    //temperature extern
    I2C_LCD_Home(I2C_LCD);
    sprintf(buff, "Ext Temp    : %d C ", ntc_temp);
    I2C_LCD_WriteString(I2C_LCD, (char *)&buff);
    ADC_conv_flag = 0;

    //State
    I2C_LCD_ACapo(I2C_LCD);
    switch (hfsm.current_state) {
        case FSM_SCARRELLINO_FSM_IDLE:
            strcpy(state, "IDLE");
            break;

        case FSM_SCARRELLINO_FSM_TSON:
            strcpy(state, "TSON");
            break;

        case FSM_SCARRELLINO_FSM_CHARGE:
            strcpy(state, "CHARGE");
            break;
            
        case FSM_SCARRELLINO_FSM_STOP_CHARGE:
            strcpy(state, "STOP CHARGE");
            break;
        
    }

    sprintf(buff, "State : %s   ", state);
    I2C_LCD_WriteString(I2C_LCD, (char *)&buff);

    // state of charge
    I2C_LCD_ACapo(I2C_LCD);
    sprintf(buff, "HV Batt SOC : %.1f%% ", SOC);

    I2C_LCD_WriteString(I2C_LCD, (char *)&buff);
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

double extern v_max_id_rx;
double extern v_min_id_rx;
double extern v_max_rx;
double extern v_min_rx;
double extern v_mean_rx;

/** @brief display page 1: min e max */
void display_routine_1() {
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

double extern charging_curr;

double extern I_out_rx, V_out_rx, mains_i_rx, mains_v_rx;

/** @brief display page 2  */
void display_routine_2() {
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

/** @brief adjust the encoder position if it goes out of the display pages */
void encoder_position_adjustment() {
    if (position > number_of_positions) {
        __HAL_TIM_SET_COUNTER(&htim3, number_of_positions * 4);
        position = number_of_positions;
    }

    else if (position < 0) {
        __HAL_TIM_SET_COUNTER(&htim3, 0);
        position = 0;
    }

    if (position != old_position){
        I2C_LCD_Clear(I2C_LCD);
    }

    return;
}

//#ifdef __CAN_H__

//funzione che trasmette dati via can
/*per usarla bisogna dargli il puntatore all'header del modulo can che volgiamo usare,
l'array col messaggio da inviare, l'ID che gli vogliamo assegnare e la variabile in cui verrà salvata la mailbox che verrà usata 
*/
/*
void txheader(CAN_TxHeaderTypeDef *TxHeader, uint8_t Txdata, uint8_t id){

    
    TxHeader->DLC = sizeof(Txdata);
    TxHeader->StdId = id;
    TxHeader->IDE = CAN_ID_STD;
    TxHeader->RTR = CAN_RTR_DATA;

}

void CAN_ECU_Tx(CAN_HandleTypeDef *hcan, uint8_t Txdata, uint8_t id, uint32_t TxMailbox ){


CAN_TxHeaderTypeDef TxHeader;

txheader(&TxHeader, Txdata, id);
    
    if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, (uint8_t *)Txdata, &TxMailbox) != HAL_OK){
       
        Error_Handler();
    }}


#endif*/

/** 
 * @brief Function to start a software timer
 */
uint32_t MilElapsed(uint8_t reset) {
    uint32_t static start;

    if (reset == 1)
        start = HAL_GetTick();
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

//void IMD_AMS_error_handler(){
//
//    extern double hvb_diag_imd_low_r ,
//                  hvb_diag_imd_sna   ,
//                  hvb_recovery_active;                   ;
//
//     if((hvb_diag_imd_low_r == 1) | (hvb_diag_imd_sna == 1)){
//       IMD_err_on;
//       WarnLedOn;
//       }
//
//     else{
//       IMD_err_off;
//       WarnLedOff;
//     }
//
//
//     if(hvb_recovery_active == 1)
//     {
//       AMS_err_on;
//       WarnLedOn;
//   }
//
//     else{
//       AMS_err_off;
//       WarnLedOff;
//     }
//     }




uint8_t AMS_detection(
    uint8_t ams_err_is_active,
    uint8_t hvb_diag_bat_vlt_sna,
    uint8_t hvb_diag_inv_vlt_sna,
    uint8_t hvb_diag_bat_curr_sna,
    uint8_t hvb_diag_vcu_can_sna,
    uint8_t hvb_diag_cell_sna,
    uint8_t hvb_diag_bat_uv,
    uint8_t hvb_diag_cell_ov,
    uint8_t hvb_diag_cell_uv,
    uint8_t hvb_diag_cell_ot,
    uint8_t hvb_diag_cell_ut,
    uint8_t hvb_diag_inv_vlt_ov,
    uint8_t hvb_diag_bat_curr_oc
   ){
    static uint8_t ams_err_prev = 0;

    if(ams_err_prev && ams_err_is_active){
        return ams_err_prev;
    }

    if(!ams_err_is_active){ // ams_err_prev & 
        ams_err_prev = 0;
        return ams_err_prev;
    }

ams_err_prev =  (hvb_diag_bat_vlt_sna
    || hvb_diag_inv_vlt_sna
    || hvb_diag_bat_curr_sna
    || hvb_diag_vcu_can_sna
    || hvb_diag_cell_sna
    || hvb_diag_bat_uv
    || hvb_diag_cell_ov
    || hvb_diag_cell_uv
    || hvb_diag_cell_ot
    || hvb_diag_cell_ut
    || hvb_diag_inv_vlt_ov
    || hvb_diag_bat_curr_oc ) & ams_err_is_active;

    return  ams_err_prev;
}


/**
 * @brief     Controls the AMS and IMD status  led
 */
void IMD_AMS_error_handler() {

    extern double imd_err_is_active, ams_err_is_active;

    if (imd_err_is_active == 1) {
        IMD_err_on();
        WarnLedOn();
    }

    else {
        IMD_err_off();
        WarnLedOff();
    }



    if (AMS_detection((uint8_t)ams_err_is_active,
                               hvb_diag_bat_vlt_sna,
                               hvb_diag_inv_vlt_sna,
                               hvb_diag_bat_curr_sna,
                               hvb_diag_vcu_can_sna,
                               hvb_diag_cell_sna,
                               hvb_diag_bat_uv,
                               hvb_diag_cell_ov,
                               hvb_diag_cell_uv,
                               hvb_diag_cell_ot,
                               hvb_diag_cell_ut,
                               hvb_diag_inv_vlt_ov,
                               hvb_diag_bat_curr_oc  ) == 1) {
        AMS_err_on();
        WarnLedOn();
    }

    else {
        AMS_err_off();
        WarnLedOff();
    }
}

/**
 * @brief Controls the speed of the TSAC fans
 */
void TSAC_FAN_routine(double charge_temp) {
    

    if (charge_temp <= 29)
        TSAC_fan_off();

    else if ((charge_temp >= 31) && (charge_temp <= 39)) {
        TSAC_fan_half();
    }

    else if (charge_temp >= 41) {
        TSAC_fan_max();
    }
}



//flag to know which command to send, to close or open the AIRs
extern uint8_t volatile AIR_CAN_Cmd;

/**
 * @brief Manage the CAN TX to close or open the AIRs
 */
void can_tx_routine(data_flagTypedef *send_flag) {
    if (send_flag == Flag_On) {
        #ifdef TEST

        can_tx_1();

        can_tx_2();

        can_tx_3();

        can_tx_4();

        can_tx_5();

        can_tx_6();

        can_tx_7();
        #endif

    switch (AIR_CAN_Cmd) {
        case 0:
            AIR_CAN_Cmd_Off();
            break;

        case 1:
            AIR_CAN_Cmd_On();
            break;
    }

    send_flag = 0;

    }

    return;
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

//value of the timer
uint16_t volatile counter = 0;

//value of the timer with sign
int16_t volatile count = 0;

//real value of the position of the rotary encoder
int16_t volatile position = 0;

uint8_t number_of_positions = 2;

/**
 * @brief IRQ that handles the rotary encoder 
 */
void encoder_IRQ() {
    counter = __HAL_TIM_GetCounter(&htim3);
    count   = (int16_t)counter;

    // every rotation the timer encreases of 4
    position = count / 4;
}

CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];
extern uint8_t volatile can_rx_flag;


/**
 * @brief Function to save the MCB CAN data received in the FIFO1
 */
HAL_StatusTypeDef MCB_FIFO1_RX_routine(Rx_CAN_Typedef *handle) {

    CAN_RxHeaderTypeDef RxHeader = {0};
    uint8_t             RxData[8] = {0};

    if(handle == NULL){
        return HAL_ERROR;
    }
    
    if (HAL_CAN_GetRxMessage(&MCB_CAN_HANDLE, CAN_RX_FIFO1, &RxHeader, (uint8_t *)&RxData) != HAL_OK) {
        //error_code = CAN_Rx_error;
        return HAL_ERROR;

    }


    switch (RxHeader.StdId) {
        
        #ifdef BRUSA_on
        case NLG5_DATABASE_CAN_NLG5_ACT_I_FRAME_ID:

        handle[HVCB_BRUSA_].data_flag = Flag_On;

        for (uint8_t i = 0; i < 8; i++) {
            handle[HVCB_BRUSA_].RxData[i] = RxData[i];
        }
        
            break;

        #endif

        case MCB_TLB_BAT_SIGNALS_STATUS_FRAME_ID:

            handle[MCB_TLB_BAT_SIGNALS_STATUS_].data_flag = Flag_On;

            for (uint8_t i = 0; i < 8; i++) {
                handle[MCB_TLB_BAT_SIGNALS_STATUS_].RxData[i] = RxData[i];
            }

            break;

        case MCB_TLB_BAT_SD_CSENSING_STATUS_FRAME_ID:

        handle[MCB_TLB_BAT_SD_CSENSING_STATUS_].data_flag = Flag_On;

        for (uint8_t i = 0; i < 8; i++) {
            handle[MCB_TLB_BAT_SD_CSENSING_STATUS_].RxData[i] = RxData[i];
        }
            break;
    }

    return HAL_OK;
    
}

/**
 * @brief Function to save the HVCB CAN data received in the FIFO1
 */

HAL_StatusTypeDef HVCB_FIFO1_RX_routine(Rx_CAN_Typedef *handle) {

    CAN_RxHeaderTypeDef RxHeader = {0};
    uint8_t             RxData[8] = {0};

    if(handle == NULL){
        return HAL_ERROR;
    }

    if (HAL_CAN_GetRxMessage(&MCB_CAN_HANDLE, CAN_RX_FIFO1, &RxHeader, (uint8_t *)&RxData) != HAL_OK) {
        //error_code = CAN_Rx_error;
        return HAL_ERROR;

    }
    

    switch (RxHeader.StdId) {

        case HVCB_HVB_RX_T_CELL_FRAME_ID:

            handle[HVCB_HVB_RX_T_CELL_].data_flag = Flag_On;

            for (uint8_t i = 0; i < 8; i++) {
                handle[HVCB_HVB_RX_T_CELL_].RxData[i] = RxData[i];
            }

            break;

        #ifndef SOC_evaluation

        case HVCB_HVB_RX_SOC_FRAME_ID:

            handle[HVCB_HVB_RX_SOC_].data_flag = Flag_On;

            for (uint8_t i = 0; i < 8; i++) {
                handle[HVCB_HVB_RX_SOC_].RxData[i] = RxData[i];
            }

            break

        #endif

        case HVCB_HVB_RX_DIAGNOSIS_FRAME_ID:

            handle[HVCB_HVB_RX_DIAGNOSIS_].data_flag = Flag_On;

            for (uint8_t i = 0; i < 8; i++) {
                handle[HVCB_HVB_RX_DIAGNOSIS_].RxData[i] = RxData[i];
            }



            break;
    }

    return HAL_OK;

}

/**
 * @brief Function to save the HVCB CAN data received in the FIFO0
 */
HAL_StatusTypeDef HVCB_FIFO0_RX_routine(Rx_CAN_Typedef *handle) {

    CAN_RxHeaderTypeDef RxHeader = {0};
    uint8_t             RxData[8] = {0};

    if(handle == NULL){
        return HAL_ERROR;
    }
    
    if (HAL_CAN_GetRxMessage(&MCB_CAN_HANDLE, CAN_RX_FIFO1, &RxHeader, (uint8_t *)&RxData) != HAL_OK) {
        //error_code = CAN_Rx_error;
        return HAL_ERROR;

    }


    switch (RxHeader.StdId) {
        case HVCB_HVB_RX_V_CELL_FRAME_ID:

        handle[HVCB_HVB_RX_V_CELL_].data_flag = Flag_On;

        for (uint8_t i = 0; i < 8; i++) {
            handle[HVCB_HVB_RX_V_CELL_].RxData[i] = RxData[i];
        }

            break;

        case HVCB_HVB_RX_MEASURE_FRAME_ID:

        handle[HVCB_HVB_RX_MEASURE_].data_flag = Flag_On;

        for (uint8_t i = 0; i < 8; i++) {
            handle[HVCB_HVB_RX_MEASURE_].RxData[i] = RxData[i];
        }

            break;

    }

    return HAL_OK;
    
}

extern volatile uint16_t ntc_value;
uint8_t flag_ntc = 0;

/**
 * @brief IRQ of the ADC to handle the NTC
 */
void ADC_IRQ() {
    flag_ntc = 1;
    ntc_temp = get_ntc_temperature(ntc_value);
    HAL_GPIO_TogglePin(STAT1_LED_GPIO_OUT_GPIO_Port, STAT1_LED_GPIO_OUT_Pin);
    ADC_conv_flag = 1;
}


#define uart_print(X)                                                        \
    do {                                                                     \
        HAL_UART_Transmit(&LOG_UART, (uint8_t *)X "\n", strlen(X "\n"), 10); \
    } while (0)

void CAN_error_print(uint32_t error) {
    error_code = CAN_generic_error;

    if (error & HAL_CAN_ERROR_EWG)
        uart_print("Protocol Error Warning\n\r");
    if (error & HAL_CAN_ERROR_EPV)
        uart_print("Error Passive\n\r");
    if (error & HAL_CAN_ERROR_BOF)
        uart_print("Bus-off Error\n\r");
    if (error & HAL_CAN_ERROR_STF)
        uart_print("Stuff Error\n\r");
    if (error & HAL_CAN_ERROR_FOR)
        uart_print("Form Error\n\r");
    if (error & HAL_CAN_ERROR_ACK)
        uart_print("ACK Error\n\r");
    if (error & HAL_CAN_ERROR_BR)
        uart_print("Bit Recessive Error\n\r");
    if (error & HAL_CAN_ERROR_BD)
        uart_print("Bit Dominant Error\n\r");
    if (error & HAL_CAN_ERROR_CRC)
        uart_print("CRC Error\n\r");
    if (error & HAL_CAN_ERROR_RX_FOV0)
        uart_print("FIFO0 Overrun\n\r");
    if (error & HAL_CAN_ERROR_RX_FOV1)
        uart_print("FIFO1 Overrun\n\r");
    if (error & HAL_CAN_ERROR_TX_ALST0)
        uart_print("Mailbox 0 TX failure (arbitration lost)\n\r");
    if (error & HAL_CAN_ERROR_TX_TERR0)
        uart_print("Mailbox 0 TX failure (tx error)\n\r");
    if (error & HAL_CAN_ERROR_TX_ALST1)
        uart_print("Mailbox 1 TX failure (arbitration lost)\n\r");
    if (error & HAL_CAN_ERROR_TX_TERR1)
        uart_print("Mailbox 1 TX failure (tx error)\n\r");
    if (error & HAL_CAN_ERROR_TX_ALST2)
        uart_print("Mailbox 2 TX failure (arbitration lost)\n\r");
    if (error & HAL_CAN_ERROR_TX_TERR2)
        uart_print("Mailbox 2 TX failure (tx error)\n\r");
    if (error & HAL_CAN_ERROR_TIMEOUT)
        uart_print("Timeout Error\n\r");
    if (error & HAL_CAN_ERROR_NOT_INITIALIZED)
        uart_print("Peripheral not initialized\n\r");
    if (error & HAL_CAN_ERROR_NOT_READY)
        uart_print("Peripheral not ready\n\r");
    if (error & HAL_CAN_ERROR_NOT_STARTED)
        uart_print("Peripheral not strated\n\r");
    if (error & HAL_CAN_ERROR_PARAM)
        uart_print("Parameter Error\n\r");
}


double extern SOC;

void TLB_Battery_SDC_CAN_data_storage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD) {

    if((handle_canRx == NULL) || (handle_canMessages == NULL) || (hWD == NULL)){
        return;
    }

    if (handle_canRx->data_flag == Flag_Off) {
        return;
    }

    #ifdef Watchdog
    hWD->SW_Watchdog_refresh(hWD);
    #endif
            
    handle_canRx->data_flag = Flag_Off;

    mcb_tlb_bat_sd_csensing_status_init(&handle_canMessages->tlb_bat_sdc);
    mcb_tlb_bat_sd_csensing_status_unpack(&handle_canMessages->tlb_bat_sdc, (uint8_t *)&handle_canRx, MCB_TLB_BAT_SD_CSENSING_STATUS_LENGTH);

    handle_canMessages->tlb_bat_sdc.sdc_tsac_initial_in_is_active    = mcb_tlb_bat_sd_csensing_status_sdc_tsac_initial_in_is_active_decode(handle_canMessages->tlb_bat_sdc.sdc_tsac_initial_in_is_active);
    handle_canMessages->tlb_bat_sdc.sdc_post_ams_imd_relay_is_active = mcb_tlb_bat_sd_csensing_status_sdc_post_ams_imd_relay_is_active_decode(handle_canMessages->tlb_bat_sdc.sdc_post_ams_imd_relay_is_active);
    handle_canMessages->tlb_bat_sdc.sdc_tsac_final_in_is_active      = mcb_tlb_bat_sd_csensing_status_sdc_tsac_final_in_is_active_decode(handle_canMessages->tlb_bat_sdc.sdc_tsac_final_in_is_active);
    handle_canMessages->tlb_bat_sdc.sdc_prech_bypass_relay_is_closed   = mcb_tlb_bat_sd_csensing_status_sdc_prech_bypass_relay_is_closed_decode(handle_canMessages->tlb_bat_sdc.sdc_prech_bypass_relay_is_closed);
    handle_canMessages->tlb_bat_sdc.sdc_tsac_initial_in_voltage             = mcb_tlb_bat_sd_csensing_status_sdc_tsac_final_in_voltage_decode(handle_canMessages->tlb_bat_sdc.sdc_tsac_initial_in_voltage);
    

    return;
}

void TLB_Battery_signals_CAN_data_storage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD) {
    

    if((handle_canRx == NULL) || (handle_canMessages == NULL) || (hWD == NULL)){
        return;
    }

    if (handle_canRx->data_flag == Flag_Off) {
        return;
    }

    #ifdef Watchdog
    hWD->SW_Watchdog_refresh(hWD);
    #endif
            
    handle_canRx->data_flag = Flag_Off;


    mcb_tlb_bat_signals_status_init(&handle_canMessages->tlb_bat_status);
    mcb_tlb_bat_signals_status_unpack(&handle_canMessages->tlb_bat_status, (uint8_t *)&handle_canRx->RxData, MCB_TLB_BAT_SIGNALS_STATUS_LENGTH);

    handle_canMessages->tlb_bat_status.air_neg_cmd_is_active                   = mcb_tlb_bat_signals_status_air_neg_cmd_is_active_decode(handle_canMessages->tlb_bat_status.air_neg_cmd_is_active);
    handle_canMessages->tlb_bat_status.air_neg_is_closed                       = mcb_tlb_bat_signals_status_air_neg_is_closed_decode(handle_canMessages->tlb_bat_status.air_neg_is_closed);
    handle_canMessages->tlb_bat_status.air_neg_stg_mech_state_signal_is_active = mcb_tlb_bat_signals_status_air_neg_stg_mech_state_signal_is_active_decode(handle_canMessages->tlb_bat_status.air_neg_stg_mech_state_signal_is_active);
    handle_canMessages->tlb_bat_status.air_pos_cmd_is_active                   = mcb_tlb_bat_signals_status_air_pos_cmd_is_active_decode(handle_canMessages->tlb_bat_status.air_pos_cmd_is_active);
    handle_canMessages->tlb_bat_status.air_pos_is_closed                       = mcb_tlb_bat_signals_status_air_pos_is_closed_decode(handle_canMessages->tlb_bat_status.air_pos_is_closed);
    handle_canMessages->tlb_bat_status.air_pos_stg_mech_state_signal_is_active = mcb_tlb_bat_signals_status_air_pos_stg_mech_state_signal_is_active_decode(handle_canMessages->tlb_bat_status.air_pos_stg_mech_state_signal_is_active);
    handle_canMessages->tlb_bat_status.ams_err_is_active                       = mcb_tlb_bat_signals_status_ams_err_is_active_decode(handle_canMessages->tlb_bat_status.ams_err_is_active);
    handle_canMessages->tlb_bat_status.dcbus_is_over60_v                       = mcb_tlb_bat_signals_status_dcbus_is_over60_v_decode(handle_canMessages->tlb_bat_status.dcbus_is_over60_v);
    handle_canMessages->tlb_bat_status.dcbus_prech_rly_cmd_is_active           = mcb_tlb_bat_signals_status_dcbus_prech_rly_cmd_is_active_decode(handle_canMessages->tlb_bat_status.dcbus_prech_rly_cmd_is_active);
    handle_canMessages->tlb_bat_status.dcbus_prech_rly_is_closed               = mcb_tlb_bat_signals_status_dcbus_prech_rly_is_closed_decode(handle_canMessages->tlb_bat_status.dcbus_prech_rly_is_closed);
    handle_canMessages->tlb_bat_status.imd_err_is_active                       = mcb_tlb_bat_signals_status_imd_err_is_active_decode(handle_canMessages->tlb_bat_status.imd_err_is_active);
    handle_canMessages->tlb_bat_status.imp_dcbus_is_active                     = mcb_tlb_bat_signals_status_imp_dcbus_is_active_decode(handle_canMessages->tlb_bat_status.imp_dcbus_is_active);
    handle_canMessages->tlb_bat_status.imp_any_is_active                       = mcb_tlb_bat_signals_status_imp_any_is_active_decode(handle_canMessages->tlb_bat_status.imp_any_is_active);
    handle_canMessages->tlb_bat_status.imp_hv_relays_state_is_active           = mcb_tlb_bat_signals_status_imp_hv_relays_state_is_active_decode(handle_canMessages->tlb_bat_status.imp_hv_relays_state_is_active);
    handle_canMessages->tlb_bat_status.tsal_green_is_active                    = mcb_tlb_bat_signals_status_tsal_green_is_active_decode(handle_canMessages->tlb_bat_status.tsal_green_is_active);

    return;
 
}

void HV_BMS1_CAN_data_storage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD) {
    
    
    if((handle_canRx == NULL) || (handle_canMessages == NULL) || (hWD == NULL)){
        return;
    }

    if (handle_canRx->data_flag == Flag_Off) {
        return;
    }

    #ifdef Watchdog
    hWD->SW_Watchdog_refresh(hWD);
    #endif

    handle_canRx->data_flag = Flag_Off;

    hvcb_hvb_rx_v_cell_init(&handle_canMessages->v_cells);
    hvcb_hvb_rx_v_cell_unpack(&handle_canMessages->v_cells, (uint8_t *)&handle_canRx->RxData, HVCB_HVB_RX_V_CELL_LENGTH);

    handle_canMessages->v_cells.hvb_idx_cell_u_max  = hvcb_hvb_rx_v_cell_hvb_idx_cell_u_max_decode(handle_canMessages->v_cells.hvb_idx_cell_u_max);
    handle_canMessages->v_cells.hvb_idx_cell_u_min  = hvcb_hvb_rx_v_cell_hvb_idx_cell_u_min_decode(handle_canMessages->v_cells.hvb_idx_cell_u_min);
    handle_canMessages->v_cells.hvb_u_cell_max      = hvcb_hvb_rx_v_cell_hvb_u_cell_max_decode(handle_canMessages->v_cells.hvb_u_cell_max);
    handle_canMessages->v_cells.hvb_u_cell_min      = hvcb_hvb_rx_v_cell_hvb_u_cell_min_decode(handle_canMessages->v_cells.hvb_u_cell_min);
    handle_canMessages->v_cells.hvb_u_cell_mean     = hvcb_hvb_rx_v_cell_hvb_u_cell_mean_decode(handle_canMessages->v_cells.hvb_u_cell_mean);

    return;

    //sprintf(buffer, "v max id = %.0f \n\rv min id = %.0f \n\rv max = %.2f \n\rv min = %.2f \n\rv mean = %.2f \n\r", v_max_id_rx, v_min_id_rx, v_max_rx, v_min_rx, v_mean_rx);
    //HAL_UART_Transmit(&LOG_UART, (uint8_t*) &buffer, strlen(buffer), 200);
    
}

void HV_BMS2_CAN_data_storage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD) {
    if((handle_canRx == NULL) || (handle_canMessages == NULL) || (hWD == NULL)){
        return;
    }

    if (handle_canRx->data_flag == Flag_Off) {
        return;
    }

    #ifdef Watchdog
    hWD->SW_Watchdog_refresh(hWD);
    #endif

    handle_canRx->data_flag = Flag_Off;

    hvcb_hvb_rx_t_cell_init(&handle_canMessages->t_cells);
    hvcb_hvb_rx_t_cell_unpack(&handle_canMessages->t_cells, (uint8_t *)&handle_canRx->RxData, HVCB_HVB_RX_T_CELL_LENGTH);

    handle_canMessages->t_cells.hvb_t_cell_max = hvcb_hvb_rx_t_cell_hvb_t_cell_max_decode(handle_canMessages->t_cells.hvb_t_cell_max);

    //sprintf(buffer, "charging temp = %.2lf\n\r", charge_temp);
    // HAL_UART_Transmit(&LOG_UART, (uint8_t*) &buffer, strlen(buffer), 200);

    return;
    
}

#ifndef SOC_evaluation

void HV_BMS3_CAN_data_storage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD) {
    if((handle_canRx == NULL) || (handle_canMessages == NULL) || (hWD == NULL)){
        return;
    }

    if (handle_canRx->data_flag == Flag_Off) {
        return;
    }

    #ifdef Watchdog
    hWD->SW_Watchdog_refresh(&hWD);
    #endif

    handle_canRx->data_flag = Flag_Off;

    hvcb_hvb_rx_soc_init(&handle_canMessages->SOC);
    hvcb_hvb_rx_soc_unpack(&handle_canMessages->SOC, (uint8_t *)&handle_canRx->RxData, HVCB_HVB_RX_SOC_LENGTH);

    handle_canMessages->SOC.hvb_r_so_c_hvb_u_cell_min = hvcb_hvb_rx_soc_hvb_r_so_c_hvb_u_cell_min_decode(handle_canMessages->SOC.hvb_r_so_c_hvb_u_cell_min);

    return;
    //  sprintf(buffer, "SOC = %.2lf\n\r", (SOC*0.04 + 50));
    //  HAL_UART_Transmit(&LOG_UART, (uint8_t*) &buffer, strlen(buffer), 200);
    
}
#endif

#ifdef BRUSA_on



void BRUSA_CAN_data_storage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD) {

    if((handle_canRx == NULL) || (handle_canMessages == NULL) || (hWD == NULL)){
        return;
    }

    if (handle_canRx->data_flag == Flag_Off) {
        return;
    }

    #ifdef Watchdog
    hWD->SW_Watchdog_refresh(hWD);
    #endif

    handle_canRx->data_flag = Flag_Off;

    memset(&handle_canMessages->brusa_rx_voltage, 0, sizeof(struct nlg5_database_can_nlg5_act_i_t));

    nlg5_database_can_nlg5_act_i_unpack(
        &handle_canMessages->brusa_rx_voltage, (uint8_t *)&handle_canRx->RxData, NLG5_DATABASE_CAN_NLG5_ACT_I_LENGTH);

    handle_canMessages->brusa_rx_voltage.nlg5_mc_act   = nlg5_database_can_nlg5_act_i_nlg5_mc_act_decode(handle_canMessages->brusa_rx_voltage.nlg5_mc_act);
    handle_canMessages->brusa_rx_voltage.nlg5_mv_act   = nlg5_database_can_nlg5_act_i_nlg5_mv_act_decode(handle_canMessages->brusa_rx_voltage.nlg5_mv_act);
    handle_canMessages->brusa_rx_voltage.nlg5_ov_act   = nlg5_database_can_nlg5_act_i_nlg5_ov_act_decode(handle_canMessages->brusa_rx_voltage.nlg5_ov_act);
    handle_canMessages->brusa_rx_voltage.nlg5_oc_act   = nlg5_database_can_nlg5_act_i_nlg5_oc_act_decode(handle_canMessages->brusa_rx_voltage.nlg5_oc_act);

    // sprintf(buffer, "V = %.2lf\n\rI = %.2lf\n\r", V_out_rx, I_out_rx);
    // HAL_UART_Transmit(&LOG_UART, (uint8_t*) &buffer, strlen(buffer), 200);
    return;
}

#endif

void stop_charge_routine() {
    ChargeENcmdOFF();
    TSAC_fan_off();
    ChargeBlueLedOff();
    buzzer_stop_charge_on = 1;
    AIR_CAN_Cmd           = 0;
}

void HV_BMS4_CAN_data_storage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD) {
    
    if((handle_canRx == NULL) || (handle_canMessages == NULL) || (hWD == NULL)){
        return;
    }

    if (handle_canRx->data_flag == Flag_Off) {
        return;
    }

    #ifdef Watchdog
    hWD->SW_Watchdog_refresh(hWD);
    #endif

    handle_canRx->data_flag = Flag_Off;

    hvcb_hvb_rx_measure_init(&handle_canMessages->measures);
    hvcb_hvb_rx_measure_unpack(&handle_canMessages->measures, (uint8_t *)&handle_canRx->RxData, HVCB_HVB_RX_MEASURE_LENGTH);

    handle_canMessages->measures.hvb_i_hvb = hvcb_hvb_rx_measure_hvb_i_hvb_decode(handle_canMessages->measures.hvb_i_hvb);

    return;

}

void HV_BMS5_CAN_data_storage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD) {
    
    if((handle_canRx == NULL) || (handle_canMessages == NULL) || (hWD == NULL)){
        return;
    }

    if (handle_canRx->data_flag == Flag_Off) {
        return;
    }

    #ifdef Watchdog
    hWD->SW_Watchdog_refresh(hWD);
    #endif

    handle_canRx->data_flag = Flag_Off;

    hvcb_hvb_rx_diagnosis_init(&handle_canMessages->diagnosis);
    hvcb_hvb_rx_diagnosis_unpack(
        &handle_canMessages->diagnosis, (uint8_t *)&handle_canRx->RxData, HVCB_HVB_RX_DIAGNOSIS_LENGTH);

    handle_canMessages->diagnosis.hvb_diag_bat_vlt_sna      = hvcb_hvb_rx_diagnosis_hvb_diag_bat_vlt_sna_decode(  handle_canMessages->diagnosis.hvb_diag_bat_vlt_sna  );
    handle_canMessages->diagnosis.hvb_diag_inv_vlt_sna      = hvcb_hvb_rx_diagnosis_hvb_diag_inv_vlt_sna_decode(  handle_canMessages->diagnosis.hvb_diag_inv_vlt_sna  );
    handle_canMessages->diagnosis.hvb_diag_bat_curr_sna     = hvcb_hvb_rx_diagnosis_hvb_diag_bat_curr_sna_decode( handle_canMessages->diagnosis.hvb_diag_bat_curr_sna  );
    handle_canMessages->diagnosis.hvb_diag_vcu_can_sna      = hvcb_hvb_rx_diagnosis_hvb_diag_vcu_can_sna_decode(  handle_canMessages->diagnosis.hvb_diag_vcu_can_sna  );
    handle_canMessages->diagnosis.hvb_diag_cell_sna         = hvcb_hvb_rx_diagnosis_hvb_diag_cell_sna_decode(     handle_canMessages->diagnosis.hvb_diag_cell_sna    );
    handle_canMessages->diagnosis.hvb_diag_bat_uv           = hvcb_hvb_rx_diagnosis_hvb_diag_bat_uv_decode(       handle_canMessages->diagnosis.hvb_diag_bat_uv      );
    handle_canMessages->diagnosis.hvb_diag_cell_ov          = hvcb_hvb_rx_diagnosis_hvb_diag_cell_ov_decode(      handle_canMessages->diagnosis.hvb_diag_cell_ov     );
    handle_canMessages->diagnosis.hvb_diag_cell_uv          = hvcb_hvb_rx_diagnosis_hvb_diag_cell_uv_decode(      handle_canMessages->diagnosis.hvb_diag_cell_uv     );
    handle_canMessages->diagnosis.hvb_diag_cell_ot          = hvcb_hvb_rx_diagnosis_hvb_diag_cell_ot_decode(      handle_canMessages->diagnosis.hvb_diag_cell_ot     );
    handle_canMessages->diagnosis.hvb_diag_cell_ut          = hvcb_hvb_rx_diagnosis_hvb_diag_cell_ut_decode(      handle_canMessages->diagnosis.hvb_diag_cell_ut     );
    handle_canMessages->diagnosis.hvb_diag_inv_vlt_ov       = hvcb_hvb_rx_diagnosis_hvb_diag_inv_vlt_ov_decode(   handle_canMessages->diagnosis.hvb_diag_inv_vlt_ov  );
    handle_canMessages->diagnosis.hvb_diag_bat_curr_oc      = hvcb_hvb_rx_diagnosis_hvb_diag_bat_curr_oc_decode(  handle_canMessages->diagnosis.hvb_diag_bat_curr_oc  );

    return;
          
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
    handle_array[HVCB_HVB_RX_SOC_]                = hWD_HVCB_HVB_RX_SOC;    
    handle_array[HVCB_HVB_RX_MEASURE_]            = hWD_HVCB_HVB_RX_MEASURE;
    handle_array[MCB_TLB_BAT_SD_CSENSING_STATUS_] = hWD_MCB_TLB_BAT_SD_CSENSING_STATUS;
    handle_array[MCB_TLB_BAT_SIGNALS_STATUS_]     = hWD_MCB_TLB_BAT_SIGNALS_STATUS;
    handle_array[HVCB_HVB_RX_DIAGNOSIS_]          = hWD_HVCB_HVB_RX_DIAGNOSIS;

    //WD values assignment 
    SW_Watchdog_init(&handle_array[HVCB_HVB_RX_V_CELL_],             HVCB_HVB_RX_V_CELL_CYCLE_TIME_MS*5);
    SW_Watchdog_init(&handle_array[HVCB_HVB_RX_T_CELL_],             HVCB_HVB_RX_T_CELL_CYCLE_TIME_MS*5);
    SW_Watchdog_init(&handle_array[HVCB_HVB_RX_SOC_],                HVCB_HVB_RX_SOC_CYCLE_TIME_MS*5);
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

    for(uint8_t i = 0; i<(number_of_watchdogs-1); i++){

        handle_array[i].SW_Watchdog_start(&handle_array[i]);

        if( HAL_GetTick() > entry_time + timeout){
            return STMLIBS_TIMEOUT;
        }
    }

    return STMLIBS_OK;

}

/**
 * @brief checks all the watchdogs
 * @return STMLIBS_ERROR if at least one watchdog has expired, STMLIBS_OK otherwise
 */
STMLIBS_StatusTypeDef can_WD_routine(SW_Watchdog_HandleTypedef *handle_array){

    STMLIBS_StatusTypeDef flag = STMLIBS_OK;

    for(uint8_t i = 0; i<(number_of_watchdogs-1); i++){

        if(handle_array[i].SW_Watchdog_check(&handle_array[i]) != STMLIBS_OK){
            flag = STMLIBS_ERROR;
        }

    }

    return flag;

}
