#ifndef CAN_FUNCTIONS_H
#define CAN_FUNCTIONS_H

#include "main.h"

#include "hvcb.h"
#include "mcb.h"
#include "nlg5_database_can.h"
#include "SW_Watchdog_V2.0.h"
#include "ECU_level_functions.h"
#include "can.h"
 
#ifdef BRUSA_on
   
  #define can_message_rx_number 7

#else 

  #define can_message_rx_number 6
#endif


/**
 * @defgroup Can messages enumeration
 */

#define             HVCB_HVB_RX_V_CELL_                  0U
#define             HVCB_HVB_RX_T_CELL_                  1U
#define             HVCB_HVB_RX_MEASURE_                 2U
#define             MCB_TLB_BAT_SD_CSENSING_STATUS_      3U
#define             MCB_TLB_BAT_SIGNALS_STATUS_          4U
#define             HVCB_HVB_RX_DIAGNOSIS_               5U
#ifdef BRUSA_on
#define             HVCB_BRUSA_                          6U
#endif


//

//#define             HVCB_HVB_RX_V_CELL_signals              5  
//#define             HVCB_HVB_RX_T_CELL_signals              1
//#define             HVCB_HVB_RX_SOC_signals                 1      
//#define             HVCB_HVB_RX_MEASURE_signals             1
//#define             MCB_TLB_BAT_SD_CSENSING_STATUS_signals  5
//#define             MCB_TLB_BAT_SIGNALS_STATUS_signals      15
//#define             HVCB_HVB_RX_DIAGNOSIS_signals           12

//uno per ogni can

/**
 * @brief Flag to indicate that IMD error is active
 */
#define IMD_ON 1

#define MCB_CAN_HANDLE hcan2
#define HVCB_CAN_HANDLE hcan1





/**
 * @brief flag type
 */
typedef enum data_flag_struct {

  Flag_Off  = 0U,
  Flag_On   = 1U

} data_flagTypedef;

/**
 * @brief RX CAN messages handle array
 */
typedef struct rx_struct{

  uint8_t             RxData[8];
  data_flagTypedef    data_flag;

} Rx_CAN_Typedef;


/**
 * @brief CAN messages and signals handle
 */
typedef struct signals_folder_struct{

  struct mcb_tlb_bat_signals_status_t     tlb_bat_status;
  struct mcb_tlb_bat_sd_csensing_status_t tlb_bat_sdc;
  struct hvcb_hvb_rx_t_cell_t             t_cells;
  struct hvcb_hvb_rx_measure_t            measures;
  struct hvcb_hvb_rx_diagnosis_t          diagnosis;
  struct hvcb_hvb_rx_v_cell_t             v_cells;
  struct nlg5_database_can_nlg5_act_i_t   brusa_rx_voltage;

  double SOC;

} Signals_folderTypedef;





void can_send_msg(uint32_t id);

/**
 * @brief Send messages on the CAN bus
 */
HAL_StatusTypeDef can_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header, uint32_t mailbox);

/**
 * @brief Wait for the CAN Tx to be ready
 */
HAL_StatusTypeDef can_wait(CAN_HandleTypeDef *hcan, uint8_t timeout);

/**
 * @brief  CAN RX polling function
 * @note from TLB BATTERY we receive signals about imd error, ams error, sdc open state, sdc precharge relay state
 * @note from the PODIUM HV BMS info about the voltages of the hv battery pack cells 
 * @note from the BRUSA CHARGER info about charging and mains voltage and current
 */
void can_rx_routine(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *handle_watchdog);

/**
 * @brief Commands the activation/disactivation of the AIRs
 */
HAL_StatusTypeDef AIR_CAN_Cmd_send(data_flagTypedef CAN_cmd);

HAL_StatusTypeDef can_messages_init(Signals_folderTypedef *folder);

/**
 * @brief Manage the CAN TX to close or open the AIRs
 */
HAL_StatusTypeDef can_tx_routine(data_flagTypedef *send_flag);

/**
 * @brief Light up the LEDs in case of IMD and AMS errors
 */
void IMD_AMS_error_handler(Signals_folderTypedef *handle);

/**
 * @brief Function to latch the AMS error
 * @return HAL_OK for no AMS error, HAL_ERROR for AMS error
 */
HAL_StatusTypeDef AMS_detection(Signals_folderTypedef *handle);


/**
 * @defgroup CAN_Storage 
 * @brief CAN data storage functions
 */

HAL_StatusTypeDef SDC_DataStorage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD);
HAL_StatusTypeDef TLB_Batt_DataStorage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD);
HAL_StatusTypeDef Cell_volt_DataStorage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD);
HAL_StatusTypeDef Cell_temp_DataStorage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD);
HAL_StatusTypeDef TSAC_curr_DataStorage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD);
HAL_StatusTypeDef PODIUM_BMS_DataStorage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD);
HAL_StatusTypeDef BRUSA_DataStorage(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *hWD);
/**
 * 
 */

/**
 * @defgroup CAN RX routine
 */

HAL_StatusTypeDef MCB_FIFO1_RX_routine(Rx_CAN_Typedef *handle);
HAL_StatusTypeDef HVCB_FIFO1_RX_routine(Rx_CAN_Typedef *handle);
HAL_StatusTypeDef HVCB_FIFO0_RX_routine(Rx_CAN_Typedef *handle);
/**
 * 
 */
void CAN_error_print(uint32_t error);
#endif