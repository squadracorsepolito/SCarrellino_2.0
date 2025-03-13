#ifndef CAN_FUNCTIONS_H
#define CAN_FUNCTIONS_H

#include "main.h"
#include "hvcb.h"
#include "mcb.h"
#include "nlg5_database_can.h"





#define v_cell_id           0x0003u
#define tlb_battery_shut_id 0x0004u



 
#if defined(BRUSA_on) || !defined(SOC_evaluation)
    #define can_message_rx_number 9

#else 
    #define can_message_rx_number 7
#endif


#define v_cell_id           0x0003u
#define tlb_battery_shut_id 0x0004u

/**
 * @defgroup Can messages enumeration
 */

#define             HVCB_HVB_RX_V_CELL_                  0U
#define             HVCB_HVB_RX_T_CELL_                  1U
#define             HVCB_HVB_RX_SOC_                     2U
#define             HVCB_HVB_RX_MEASURE_                 3U
#define             MCB_TLB_BAT_SD_CSENSING_STATUS_      4U
#define             MCB_TLB_BAT_SIGNALS_STATUS_          5U
#define             HVCB_HVB_RX_DIAGNOSIS_               6U
#ifdef BRUSA_on
#define             HVCB_BRUSA_                          7U
#endif
#ifndef SOC_evaluation
#define             HVCB_HVB_RX_SOC_                     8U
#endif

//

//#define             HVCB_HVB_RX_V_CELL_signals              5  
//#define             HVCB_HVB_RX_T_CELL_signals              1
////#define             HVCB_HVB_RX_SOC_signals               1      
//#define             HVCB_HVB_RX_MEASURE_signals             1
//#define             MCB_TLB_BAT_SD_CSENSING_STATUS_signals  5
//#define             MCB_TLB_BAT_SIGNALS_STATUS_signals      15
//#define             HVCB_HVB_RX_DIAGNOSIS_signals           12


//uno per ogni can



typedef enum data_flag_struct {

    Flag_Off  = 0U,
    Flag_On = 1U

  } data_flagTypedef;



/**
 * @brief Typedef to handle rx CAN messages
 */
typedef struct rx_struct{

    uint8_t             RxData[8];
    data_flagTypedef    data_flag;

} Rx_CAN_Typedef;

//typedef struct
//{
//    uint32_t id;
//    uint8_t  data[8];
//    bool     Flag_On; 
//
//}can_message;

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
  struct hvcb_hvb_rx_soc_t                SOC;
  struct nlg5_database_can_nlg5_act_i_t   brusa_rx_voltage;

} Signals_folderTypedef;


void can_send_msg(uint32_t id);
HAL_StatusTypeDef can_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header, uint32_t mailbox);
HAL_StatusTypeDef can_wait(CAN_HandleTypeDef *hcan, uint8_t timeout);

void can_rx_routine(Rx_CAN_Typedef *handle_canRx, Signals_folderTypedef *handle_canMessages, SW_Watchdog_HandleTypedef *handle_watchdog);
void can_tx_1();
void can_tx_2();
void can_tx_3();
void can_tx_4();
void can_tx_5();
void can_tx_6();
void can_tx_7();
void AIR_CAN_Cmd_On();
void AIR_CAN_Cmd_Off();
HAL_StatusTypeDef can_messages_init(Signals_folderTypedef *folder);


#endif