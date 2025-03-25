#ifndef MAIN_ROUTINE_H
#define MAIN_ROUTINE_H

#include "main.h"

#include "main_routine.h"
#include "scarrellino_fsm.h"
#include "ECU_level_functions.h"
#include "tim.h"
#include "I2C_LCD.h"
#include "display.h"
#include "ntc.h"

/**
 * @brief CAN polling functions container
 */
void can_routine();

/**
 * @brief Libraries initialization
 */
void System_init();

/**
 * @brief Charge routines container
 */
void charge_control_routine();

/**
 * @brief Human-Machine Interface routine
 */
void HMI_routine();

/**
 * @brief Watchdog check routine
 */
void watchdog_routine();


/**
 * @brief Errors handle array
 */
extern HAL_StatusTypeDef           herror[number_of_errors];

/**
 * @brief CAN messages and signals handle
 */
extern Signals_folderTypedef       hmessages;

/**
 * @brief RX CAN messages handle array
 */
extern volatile Rx_CAN_Typedef     hcan_rx[can_message_rx_number];

/**
 * @brief Watchdog handle array
 */
extern SW_Watchdog_HandleTypedef   hWD[number_of_watchdogs];


/**
 * @brief Flag to know when to send the signal via can to reach the 100Hz frequency
 */
extern volatile data_flagTypedef   can_send_flag = Flag_Off;



#endif