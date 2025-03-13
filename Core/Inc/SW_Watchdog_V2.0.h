#ifndef  SW_WATCHDOG_V2_H
#define  SW_WATCHDOG_V2_H


#include "main.h"
#include "stmlibs_status.h"
#include "stdio.h"

#define number_of_watchdogs 7U

#pragma once
/**
 * @param timeout_true timeout has occurred
 * @param timeout_false timeout has not occurred
 */
typedef enum timeout_struct {
    timeout_true  = 1U,
    timeout_false = 0U
} Timeout_Typedef;

typedef struct SW_Watchdog SW_Watchdog_HandleTypedef;

typedef STMLIBS_StatusTypeDef(*SW_Watchdog_function)(SW_Watchdog_HandleTypedef *handle);



/**
 * @brief Struct to store the data about the Watchdog, time in millisecond
 */
struct SW_Watchdog
{

    Timeout_Typedef  timeout;
    uint32_t         watchdog_time;
    uint32_t         last_refresh_time;

    SW_Watchdog_function SW_Watchdog_check;
    SW_Watchdog_function SW_Watchdog_start;
    SW_Watchdog_function SW_Watchdog_refresh;

};


/**
 * @brief watchdog time assignment
 */
STMLIBS_StatusTypeDef SW_Watchdog_init(SW_Watchdog_HandleTypedef *handle, uint32_t watchdog_time);

#endif