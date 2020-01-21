/**
 * app_witch.c 
 * Copyright (C) 2019 blueseidon

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include "app_switch.h"

#include "sdk_config.h"
#include "example_common.h"
#include "generic_onoff_server.h"

#include "log.h"
#include "app_timer.h"

/** This sample implementation shows how the model behavior requirements of Generic OnOff server can
 * be implemented.
 */

/* Forward declaration */
static void generic_onoff_state_get_cb(const generic_onoff_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       generic_onoff_status_params_t * p_out);
static void generic_onoff_state_set_cb(const generic_onoff_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       const generic_onoff_set_params_t * p_in,
                                       const model_transition_t * p_in_transition,
                                       generic_onoff_status_params_t * p_out);

const generic_onoff_server_callbacks_t onoff_srv_cbs =
{
    .onoff_cbs.set_cb = generic_onoff_state_set_cb,
    .onoff_cbs.get_cb = generic_onoff_state_get_cb
};

static void switch_state_process_timing(app_switch_server_t * p_server)
{
    uint32_t status = NRF_SUCCESS;

    (void) app_timer_stop(*p_server->p_timer_id);

    /* Process timing requirements */
    if (p_server->state.delay_ms != 0)
    {
        status = app_timer_start(*p_server->p_timer_id, APP_TIMER_TICKS(p_server->state.delay_ms), p_server);
    }
    else if (p_server->state.remaining_time_ms != 0)
    {
        /* Note: We cannot use the full length of the app_timer, since RTC counter is 24 bit, and
        application needs to report the remaining time whenever GET message is received in the
        middle of the transition. Correctness of the reported value is limited to 100 ms at the
        highest resolution as defined in section 3.1.3 of Mesh Model Specification v1.0 */
        uint32_t app_timer_ticks = APP_TIMER_TICKS(p_server->state.remaining_time_ms);
        if (app_timer_ticks > APP_TIMER_MAX_CNT_VAL)
        {
            status = app_timer_start(*p_server->p_timer_id, APP_TIMER_MAX_CNT_VAL, p_server);
        }
        else if (app_timer_ticks >= APP_TIMER_MIN_TIMEOUT_TICKS)
        {
            status = app_timer_start(*p_server->p_timer_id, APP_TIMER_TICKS(p_server->state.remaining_time_ms), p_server);
        }
        else
        {
            status = app_timer_start(*p_server->p_timer_id, APP_TIMER_MIN_TIMEOUT_TICKS, p_server);
        }
        p_server->last_rtc_counter = app_timer_cnt_get();
    }

    if (status != NRF_SUCCESS)
    {
        __LOG(LOG_SRC_APP, LOG_LEVEL_ERROR, "State transition timer error\n");
    }
}

static void switch_state_value_update(app_switch_server_t * p_server)
{
    /* Requirement: If delay and transition time is zero, current state changes to the target state. */
    if ((p_server->state.delay_ms == 0 && p_server->state.remaining_time_ms == 0) ||
    /* Requirement: If current state is 0 (checked earlier) and target state is 1, current state value changes
     * to the target state value immediately after the delay.
     */
        (p_server->state.delay_ms == 0 && p_server->state.target_switch == 1))
    {
        p_server->state.present_switch = p_server->state.target_switch;

        generic_onoff_status_params_t status_params;
        status_params.present_on_off = p_server->state.present_switch;
        status_params.target_on_off = p_server->state.target_switch;
        status_params.remaining_time_ms = p_server->state.remaining_time_ms;
        (void) generic_onoff_server_status_publish(&p_server->server, &status_params);

        if (!p_server->value_updated)
        {
            p_server->switch_set_cb(p_server, p_server->state.present_switch);
            p_server->value_updated = true;
        }
    }

    __LOG(LOG_SRC_APP, LOG_LEVEL_DBG1, "cur switch: %d  target: %d  delay: %d ms  remaining time: %d ms\n",
          p_server->state.present_switch, p_server->state.target_switch, p_server->state.delay_ms, p_server->state.remaining_time_ms);
}

static void switch_state_timer_cb(void * p_context)
{
    app_switch_server_t * p_server = (app_switch_server_t *) p_context;

    /* Requirement: Process timing. Process the delay first (Non-zero delay will delay the required
     * state transition by the specified amount) and then the transition time.
     */
    if (p_server->state.delay_ms != 0)
    {
        p_server->state.delay_ms = 0;
        switch_state_value_update(p_server);
    }
    else if (p_server->state.remaining_time_ms != 0)
    {
        if (APP_TIMER_TICKS(p_server->state.remaining_time_ms) > APP_TIMER_MAX_CNT_VAL)
        {
            p_server->state.remaining_time_ms -= (APP_TIMER_MAX_CNT_VAL/APP_TIMER_CLOCK_FREQ);
        }
        else
        {
            p_server->state.remaining_time_ms = 0;
            switch_state_value_update(p_server);
        }
    }
    switch_state_process_timing(p_server);
}


/***** Generic OnOff model interface callbacks *****/
static void generic_onoff_state_get_cb(const generic_onoff_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       generic_onoff_status_params_t * p_out)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "msg: GET \n");

    app_switch_server_t   * p_server = PARENT_BY_FIELD_GET(app_switch_server_t, server, p_self);

    /* Requirement: Provide the current value of the OnOff state */
    p_server->switch_get_cb(p_server, &p_server->state.present_switch);
    p_out->present_on_off = p_server->state.present_switch;
    p_out->target_on_off = p_server->state.target_switch;

    /* Requirement: Always report remaining time */
    if (p_server->state.remaining_time_ms > 0 && p_server->state.delay_ms == 0)
    {
        uint32_t delta = (1000ul * app_timer_cnt_diff_compute(app_timer_cnt_get(), p_server->last_rtc_counter)) / APP_TIMER_CLOCK_FREQ;
        if (p_server->state.remaining_time_ms >= delta && delta > 0)
        {
            p_out->remaining_time_ms = p_server->state.remaining_time_ms - delta;
        }
        else
        {
            p_out->remaining_time_ms = 0;
        }
    }
    else
    {
        p_out->remaining_time_ms = p_server->state.remaining_time_ms;
    }
}

static void generic_onoff_state_set_cb(const generic_onoff_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       const generic_onoff_set_params_t * p_in,
                                       const model_transition_t * p_in_transition,
                                       generic_onoff_status_params_t * p_out)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "msg: SET: %d\n", p_in->on_off);

    app_switch_server_t   * p_server = PARENT_BY_FIELD_GET(app_switch_server_t, server, p_self);

    /* Update internal representation of OnOff value, process timing */
    p_server->value_updated = false;
    p_server->state.target_switch = p_in->on_off;
    if (p_in_transition == NULL)
    {
        p_server->state.delay_ms = 0;
        p_server->state.remaining_time_ms = 0;
    }
    else
    {
        p_server->state.delay_ms = p_in_transition->delay_ms;
        p_server->state.remaining_time_ms = p_in_transition->transition_time_ms;
    }

    switch_state_value_update(p_server);
    switch_state_process_timing(p_server);

    /* Prepare response */
    if (p_out != NULL)
    {
        p_out->present_on_off = p_server->state.present_switch;
        p_out->target_on_off = p_server->state.target_switch;
        p_out->remaining_time_ms = p_server->state.remaining_time_ms;
    }
}

/***** Interface functions *****/

void app_switch_status_publish(app_switch_server_t * p_server)
{
    p_server->switch_get_cb(p_server, &p_server->state.present_switch);

    p_server->state.target_switch = p_server->state.present_switch;
    p_server->state.delay_ms = 0;
    p_server->state.remaining_time_ms = 0;
    (void) app_timer_stop(*p_server->p_timer_id);

    generic_onoff_status_params_t status = {
                .present_on_off = p_server->state.present_switch,
                .target_on_off = p_server->state.target_switch,
                .remaining_time_ms = p_server->state.remaining_time_ms
            };
    (void) generic_onoff_server_status_publish(&p_server->server, &status);
}

uint32_t app_switch_init(app_switch_server_t * p_server, uint8_t element_index)
{
    uint32_t status = NRF_ERROR_INTERNAL;

    if (p_server == NULL)
    {
        return NRF_ERROR_NULL;
    }

    p_server->server.settings.p_callbacks = &onoff_srv_cbs;
    if (p_server->switch_set_cb == NULL || p_server->switch_get_cb == NULL)
    {
        return NRF_ERROR_NULL;
    }

    status = generic_onoff_server_init(&p_server->server, element_index);
    if (status == NRF_SUCCESS)
    {
        status = app_timer_create(p_server->p_timer_id, APP_TIMER_MODE_SINGLE_SHOT,
                                  switch_state_timer_cb);
    }

    return status;
}