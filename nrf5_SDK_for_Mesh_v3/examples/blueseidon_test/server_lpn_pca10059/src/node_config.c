/**
 * node_config.c 
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
#include <string.h>

#include "node_config.h"

/* HAL */
//#include "app_timer.h"
#include "boards.h"
#include "simple_hal.h"

/* Provisioning and configuration */
#include "mesh_app_utils.h"

/* Models */
#include "generic_onoff_server.h"
#include "generic_onoff_client.h"
#include "generic_level_server.h"
#include "generic_level_client.h"

/* Logging and RTT */
#include "log.h"

/* Example specific includes */
#include "app_level_userdefined.h"
#include "app_switch.h"
#include "example_common.h"
#include "flash_write.h"

#define SWITCH_SERVER_1_PIN (BSP_LED_0)
#define SWITCH_SERVER_2_PIN (BSP_LED_1)
#define SWITCH_SERVER_3_PIN (BSP_LED_2)
#define SWITCH_SERVER_4_PIN (BSP_LED_3)

/***********************************Get node configuration code*************************************/

uint8_t get_node_configuration(int CONFIG) {
  uint32_t config = get_node_config();
  uint8_t CHANNEL_1 = (uint8_t)(config >> 28);
  uint8_t CHANNEL_2 = (((uint8_t)(config >> 24)) & 0b00001111);
  uint8_t CHANNEL_3 = (((uint8_t)(config >> 20)) & 0b00001111);
  uint8_t CHANNEL_4 = (((uint8_t)(config >> 16)) & 0b00001111);
  uint8_t LPN_ONOFF = (((uint8_t)(config >> 12)) & 0b00001111);
  uint8_t LPN_POLL = (((uint8_t)(config >> 8)) & 0b00001111);
  uint8_t LPN_DELAY = (((uint8_t)(config >> 4)) & 0b00001111);
  uint8_t LPN_RX = (((uint8_t)(config >> 0)) & 0b00001111);

  switch (CONFIG) {

  case 0: {
    return CHANNEL_1;
    break;
  }

  case 1: {
    return CHANNEL_2;
    break;
  }

  case 2: {
    return CHANNEL_3;
    break;
  }

  case 3: {
    return CHANNEL_4;
    break;
  }

  case 4: {
    return 1;// LPN_ONOFF;
    break;
  }

  case 5: {
    return 1;//LPN_POLL;
    break;
  }

  case 6: {
    return LPN_DELAY;
    break;
  }

  case 7: {
    return LPN_RX;
    break;
  }

  default:
    return 0;
    break;
  }
}

/*********************************************END***************************************************/
/************************Initialisation for node config level server model**************************/
//static void app_level_config_server_set_cb(const app_level_server_t *p_server, uint32_t present_level);
//static void app_level_config_server_get_cb(const app_level_server_t *p_server, uint32_t *p_present_level);

/* Callback for updating the hardware state */
static void app_level_config_server_set_cb(const app_level_server_t *p_server, uint32_t present_level) {
  /* Resolve the server instance here if required, this example uses only 1 instance. */
  set_node_config(present_level);
}

/* Callback for reading the hardware state */
static void app_level_config_server_get_cb(const app_level_server_t *p_server, uint32_t *p_present_level) {
  /* Resolve the server instance here if required, this example uses only 1 instance. */
  *p_present_level = get_node_config();
}
/*******************************************END***************************************************/
/******************************Initialisation of SWITCH server MODEL******************************/
/* Callback for updating the hardware state */
static void app_switch_server_1_set_cb(const app_switch_server_t *p_server, bool switch_state) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Setting GPIO value: %d\n", switch_state)

  hal_led_pin_set(SWITCH_SERVER_1_PIN, switch_state);
}

/* Callback for updating the hardware state */
static void app_switch_server_2_set_cb(const app_switch_server_t *p_server, bool switch_state) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Setting GPIO value: %d\n", switch_state)

  hal_led_pin_set(SWITCH_SERVER_2_PIN, switch_state);
}

/* Callback for updating the hardware state */
static void app_switch_server_3_set_cb(const app_switch_server_t *p_server, bool switch_state) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Setting GPIO value: %d\n", switch_state)

  hal_led_pin_set(SWITCH_SERVER_3_PIN, switch_state);
}

/* Callback for updating the hardware state */
static void app_switch_server_4_set_cb(const app_switch_server_t *p_server, bool switch_state) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Setting GPIO value: %d\n", switch_state)

  hal_led_pin_set(SWITCH_SERVER_4_PIN, switch_state);
}

/* Callback for reading the hardware state */
static void app_switch_server_1_get_cb(const app_switch_server_t *p_server, bool *p_present_switch) {
  *p_present_switch = hal_led_pin_get(SWITCH_SERVER_1_PIN);
}

/* Callback for reading the hardware state */
static void app_switch_server_2_get_cb(const app_switch_server_t *p_server, bool *p_present_switch) {
  *p_present_switch = hal_led_pin_get(SWITCH_SERVER_2_PIN);
}

/* Callback for reading the hardware state */
static void app_switch_server_3_get_cb(const app_switch_server_t *p_server, bool *p_present_switch) {
  *p_present_switch = hal_led_pin_get(SWITCH_SERVER_3_PIN);
}

/* Callback for reading the hardware state */
static void app_switch_server_4_get_cb(const app_switch_server_t *p_server, bool *p_present_switch) {
  *p_present_switch = hal_led_pin_get(SWITCH_SERVER_4_PIN);
}
/*******************************************END***************************************************/
/******************************Initialisation of LEVEL server MODEL*******************************/
/* Callback for updating the hardware state */
static void app_level_server_1_set_cb(const app_level_server_t *p_server, uint32_t present_level) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Setting GPIO value: %d\n", present_level)
}

/* Callback for updating the hardware state */
static void app_level_server_2_set_cb(const app_level_server_t *p_server, uint32_t present_level) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Setting GPIO value: %d\n", present_level)
}

/* Callback for updating the hardware state */
static void app_level_server_3_set_cb(const app_level_server_t *p_server, uint32_t present_level) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Setting GPIO value: %d\n", present_level)
}

/* Callback for updating the hardware state */
static void app_level_server_4_set_cb(const app_level_server_t *p_server, uint32_t present_level) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Setting GPIO value: %d\n", present_level)
}

/* Callback for reading the hardware state */
static void app_level_server_1_get_cb(const app_level_server_t *p_server, uint32_t *present_level) {
  *present_level = 1;
}

/* Callback for reading the hardware state */
static void app_level_server_2_get_cb(const app_level_server_t *p_server, uint32_t *present_level) {
  *present_level = 2;
}

/* Callback for reading the hardware state */
static void app_level_server_3_get_cb(const app_level_server_t *p_server, uint32_t *present_level) {
  *present_level = 3;
}

/* Callback for reading the hardware state */
static void app_level_server_4_get_cb(const app_level_server_t *p_server, uint32_t *present_level) {
  *present_level = 4;
}
/*******************************************END***************************************************/
/******************************Initialisation of SWITCH client MODEL******************************/
/* Forward declaration */
static void app_gen_onoff_client_publish_interval_cb(access_model_handle_t handle, void *p_self);
static void app_generic_onoff_client_status_cb(const generic_onoff_client_t *p_self,
    const access_message_rx_meta_t *p_meta,
    const generic_onoff_status_params_t *p_in);
static void app_gen_onoff_client_transaction_status_cb(access_model_handle_t model_handle,
    void *p_args,
    access_reliable_status_t status);

const generic_onoff_client_callbacks_t switch_client_cbs =
    {
        .onoff_status_cb = app_generic_onoff_client_status_cb,
        .ack_transaction_status_cb = app_gen_onoff_client_transaction_status_cb,
        .periodic_publish_cb = app_gen_onoff_client_publish_interval_cb};

/* This callback is called periodically if model is configured for periodic publishing */
static void app_gen_onoff_client_publish_interval_cb(access_model_handle_t handle, void *p_self) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_WARN, "Publish desired message here.\n");
}

/* Acknowledged transaction status callback, if acknowledged transfer fails, application can
* determine suitable course of action (e.g. re-initiate previous transaction) by using this
* callback.
*/
static void app_gen_onoff_client_transaction_status_cb(access_model_handle_t model_handle,
    void *p_args,
    access_reliable_status_t status) {
  switch (status) {
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
    const generic_onoff_status_params_t *p_in) {
  if (p_in->remaining_time_ms > 0) {
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "OnOff server: 0x%04x, Present OnOff: %d, Target OnOff: %d, Remaining Time: %d ms\n",
        p_meta->src.value, p_in->present_on_off, p_in->target_on_off, p_in->remaining_time_ms);
  } else {
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "OnOff server: 0x%04x, Present OnOff: %d\n",
        p_meta->src.value, p_in->present_on_off);
  }
}
/*******************************************END***************************************************/
/******************************Initialisation of LEVEL client MODEL*******************************/
/* Forward declaration */
static void app_gen_level_client_publish_interval_cb(access_model_handle_t handle, void *p_self);
static void app_generic_level_client_status_cb(const generic_level_client_t *p_self,
    const access_message_rx_meta_t *p_meta,
    const generic_level_status_params_t *p_in);
static void app_gen_level_client_transaction_status_cb(access_model_handle_t model_handle,
    void *p_args,
    access_reliable_status_t status);

const generic_level_client_callbacks_t level_client_cbs =
    {
        .level_status_cb = app_generic_level_client_status_cb,
        .ack_transaction_status_cb = app_gen_level_client_transaction_status_cb,
        .periodic_publish_cb = app_gen_level_client_publish_interval_cb};

/* This callback is called periodically if model is configured for periodic publishing */
static void app_gen_level_client_publish_interval_cb(access_model_handle_t handle, void *p_self) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_WARN, "Publish desired message here.\n");
}

/* Acknowledged transaction status callback, if acknowledged transfer fails, application can
* determine suitable course of action (e.g. re-initiate previous transaction) by using this
* callback.
*/
static void app_gen_level_client_transaction_status_cb(access_model_handle_t model_handle,
    void *p_args,
    access_reliable_status_t status) {
  switch (status) {
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

/* Generic Level client model interface: Process the received status message in this callback */
static void app_generic_level_client_status_cb(const generic_level_client_t *p_self,
    const access_message_rx_meta_t *p_meta,
    const generic_level_status_params_t *p_in) {
  if (p_in->remaining_time_ms > 0) {
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Level server: 0x%04x, Present Level: %d, Target Level: %d, Remaining Time: %d ms\n",
        p_meta->src.value, p_in->present_level, p_in->target_level, p_in->remaining_time_ms);
  } else {
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Level server: 0x%04x, Present Level: %d\n",
        p_meta->src.value, p_in->present_level);
  }
}

/*******************************************END***************************************************/
/***********************************Initialisation of ALL MODELS**********************************/
#define SWITCH_SERVER_CONFIG (1)
#define SWITCH_CLIENT_CONFIG (2)
#define LEVEL_SERVER_CONFIG (3)
#define LEVEL_CLIENT_CONFIG (4)

void app_model_init(void) {
  node_config_flash_init();

  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "\n Channel 1: %u | Channel 2: %u | Channel 3: %u | Channel 4: %u \n LPN State: %u | LPN Poll time: %u | LPN Delay: %u | LPN Receive Time: %u \n",
      get_node_configuration(CONFIG_CHANNEL_1), get_node_configuration(CONFIG_CHANNEL_2), get_node_configuration(CONFIG_CHANNEL_3), get_node_configuration(CONFIG_CHANNEL_4),
      get_node_configuration(CONFIG_LPN_ONOFF), LPN_POLL_TIME[get_node_configuration(CONFIG_LPN_POLL)], LPN_DELEAY_TIME[get_node_configuration(CONFIG_LPN_DELAY)], LPN_RX_WINDOW_TIME[get_node_configuration(CONFIG_LPN_RX_WINDOW)]);

  /* Instantiate level server on element index 0 */
    APP_TIMER_DEF (level_server_timer_0);
    m_level_servers[0].server.settings.force_segmented = APP_CONFIG_FORCE_SEGMENTATION;
    m_level_servers[0].server.settings.transmic_size = APP_CONFIG_MIC_SIZE;
    m_level_servers[0].timer.p_timer_id = &level_server_timer_0;
    m_level_servers[0].p_dtt_ms = NULL;
    m_level_servers[0].level_set_cb = app_level_config_server_set_cb;
    m_level_servers[0].level_get_cb = app_level_config_server_get_cb;
    ERROR_CHECK(app_level_init(&m_level_servers[0], 0));
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "App node config Model Handle: %d\n", m_level_servers[0].server.model_handle);

    static void (*app_switch_server_set_cb[])(const app_switch_server_t *, bool) = {app_switch_server_1_set_cb, app_switch_server_2_set_cb, 
                                                                                      app_switch_server_3_set_cb, app_switch_server_4_set_cb};

    static void (*app_switch_server_get_cb[])(const app_switch_server_t *, bool *) = {app_switch_server_1_get_cb, app_switch_server_2_get_cb, 
                                                                                      app_switch_server_3_get_cb, app_switch_server_4_get_cb};

    static void (*app_level_server_set_cb[])(const app_level_server_t *, uint32_t) = {app_level_server_1_set_cb, app_level_server_2_set_cb, 
                                                                                      app_level_server_3_set_cb, app_level_server_4_set_cb};

    static void (*app_level_server_get_cb[])(const app_level_server_t *, uint32_t *) = {app_level_server_1_get_cb, app_level_server_2_get_cb, 
                                                                                      app_level_server_3_get_cb, app_level_server_4_get_cb};

    APP_TIMER_DEF (switch_server_timer_1);
    m_switch_servers[0].p_timer_id = &switch_server_timer_1;

    APP_TIMER_DEF (switch_server_timer_2);
    m_switch_servers[1].p_timer_id = &switch_server_timer_2;

    APP_TIMER_DEF (switch_server_timer_3);
    m_switch_servers[2].p_timer_id = &switch_server_timer_3;

    APP_TIMER_DEF (switch_server_timer_4);
    m_switch_servers[3].p_timer_id = &switch_server_timer_4;

    APP_TIMER_DEF (level_server_timer_1);
    m_level_servers[1].timer.p_timer_id = &level_server_timer_1;

    APP_TIMER_DEF (level_server_timer_2);
    m_level_servers[2].timer.p_timer_id = &level_server_timer_2;

    APP_TIMER_DEF (level_server_timer_3);
    m_level_servers[3].timer.p_timer_id = &level_server_timer_3;

    APP_TIMER_DEF (level_server_timer_4);
    m_level_servers[4].timer.p_timer_id = &level_server_timer_4;

  for(int i=0; i<4; i++){
      if (get_node_configuration(i) == SWITCH_SERVER_CONFIG) {
        m_switch_servers[i].server.settings.force_segmented = APP_CONFIG_FORCE_SEGMENTATION;
        m_switch_servers[i].server.settings.transmic_size = APP_CONFIG_MIC_SIZE;
        m_switch_servers[i].switch_set_cb = *app_switch_server_set_cb[i];
        m_switch_servers[i].switch_get_cb = *app_switch_server_get_cb[i];

        ERROR_CHECK(app_switch_init(&m_switch_servers[i], (i+1)));
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "App switch server Model %d Handle: %d\n", (i+1), m_switch_servers[i].server.model_handle);
      }
      if (get_node_configuration(i) == SWITCH_CLIENT_CONFIG) {
        m_switch_clients[i].settings.p_callbacks = &switch_client_cbs;
        m_switch_clients[i].settings.timeout = 0;
        m_switch_clients[i].settings.force_segmented = APP_CONFIG_FORCE_SEGMENTATION;
        m_switch_clients[i].settings.transmic_size = APP_CONFIG_MIC_SIZE;

        ERROR_CHECK(generic_onoff_client_init(&m_switch_clients[i], (i+1)));
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "App switch client Model %d Handle: %d\n", (i+1), m_switch_clients[i].model_handle);
      }
      if (get_node_configuration(i) == LEVEL_SERVER_CONFIG) {
        m_level_servers[i+1].server.settings.force_segmented = APP_CONFIG_FORCE_SEGMENTATION;
        m_level_servers[i+1].server.settings.transmic_size = APP_CONFIG_MIC_SIZE;
        m_level_servers[i+1].p_dtt_ms = NULL;
        m_level_servers[i+1].level_set_cb = *app_level_server_set_cb[i];
        m_level_servers[i+1].level_get_cb = *app_level_server_get_cb[i];

        ERROR_CHECK(app_level_init(&m_level_servers[i+1], (i+1)));
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "App level server Model %d Handle: %d\n", (i+1), m_level_servers[i+1].server.model_handle);
      }
      if (get_node_configuration(i) == LEVEL_CLIENT_CONFIG) {
        m_level_clients[i].settings.p_callbacks = &level_client_cbs;
        m_level_clients[i].settings.timeout = 0;
        m_level_clients[i].settings.force_segmented = APP_CONFIG_FORCE_SEGMENTATION;
        m_level_clients[i].settings.transmic_size = APP_CONFIG_MIC_SIZE;

        ERROR_CHECK(generic_level_client_init(&m_level_clients[i], (i+1)));
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "App level client Model %d Handle: %d\n", (i+1), m_level_clients[i].model_handle);
      }
  }
}
/*******************************************END***************************************************/