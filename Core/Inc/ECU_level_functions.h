#ifndef  ECU_LEVEL_FUNCTIONS_H
#define ECU_LEVEL_FUNCTIONS_H

#include "stdio.h"
#include "can.h" 
#include "stdbool.h"
#include "stmlibs_status.h"
#include "SW_Watchdog_V2.0.h"
#include "can_functions.h"



void CAN_ECU_Tx(CAN_HandleTypeDef *hcan, uint8_t Txdata,uint8_t id,uint32_t TxMailbox );
void display_routine();
void display_routine_0();
void display_routine_1();
void display_routine_2();
void encoder_position_adjustment();
void error_display();
uint32_t MilElapsed(uint8_t reset);
void buzzer_routine();
void IMD_AMS_error_handler();
void TSAC_FAN_routine(double charge_temp);
void can_tx_routine(data_flagTypedef *send_flag);
void start_charge_delay();
void end_charge_delay();
void encoder_IRQ();
HAL_StatusTypeDef MCB_FIFO1_RX_routine(Rx_CAN_Typedef *handle);
HAL_StatusTypeDef HVCB_FIFO1_RX_routine(Rx_CAN_Typedef *handle);
HAL_StatusTypeDef HVCB_FIFO0_RX_routine(Rx_CAN_Typedef *handle);
void ADC_IRQ();
void CAN_error_print(uint32_t error);
void TLB_Battery_SDC_CAN_data_storage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD);
void TLB_Battery_signals_CAN_data_storage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD);
void HV_BMS1_CAN_data_storage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD);
void HV_BMS2_CAN_data_storage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD);
void HV_BMS3_CAN_data_storage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD);
void HV_BMS4_CAN_data_storage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD);
void HV_BMS5_CAN_data_storage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD);
void stop_charge_routine();
void BRUSA_CAN_data_storage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD);
uint8_t charge_control();
void can_WD_set();

void can_WD_init(SW_Watchdog_HandleTypedef *handle_array);

STMLIBS_StatusTypeDef can_WD_start(SW_Watchdog_HandleTypedef *handle_array);

STMLIBS_StatusTypeDef can_WD_start(SW_Watchdog_HandleTypedef *handle_array);

STMLIBS_StatusTypeDef can_WD_routine(SW_Watchdog_HandleTypedef *handle_array);

void can_SW_Watchdog_init(SW_Watchdog_HandleTypedef *handle_array);



#define buffer_TLB_signals 4u
#define buffer_TLB_SDC 1u
#define buffer_BMS_HV_1 0u
#define buffer_BMS_HV_2 2u
#define buffer_BMS_HV_3 3u
#define buffer_BMS_HV_4 5u
#define buffer_BMS_HV_5 6u
#define CHG_EN_REQ 0U
#define Display_Timeout 100


#endif