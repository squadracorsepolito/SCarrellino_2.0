#include "main.h" 
#include "SW_Watchdog_V2.0.h"
#include "string.h"
#include "stdio.h"

#ifndef __weak
#define __weak __attribute__((weak))
#endif  // __weak



/**
 * @brief Checks whether the watchdog has expired
 */
static STMLIBS_StatusTypeDef SW_Watchdog_check(SW_Watchdog_HandleTypedef *handle);

/**
 * @brief Start the watchdog
 * @param *handle pointer to the watchdog handle
 */
static STMLIBS_StatusTypeDef SW_Watchdog_start(SW_Watchdog_HandleTypedef *handle);

/**
 * @brief Refresh the watchdog
 */
static STMLIBS_StatusTypeDef SW_Watchdog_refresh(SW_Watchdog_HandleTypedef *handle);



static STMLIBS_StatusTypeDef SW_Watchdog_check(SW_Watchdog_HandleTypedef *handle){

    uint32_t time = HAL_GetTick();

    if(handle == NULL){
        return STMLIBS_ERROR;
    }

    if(time - handle->last_refresh_time >= handle->watchdog_time){
        handle->timeout = timeout_true;
        return STMLIBS_ERROR;
    }

    return STMLIBS_OK;

}

static STMLIBS_StatusTypeDef SW_Watchdog_start(SW_Watchdog_HandleTypedef *handle){

    uint32_t time = HAL_GetTick();

    if(handle == NULL){
        return STMLIBS_ERROR;
    }

    if(handle->last_refresh_time == 0){
        handle->last_refresh_time = time;
        return STMLIBS_OK;
    }

    return STMLIBS_ERROR;

}


static STMLIBS_StatusTypeDef SW_Watchdog_refresh(SW_Watchdog_HandleTypedef *handle){
    
    uint32_t time = HAL_GetTick();

    if(handle == NULL){
        return STMLIBS_ERROR;
    }

    if(handle->last_refresh_time != 0){
        handle->last_refresh_time = time;
        return STMLIBS_OK;
    }

    return STMLIBS_ERROR;


}


STMLIBS_StatusTypeDef SW_Watchdog_init(SW_Watchdog_HandleTypedef *handle, uint32_t watchdog_time){

    if(handle == NULL){
        return STMLIBS_ERROR;
    }

    if (watchdog_time <= 0U){
        return STMLIBS_ERROR;
    }   
    
    handle->last_refresh_time = 0U;
    handle->timeout = 0U;

    handle->SW_Watchdog_check = SW_Watchdog_check;
    handle->SW_Watchdog_refresh = SW_Watchdog_refresh;
    handle->SW_Watchdog_start = SW_Watchdog_start;

    handle->watchdog_time = watchdog_time;

    return STMLIBS_OK;

}