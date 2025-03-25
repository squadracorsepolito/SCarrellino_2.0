/*
 * "THE BEER-WARE LICENSE" (Revision 69):
 * Squadra Corse firmware team wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy us a beer in return.
 *
 * Authors
 * - Filippo Rossi <filippo.rossi.sc@gmail.com>
 * - Federico Carbone <federico.carbone.sc@gmail.com>
 */
#include "scarrellino_fsm.h"

#ifndef __weak
#define __weak __attribute__((weak))
#endif  // __weak


/**
 * @brief Flag to know which command to send, to close or open the AIRs
 */
data_flagTypedef volatile AIR_CAN_Cmd = 0;

bool volatile ChargeEN_risingedge = 0;
bool volatile ChargeEN_fallingedge = 0;

//flag to register the fungo pression
bool volatile fungo_pressed = 0;

void run_callback_1(uint32_t state) {
    ((void)0U);
}

void transition_callback_1(uint32_t state) {
    ((void)0U);
}

STMLIBS_StatusTypeDef _FSM_init(FSM_HandleTypeDef *hfsm){


    uint8_t n_events = 0U;

    if (FSM_SCARRELLINO_FSM_init(&hfsm, n_events, run_callback_1, transition_callback_1) != STMLIBS_OK) {
        return STMLIBS_ERROR;
    }
    if (FSM_start(&hfsm) != STMLIBS_OK) {
        return STMLIBS_ERROR;
    }

    return STMLIBS_OK;

}



// States global variables
uint32_t fsm_tson_entry_time        = (-1U);
uint32_t fsm_charge_entry_time      = (-1U);
uint32_t fsm_stop_charge_entry_time = (-1U);



//buzzer flags
bool volatile buzzer_charge_on, buzzer_stop_charge_on;


extern volatile uint16_t ADC_value;



// Private wrapper function signatures
uint32_t _FSM_SCARRELLINO_FSM_IDLE_event_handle(uint8_t event);
uint32_t _FSM_SCARRELLINO_FSM_IDLE_do_work();

uint32_t _FSM_SCARRELLINO_FSM_TSON_event_handle(uint8_t event);
uint32_t _FSM_SCARRELLINO_FSM_TSON_do_work();

uint32_t _FSM_SCARRELLINO_FSM_CHARGE_event_handle(uint8_t event);
uint32_t _FSM_SCARRELLINO_FSM_CHARGE_do_work();

uint32_t _FSM_SCARRELLINO_FSM_STOP_CHARGE_event_handle(uint8_t event);
uint32_t _FSM_SCARRELLINO_FSM_STOP_CHARGE_do_work();

FSM_StateTypeDef state_table[_FSM_SCARRELLINO_FSM_STATE_COUNT] = {
    [FSM_SCARRELLINO_FSM_IDLE] =
        {
            .event_handler = _FSM_SCARRELLINO_FSM_IDLE_event_handle,
            .entry         = FSM_SCARRELLINO_FSM_IDLE_entry,
            .do_work       = _FSM_SCARRELLINO_FSM_IDLE_do_work,
            .exit          = FSM_SCARRELLINO_FSM_IDLE_exit,
        },
    [FSM_SCARRELLINO_FSM_TSON] =
        {
            .event_handler = _FSM_SCARRELLINO_FSM_TSON_event_handle,
            .entry         = FSM_SCARRELLINO_FSM_TSON_entry,
            .do_work       = _FSM_SCARRELLINO_FSM_TSON_do_work,
            .exit          = FSM_SCARRELLINO_FSM_TSON_exit,
        },
    [FSM_SCARRELLINO_FSM_CHARGE] =
        {
            .event_handler = _FSM_SCARRELLINO_FSM_CHARGE_event_handle,
            .entry         = FSM_SCARRELLINO_FSM_CHARGE_entry,
            .do_work       = _FSM_SCARRELLINO_FSM_CHARGE_do_work,
            .exit          = FSM_SCARRELLINO_FSM_CHARGE_exit,
        },
    [FSM_SCARRELLINO_FSM_STOP_CHARGE] = {
        .event_handler = _FSM_SCARRELLINO_FSM_STOP_CHARGE_event_handle,
        .entry         = FSM_SCARRELLINO_FSM_STOP_CHARGE_entry,
        .do_work       = _FSM_SCARRELLINO_FSM_STOP_CHARGE_do_work,
        .exit          = FSM_SCARRELLINO_FSM_STOP_CHARGE_exit,
    }};

FSM_ConfigTypeDef config = {
    .state_length = _FSM_SCARRELLINO_FSM_STATE_COUNT,
    .state_table  = state_table,
};

STMLIBS_StatusTypeDef FSM_SCARRELLINO_FSM_init(FSM_HandleTypeDef *handle,
                                               uint8_t event_count,
                                               FSM_callback_function run_callback,
                                               FSM_callback_function transition_callback) {
    return FSM_init(handle, &config, event_count, run_callback, transition_callback);
}




// State control functions

/** @brief wrapper of FSM_SCARRELLINO_FSM_event_handle, with exit state checking */
uint32_t _FSM_SCARRELLINO_FSM_IDLE_event_handle(uint8_t event) {
    uint32_t next = (uint32_t)FSM_SCARRELLINO_FSM_IDLE_event_handle(event);

    switch (next) {
        case FSM_SCARRELLINO_FSM_IDLE:
        case FSM_SCARRELLINO_FSM_TSON:
            return next;
        default:
            return _FSM_SCARRELLINO_FSM_DIE;
    }
}
    static  int_state_variable_Typedef variables;    

FSM_SCARRELLINO_FSM_StateTypeDef FSM_SCARRELLINO_FSM_IDLE_do_work(){

    // #############################################     Saves all the variables in a struct to stay consistent during the state
    extern Signals_folderTypedef hmessages;
    static Signals_folderTypedef Variables_Saving = {0};

    Variables_Saving = hmessages;

    // ################################################## OUTPUTS for this state
    // by default in TSON keep the airs OPEN
    AIR_CAN_Cmd = 0;


    // don't enable charge led by default
    ChargeBlueLedOff();

    // dont' charge by default
    ChargeENcmdOFF();

    // ###################################### TRNASITION FUNCTION for this state
    // Set default state if nothing changes
    uint32_t next = FSM_SCARRELLINO_FSM_IDLE;

    if((IDLE_checks(&Variables_Saving) == HAL_OK) && (ChargeEN_risingedge == 1))

    
        if (SDC_isactive(&variables) && (variables.ams_err_is_active == 0) && (variables.imd_err_is_active == 0) && (variables.tsal_green_is_active == 1) && 
            HVRelays_IsAllOpen(&variables) && (variables.dcbus_is_over60_v == 0) &&
    #ifdef IMP_EN
            (variables.imp_dcbus_is_active == 0) && (variables.imp_hv_relays_signals_is_active == 0) && (variables.imp_any_is_active == 0) &&
    #endif
    #ifdef TEMP_CHECK_EN
            (variables.charge_temp < 60) &&
    #endif
            ChargeEN_risingedge == 1)
    
        {

        next = FSM_SCARRELLINO_FSM_TSON;
        }
    

    // check for implausibile transition
    switch (next) {
        case FSM_SCARRELLINO_FSM_IDLE:
        case FSM_SCARRELLINO_FSM_TSON:
            return next;
        default:
            return _FSM_SCARRELLINO_FSM_DIE;
    }
    }



/** @brief wrapper of FSM_SCARRELLINO_FSM_event_handle, with exit state checking */
uint32_t _FSM_SCARRELLINO_FSM_TSON_event_handle(uint8_t event) {
    uint32_t next = (uint32_t)FSM_SCARRELLINO_FSM_TSON_event_handle(event);

    switch (next) {
        case FSM_SCARRELLINO_FSM_TSON:
        case FSM_SCARRELLINO_FSM_IDLE:
            return next;
        default:
            return _FSM_SCARRELLINO_FSM_DIE;
    }
}


void FSM_SCARRELLINO_FSM_TSON_entry() {
    fsm_tson_entry_time = HAL_GetTick();
}


FSM_SCARRELLINO_FSM_StateTypeDef FSM_SCARRELLINO_FSM_TSON_do_work() {
    // #############################################     Saves all the variables in a struct to stay consistent during the state

    VariableSaving(&variables);

    // ################################################## OUTPUTS for this state
    // by default in TSON keep the airs closed
    #ifdef air
    AIR_CAN_Cmd = 1;
    #else
    AIR_CAN_Cmd = 0;

    #endif


    // don't enable charge led by default
    ChargeBlueLedOff();
    // dont' charge by default
    ChargeENcmdOFF();

    // ###################################### TRNASITION FUNCTION for this state
    // Set default state if nothing changes
    uint32_t next = FSM_SCARRELLINO_FSM_TSON;

    if (SDC_isactive(&variables) &&                                        // SDC active
        (variables.ams_err_is_active == 0) && (variables.imd_err_is_active == 0) &&  // NO AMS/IMD errors
        //(variables.tsal_green_is_active == 0) &&                           // TSAL not green (some HV RELAY is closed)
        HVRelays_isairsclosed(&variables) &&   // only AIR POS/NEG are closed (DCBUS PRECHARGE completed)
        (variables.dcbus_is_over60_v == 1) &&  // We see correctly the DCBUS over 60V
#ifdef IMP_EN
        // No implausibilities active
        (variables.imp_dcbus_is_active == 0) && (variables.imp_hv_relays_signals_is_active == 0) && (variables.imp_any_is_active == 0) &&
#endif
#ifdef TEMP_CHECK_EN
        (variables.charge_temp < 60) &&
#endif
        // if We are still requesting to charge
        (ChargeEN_risingedge == 1)) {
        // Go to CHARGE
        next = FSM_SCARRELLINO_FSM_CHARGE;
    }
    // If we found AMS/IMD errors or the SDC opens or some implausibility
    else if ((variables.ams_err_is_active == 1) || (variables.imd_err_is_active == 1) || !SDC_isactive(&variables) ||
#ifdef IMP_EN
             (variables.imp_dcbus_is_active == 1) || (variables.imp_hv_relays_signals_is_active == 1) || (variables.imp_any_is_active == 1) ||
#endif
             (ChargeEN_risingedge == 0)) {
        next = FSM_SCARRELLINO_FSM_IDLE;
    }
    // If we are timing out the TS ON procedure (5sec)
    else if (HAL_GetTick() >= (fsm_tson_entry_time + 5000U)) {
        next = FSM_SCARRELLINO_FSM_IDLE;
      // go back to idle
        }

    switch (next) {
            case FSM_SCARRELLINO_FSM_TSON:
            case FSM_SCARRELLINO_FSM_IDLE:
            case FSM_SCARRELLINO_FSM_CHARGE:
                return next;
            default:
                return _FSM_SCARRELLINO_FSM_DIE;
    }
    }


/** @brief wrapper of FSM_SCARRELLINO_FSM_event_handle, with exit state checking */
uint32_t _FSM_SCARRELLINO_FSM_CHARGE_event_handle(uint8_t event) {
    uint32_t next = (uint32_t)FSM_SCARRELLINO_FSM_CHARGE_event_handle(event);

    switch (next) {
        case FSM_SCARRELLINO_FSM_CHARGE:
        case FSM_SCARRELLINO_FSM_STOP_CHARGE:
            return next;
        default:
            return _FSM_SCARRELLINO_FSM_DIE;
    }
}

void FSM_SCARRELLINO_FSM_CHARGE_entry() {
    fsm_charge_entry_time = HAL_GetTick();
    buzzer_charge_on = 1;

}


/** @brief wrapper of FSM_SCARRELLINO_FSM_do_work, with exit state checking */
FSM_SCARRELLINO_FSM_StateTypeDef FSM_SCARRELLINO_FSM_CHARGE_do_work() {
    // #############################################     Saves all the variables in a struct to stay consistent during the state

    VariableSaving(&variables);

    // ################################################## OUTPUTS for this state

    // by default in TSON keep the airs closed
    #ifdef air
    AIR_CAN_Cmd = 1;
    #else
    AIR_CAN_Cmd = 0;
    #endif    
    

    // SIgnal we are in charge state by toggling led
    ChargeBlueLedOn();

    // Wait 1 second before starting to charge
    if (HAL_GetTick() >= (fsm_charge_entry_time + 1000U)) {
        // Enable brusa charging
        ChargeENcmdON();
        // Signal start charge with sound
    } 
    else {
        // dont' charge by default
        ChargeENcmdOFF();
    }

    // ###################################### TRNASITION FUNCTION for this state
    // Set default state if nothing changes
    uint32_t next = FSM_SCARRELLINO_FSM_CHARGE;

    if ((variables.ams_err_is_active == 1) || (variables.imd_err_is_active == 1) ||  // If we found AMS/IMD errors
        !SDC_isactive(&variables) ||                                                 // or the SDC opens
#ifdef IMP_EN
        (variables.imp_dcbus_is_active == 1) || (variables.imp_hv_relays_signals_is_active == 1) ||
        (variables.imp_any_is_active == 0) ||       // or implausibility
#endif
        HVRelays_IsAnyAirOpen(&variables) ||        // The AIRs somehow opened
        (variables.charge_temp > 60)      ||        //the charging temp goes over 60°C 
        ChargeEN_fallingedge == 1 || 
        (ChargeEN() == !CHG_EN_REQ)                 // or the charge is disabled via switch
    ) {
        next = FSM_SCARRELLINO_FSM_STOP_CHARGE;
    }

    //if after a second past the starting of the charge the current is minor of 0.5 Amps (stop the charge when the battery is fully charged)
    if ((HAL_GetTick() >= (fsm_charge_entry_time + 2000U)) && (variables.charging_curr > -0.5)){
        
        next = FSM_SCARRELLINO_FSM_STOP_CHARGE;
    }

    switch (next) {
        case FSM_SCARRELLINO_FSM_CHARGE:
        case FSM_SCARRELLINO_FSM_STOP_CHARGE:
            return next;
        default:
            return _FSM_SCARRELLINO_FSM_DIE;
    }
}



/** @brief wrapper of FSM_SCARRELLINO_FSM_event_handle, with exit state checking */
uint32_t _FSM_SCARRELLINO_FSM_STOP_CHARGE_event_handle(uint8_t event) {
    uint32_t next = (uint32_t)FSM_SCARRELLINO_FSM_STOP_CHARGE_event_handle(event);

    switch (next) {
        case FSM_SCARRELLINO_FSM_STOP_CHARGE:  // Reentrance is always supported on event handlers
        case FSM_SCARRELLINO_FSM_CHARGE:
            return next;
        default:
            return _FSM_SCARRELLINO_FSM_DIE;
    }
}

void FSM_SCARRELLINO_FSM_STOP_CHARGE_entry() {
    fsm_stop_charge_entry_time = HAL_GetTick();
}

FSM_SCARRELLINO_FSM_StateTypeDef FSM_SCARRELLINO_FSM_STOP_CHARGE_do_work() {
    // #############################################     Saves all the variables in a struct to stay consistent during the state

    VariableSaving(&variables);

    // ################################################## OUTPUTS for this state

    // keep airs closed
    #ifdef air
    AIR_CAN_Cmd = 1;
    #else
    AIR_CAN_Cmd = 0;
    #endif

    buzzer_stop_charge_on  = 1;

    // disable led
    ChargeBlueLedOff();

    // dont' charge
    ChargeENcmdOFF();

    // ###################################### TRNASITION FUNCTION for this state
    // Set default state if nothing changes
    uint32_t next = FSM_SCARRELLINO_FSM_STOP_CHARGE;

    // when no more current flows or we timeout
    if (variables.charging_curr > -0.5 || HAL_GetTick() > fsm_stop_charge_entry_time + 500U) {
        next = FSM_SCARRELLINO_FSM_IDLE;
    }

    switch (next) {
        case FSM_SCARRELLINO_FSM_IDLE:
        case FSM_SCARRELLINO_FSM_STOP_CHARGE:
            return next;
        default:
            return _FSM_SCARRELLINO_FSM_DIE;
    }
}


void FSM_SCARRELLINO_FSM_STOP_CHARGE_exit() {
    buzzer_stop_charge_on = 1;
}

