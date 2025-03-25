/*
CAN high level functions
*/

#include "can_functions.h"

HAL_StatusTypeDef can_wait(CAN_HandleTypeDef *hcan, uint8_t timeout) {
    uint32_t tick = HAL_GetTick();
    while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0) {
        if (HAL_GetTick() - tick > timeout)
            return HAL_TIMEOUT;
    }
    return HAL_OK;
}

HAL_StatusTypeDef can_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header, uint32_t mailbox) {
    if (can_wait(hcan, 1) != HAL_OK)
        return HAL_TIMEOUT;

    volatile HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(hcan, header, buffer, &mailbox);

    return status;
}

HAL_StatusTypeDef can_messages_init(Signals_folderTypedef *folder){

    memset(folder, 0, sizeof(Signals_folderTypedef));

    return HAL_OK;
}



CAN_TxHeaderTypeDef TxHeader;

uint8_t buffer_tx[8];


//flag to know when a message is received in CAN
bool volatile can_rx_flag = 0;

extern CAN_RxHeaderTypeDef RxHeader;

double extern v_max_id_rx, v_min_id_rx, v_max_rx, v_min_rx, v_mean_rx;

double mains_v_rx, mains_i_rx, I_out_rx, V_out_rx;

double charge_temp;

double SOC = 0;

double charging_curr = 0;

double hvb_diag_imd_low_r = 1, hvb_recovery_active = 1, hvb_diag_imd_sna = 1;

extern can_message can_buffer[can_message_rx_number];

#ifdef BRUSA_on
extern can_message can_buffer_brusa;
#endif

char buffer[1200] = {0};

volatile extern uint8_t can_id;


void can_rx_routine(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *handle_watchdog) {
    

    #ifdef BRUSA_on
    BRUSA_DataStorage(&handle_canRx[HVCB_BRUSA_], &handle_canMessages, &hWD[HVCB_BRUSA_]); 
    #endif

    Cell_volt_DataStorage(&handle_canRx[HVCB_HVB_RX_V_CELL_], &handle_canMessages, &handle_watchdog[HVCB_HVB_RX_V_CELL_]);
    Cell_temp_DataStorage(&handle_canRx[HVCB_HVB_RX_T_CELL_], &handle_canMessages, &handle_watchdog[HVCB_HVB_RX_T_CELL_]);
    TSAC_curr_DataStorage(&handle_canRx[HVCB_HVB_RX_MEASURE_], &handle_canMessages, &handle_watchdog[HVCB_HVB_RX_MEASURE_]);
    PODIUM_BMS_DataStorage(&handle_canRx[HVCB_HVB_RX_DIAGNOSIS_], &handle_canMessages, &handle_watchdog[HVCB_HVB_RX_DIAGNOSIS_]);
    TLB_Batt_DataStorage(&handle_canRx[MCB_TLB_BAT_SIGNALS_STATUS_], &handle_canMessages, &handle_watchdog[MCB_TLB_BAT_SIGNALS_STATUS_]);
    SDC_DataStorage(&handle_canRx[MCB_TLB_BAT_SD_CSENSING_STATUS_], &handle_canMessages, &handle_watchdog[MCB_TLB_BAT_SD_CSENSING_STATUS_]);

    return;
    
}

HAL_StatusTypeDef    AIR_CAN_Cmd_send(data_flagTypedef CAN_cmd) {
    struct hvcb_hvb_tx_vcu_cmd_t static AIR_Cmd;

    hvcb_hvb_tx_vcu_cmd_init(&AIR_Cmd);

    AIR_Cmd.vcu_b_hvb_inv_req = hvcb_hvb_tx_vcu_cmd_vcu_b_hvb_inv_req_encode((double)CAN_cmd);
    AIR_Cmd.vcu_clr_err       = hvcb_hvb_tx_vcu_cmd_vcu_clr_err_encode(0);
    AIR_Cmd.vcu_b_bal_req     = hvcb_hvb_tx_vcu_cmd_vcu_b_bal_req_encode((double)CAN_Cmd);
    AIR_Cmd.vcu_b_all_vt_req  = hvcb_hvb_tx_vcu_cmd_vcu_b_all_vt_req_encode(1);

    hvcb_hvb_tx_vcu_cmd_pack((uint8_t *)&buffer_tx, &AIR_Cmd, HVCB_HVB_TX_VCU_CMD_LENGTH);

    TxHeader.DLC   = HVCB_HVB_TX_VCU_CMD_LENGTH;
    TxHeader.StdId = HVCB_HVB_TX_VCU_CMD_FRAME_ID;

    if (can_send(&HVCB_CAN_HANDLE, (uint8_t *)&buffer_tx, &TxHeader, CAN_TX_MAILBOX0) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef can_tx_routine(data_flagTypedef *send_flag) {

    extern uint8_t volatile AIR_CAN_Cmd;
    
    if(send_flag == NULL){
        return HAL_ERROR;
    }

    if (send_flag == Flag_Off) {

        return HAL_OK;
    }

    if (AIR_CAN_Cmd_send(AIR_CAN_Cmd) != HAL_OK){
        return HAL_ERROR;
    }

    send_flag = 0;

    return HAL_OK;
}

HAL_StatusTypeDef SDC_DataStorage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD) {

    if((handle_canRx == NULL) || (handle_canMessages == NULL) || (hWD == NULL)){
        return HAL_ERROR;
    }

    if (handle_canRx->data_flag == Flag_Off) {
        return HAL_OK;
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
    

    return HAL_OK;
}

HAL_StatusTypeDef TLB_Batt_DataStorage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD) {
    

    if((handle_canRx == NULL) || (handle_canMessages == NULL) || (hWD == NULL)){
        return HAL_ERROR; 
    }

    if (handle_canRx->data_flag == Flag_Off) {
        return HAL_OK;
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

    return HAL_OK; 
 
}

HAL_StatusTypeDef Cell_volt_DataStorage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD) {
    
    
    if((handle_canRx == NULL) || (handle_canMessages == NULL) || (hWD == NULL)){
        return HAL_ERROR;
    }

    if (handle_canRx->data_flag == Flag_Off) {
        return HAL_OK;
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

    return HAL_OK;

    //sprintf(buffer, "v max id = %.0f \n\rv min id = %.0f \n\rv max = %.2f \n\rv min = %.2f \n\rv mean = %.2f \n\r", v_max_id_rx, v_min_id_rx, v_max_rx, v_min_rx, v_mean_rx);
    //HAL_UART_Transmit(&LOG_UART, (uint8_t*) &buffer, strlen(buffer), 200);
    
}

HAL_StatusTypeDef Cell_temp_DataStorage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD) {
    if((handle_canRx == NULL) || (handle_canMessages == NULL) || (hWD == NULL)){
        return HAL_ERROR;
    }

    if (handle_canRx->data_flag == Flag_Off) {
        return HAL_OK;
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

    return HAL_OK;
    
}





HAL_StatusTypeDef BRUSA_DataStorage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD) {

    if((handle_canRx == NULL) || (handle_canMessages == NULL) || (hWD == NULL)){
        return HAL_ERROR;
    }

    if (handle_canRx->data_flag == Flag_Off) {
        return HAL_OK;  
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
    return HAL_OK;
}

HAL_StatusTypeDef TSAC_curr_DataStorage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD) {
    
    if((handle_canRx == NULL) || (handle_canMessages == NULL) || (hWD == NULL)){
        return HAL_ERROR;
    }

    if (handle_canRx->data_flag == Flag_Off) {
        return HAL_OK;
    }

    #ifdef Watchdog
    hWD->SW_Watchdog_refresh(hWD);
    #endif

    handle_canRx->data_flag = Flag_Off;

    hvcb_hvb_rx_measure_init(&handle_canMessages->measures);
    hvcb_hvb_rx_measure_unpack(&handle_canMessages->measures, (uint8_t *)&handle_canRx->RxData, HVCB_HVB_RX_MEASURE_LENGTH);

    handle_canMessages->measures.hvb_i_hvb = hvcb_hvb_rx_measure_hvb_i_hvb_decode(handle_canMessages->measures.hvb_i_hvb);

    return HAL_OK;

}

HAL_StatusTypeDef PODIUM_BMS_DataStorage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD) {
    
    if((handle_canRx == NULL) || (handle_canMessages == NULL) || (hWD == NULL)){
        return HAL_ERROR;
    }

    if (handle_canRx->data_flag == Flag_Off) {
        return HAL_OK;
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

    return HAL_OK;
          
}


void IMD_AMS_error_handler(Signals_folderTypedef *handle) {

    if (handle->tlb_bat_status.imd_err_is_active == IMD_ON) {
        IMD_err_on();
        WarnLedOn();
    }

    else {
        IMD_err_off();
        WarnLedOff();
    }


    if (AMS_detection(handle) == HAL_ERROR) {
        AMS_err_on();
        WarnLedOn();
    }

    else {
        AMS_err_off();
        WarnLedOff();
    }
}


HAL_StatusTypeDef AMS_detection(Signals_folderTypedef *handle){

    static HAL_StatusTypeDef ams_err_prev = HAL_OK;

    if(ams_err_prev && handle->tlb_bat_status.ams_err_is_active){
        return ams_err_prev;
    }

    if(!handle->tlb_bat_status.ams_err_is_active){ // ams_err_prev & 
        ams_err_prev = HAL_OK;
        return ams_err_prev;
    }
    
    ams_err_prev =  (HAL_StatusTypeDef) ((handle->diagnosis.hvb_diag_bat_vlt_sna
                                         || handle->diagnosis.hvb_diag_inv_vlt_sna
                                         || handle->diagnosis.hvb_diag_bat_curr_sna
                                         || handle->diagnosis.hvb_diag_vcu_can_sna
                                         || handle->diagnosis.hvb_diag_cell_sna
                                         || handle->diagnosis.hvb_diag_bat_uv
                                         || handle->diagnosis.hvb_diag_cell_ov
                                         || handle->diagnosis.hvb_diag_cell_uv
                                         || handle->diagnosis.hvb_diag_cell_ot
                                         || handle->diagnosis.hvb_diag_cell_ut
                                         || handle->diagnosis.hvb_diag_inv_vlt_ov
                                         || handle->diagnosis.hvb_diag_bat_curr_oc ) && handle->tlb_bat_status.ams_err_is_active);
                             
    return  ams_err_prev;
}



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
