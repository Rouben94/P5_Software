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

#include "app_switch_client.h"

#include <stdint.h>

#include "sdk_config.h"
#include "example_common.h"
#include "generic_onoff_client.h"

#include "log.h"
#include "app_timer.h"

/** This sample implementation shows how the model behavior requirements of Generic OnOff server can
 * be implemented.
 */

/* Forward declaration */
static void genric_onoff_client_publish_interval_cb(access_model_handle_t handle, 
                                                    void *p_self);

static void generic_onoff_client_status_cb(const generic_onoff_client_t *p_self,
                                           const access_message_rx_meta_t *p_meta,
                                           const generic_onoff_status_params_t *p_in);

static void app_gen_onoff_client_transaction_status_cb(access_model_handle_t model_handle,
                                                       void *p_args,
                                                       access_reliable_status_t status);

const generic_onoff_client_callbacks_t client_cbs =
{
    .onoff_status_cb = app_generic_onoff_client_status_cb,
    .ack_transaction_status_cb = app_gen_onoff_client_transaction_status_cb,
    .periodic_publish_cb = app_gen_onoff_client_publish_interval_cb
};

static void switch_client_state_process_timing(app_switch_server_t * p_server)
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

static void switch_client_state_value_update(app_switch_server_t * p_server)
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

/* This callback is called periodically if model is configured for periodic publishing */
static void app_gen_onoff_client_publish_interval_cb(access_model_handle_t handle, void *p_self)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_WARN, "Publish desired message here.\n");
}

/* Acknowledged transaction status callback, if acknowledged transfer fails, application can
* determine suitable course of action (e.g. re-initiate previous transaction) by using this
* callback.
*/
static void app_gen_onoff_client_transaction_status_cb(access_model_handle_t model_handle,
    void *p_args,
    access_reliable_status_t status)
{
    switch (status)
    {
    case ACCESS_RELIABLE_TRANSFER_SUCCESS:
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Acknowledged transfer success.\n");
        break;

    case ACCESS_RELIABLE_TRANSFER_TIMEOUT:
        hal_led_blink_ms(LEDS_MASK, LED_BLINK_SHORT_INTERVAL_MS, LED_BLINK_CNT_NO_REPLY);
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Acknowledged transfer timeout.\n");
        break;

    case ACCESS_RELIABLE_TRANSFER_CANCELLED:
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Acknowledged transfer cancelled.\n");
        break;

    default:
        ERROR_CHECK(NRF_ERROR_INTERNAL);
        break;
    }
}

/* Generic OnOff client model interface: Process the received status message in this callback */
static void app_generic_onoff_client_status_cb(const generic_onoff_client_t *p_self,
    const access_message_rx_meta_t *p_meta,
    const generic_onoff_status_params_t *p_in)
{
    if (p_in->remaining_time_ms > 0)
    {
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "OnOff server: 0x%04x, Present OnOff: %d, Target OnOff: %d, Remaining Time: %d ms\n",
            p_meta->src.value, p_in->present_on_off, p_in->target_on_off, p_in->remaining_time_ms);
    }
    else
    {
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "OnOff server: 0x%04x, Present OnOff: %d\n",
            p_meta->src.value, p_in->present_on_off);
    }
}


/***** Interface functions *****/
void app_switch_client_status_publish(app_switch_server_t * p_server)
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

uint32_t app_switch_client_init(app_switch_server_t * p_server, uint8_t element_index)
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