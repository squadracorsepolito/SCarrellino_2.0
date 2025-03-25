#ifndef  ECU_LEVEL_FUNCTIONS_H
#define ECU_LEVEL_FUNCTIONS_H

#include "main.h"

#include "ECU_level_functions.h"
#include "I2C_LCD.h"
#include "SW_Watchdog_V2.0.h"
#include "adc.h"
#include "can.h"
#include "can_functions.h"
#include "fsm.h"
#include "hvcb.h"
#include "mcb.h"
#include "ntc.h"
#include "scarrellino_fsm.h"
#include "stdio.h"
#include "string.h"
#include "tim.h"
#include "usart.h"



void CAN_ECU_Tx(CAN_HandleTypeDef *hcan, uint8_t Txdata,uint8_t id,uint32_t TxMailbox );

void encoder_position_adjustment(TIM_HandleTypeDef *htim);
void error_display();
uint32_t MilElapsed(uint8_t reset);
void buzzer_routine();

/**
 * @brief Command the fan power to cool the battery pack
 */
void TSAC_FAN_routine(uint16_t cell_temp);
void start_charge_delay();
void end_charge_delay();


void stop_charge_routine();
HAL_StatusTypeDef BRUSA_DataStorage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD);
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


#endif