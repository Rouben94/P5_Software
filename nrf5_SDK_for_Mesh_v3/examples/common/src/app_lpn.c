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

#include "app_lpn.h"
#include "mesh_friendship_types.h"
#include "mesh_lpn.h"

/* nRF5 SDK */
#include "nrf_pwr_mgmt.h"
#include "nrf_soc.h"
#include "boards.h"
#include "simple_hal.h"


#include <stdint.h>

#include "sdk_config.h"
#include "example_common.h"
#include "generic_onoff_server.h"

#include "log.h"
#include "app_timer.h"

/** The maximum duration to scan for incoming Friend Offers. */
#define FRIEND_REQUEST_TIMEOUT_MS (MESH_LPN_FRIEND_REQUEST_TIMEOUT_MAX_MS)
/** The upper limit for two subsequent Friend Polls. */
#define POLL_TIMEOUT_MS (SEC_TO_MS(10))
/** The time between LPN sending a request and listening for a response. */
#define RECEIVE_DELAY_MS (100)


static void state_on_timer_handler(void *p_unused) {
  UNUSED_VARIABLE(p_unused);

  /* Send state off */
  //send_app_state(APP_STATE_OFF);
}

static void initiate_friendship() {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initiating the friendship establishment procedure.\n");

  mesh_lpn_friend_request_t freq;
  freq.friend_criteria.friend_queue_size_min_log = MESH_FRIENDSHIP_MIN_FRIEND_QUEUE_SIZE_16;
  freq.friend_criteria.receive_window_factor = MESH_FRIENDSHIP_RECEIVE_WINDOW_FACTOR_1_0;
  freq.friend_criteria.rssi_factor = MESH_FRIENDSHIP_RSSI_FACTOR_2_0;
  freq.poll_timeout_ms = POLL_TIMEOUT_MS;
  freq.receive_delay_ms = RECEIVE_DELAY_MS;

  uint32_t status = mesh_lpn_friend_request(freq, FRIEND_REQUEST_TIMEOUT_MS);
  switch (status) {
  case NRF_SUCCESS:
    break;

  case NRF_ERROR_INVALID_STATE:
    __LOG(LOG_SRC_APP, LOG_LEVEL_ERROR, "Already in an active friendship\n");
#if SIMPLE_HAL_LEDS_ENABLED
    hal_led_blink_ms(LEDS_MASK, LED_BLINK_SHORT_INTERVAL_MS, LED_BLINK_CNT_ERROR);
#endif
    break;

  case NRF_ERROR_INVALID_PARAM:
    __LOG(LOG_SRC_APP, LOG_LEVEL_ERROR, "Friend request parameters outside of valid ranges.\n");
#if SIMPLE_HAL_LEDS_ENABLED
    hal_led_blink_ms(LEDS_MASK, LED_BLINK_SHORT_INTERVAL_MS, LED_BLINK_CNT_ERROR);
#endif
    break;

  default:
    ERROR_CHECK(status);
    break;
  }
}

static void terminate_friendship() {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Terminating the active friendship\n");

  uint32_t status = mesh_lpn_friendship_terminate();
  switch (status) {
  case NRF_SUCCESS:
    break;

  case NRF_ERROR_INVALID_STATE:
    __LOG(LOG_SRC_APP, LOG_LEVEL_ERROR, "Not in an active friendship\n");
#if SIMPLE_HAL_LEDS_ENABLED
    hal_led_blink_ms(LEDS_MASK, LED_BLINK_SHORT_INTERVAL_MS, LED_BLINK_CNT_ERROR);
#endif
    break;

  default:
    ERROR_CHECK(status);
    break;
  }
}

static void app_mesh_core_event_cb(const nrf_mesh_evt_t *p_evt) {
  /* USER_NOTE: User can insert mesh core event proceesing here */
  switch (p_evt->type) {
  case NRF_MESH_EVT_LPN_FRIEND_OFFER: {
    const nrf_mesh_evt_lpn_friend_offer_t *p_offer = &p_evt->params.friend_offer;

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,
        "Received friend offer from 0x%04X\n",
        p_offer->src);

    uint32_t status = mesh_lpn_friend_accept(p_offer);
    switch (status) {
    case NRF_SUCCESS:
      break;

    case NRF_ERROR_INVALID_STATE:
    case NRF_ERROR_INVALID_PARAM:
    case NRF_ERROR_NULL:
      __LOG(LOG_SRC_APP, LOG_LEVEL_ERROR,
          "Cannot accept friendship: %d\n",
          status);
#if SIMPLE_HAL_LEDS_ENABLED
      hal_led_blink_ms(LEDS_MASK, LED_BLINK_SHORT_INTERVAL_MS,
          LED_BLINK_CNT_ERROR);
#endif
      break;

    default:
      ERROR_CHECK(status);
      break;
    }

    break;
  }

  case NRF_MESH_EVT_LPN_FRIEND_POLL_COMPLETE:
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Friend poll procedure complete\n");
    break;

  case NRF_MESH_EVT_LPN_FRIEND_REQUEST_TIMEOUT:
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Friend Request timed out\n");
#if SIMPLE_HAL_LEDS_ENABLED
    hal_led_blink_ms(LEDS_MASK, LED_BLINK_SHORT_INTERVAL_MS, LED_BLINK_CNT_ERROR);
#endif
    break;

  case NRF_MESH_EVT_FRIENDSHIP_ESTABLISHED: {
    const nrf_mesh_evt_friendship_established_t *p_est =
        &p_evt->params.friendship_established;
    (void)p_est;

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,
        "Friendship established with: 0x%04X\n",
        p_est->friend_src);

#if SIMPLE_HAL_LEDS_ENABLED
    hal_led_pin_set(BSP_LED_1, true);
#endif
    break;
  }

  case NRF_MESH_EVT_FRIENDSHIP_TERMINATED: {
    const nrf_mesh_evt_friendship_terminated_t *p_term = &p_evt->params.friendship_terminated;
    UNUSED_VARIABLE(p_term);

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,
        "Friendship with 0x%04X terminated. Reason: %d\n",
        p_term->friend_src, p_term->reason);

#if SIMPLE_HAL_LEDS_ENABLED
    hal_led_pin_set(BSP_LED_1, false);
#endif

    ERROR_CHECK(app_timer_stop(m_state_on_timer));
    break;
  }

  default:
    break;
  }
}

//