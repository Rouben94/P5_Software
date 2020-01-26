#ifndef NODE_CONFIG_H__
#define NODE_CONFIG_H__

#include "app_switch.h"
#include "generic_onoff_client.h"
#include "app_level_userdefined.h"
#include "generic_level_client.h"

static app_switch_server_t m_switch_servers[4];
static generic_onoff_client_t m_switch_clients[4];

static app_level_server_t m_level_servers[5];
static generic_level_client_t m_level_clients[4];

static const enum CONFIG {
  CONFIG_CHANNEL_1,
  CONFIG_CHANNEL_2,
  CONFIG_CHANNEL_3,
  CONFIG_CHANNEL_4,
  CONFIG_LPN_ONOFF,
  CONFIG_LPN_POLL,
  CONFIG_LPN_DELAY,
  CONFIG_LPN_RX_WINDOW
} e_config;

static const int LPN_POLL_TIME[8]      = {2000, 10000, 60000, 600000, 3600000, 43200000, 86400000, 345599900}; // 2s - 10s - 1min - 10min - 1h - 12h - 24h- 96h
static const int LPN_DELEAY_TIME[8]    = {10, 64, 100, 128, 160, 192, 224, 255}; // 10ms - 64ms - 100ms - 128ms - 160ms - 192ms - 224ms - 255ms
static const int LPN_RX_WINDOW_TIME[8] = {0, 1, 2, 3, 0, 0, 0, 0}; //receive window 1.0 - 1.5 - 2 - 2.5

uint8_t get_node_configuration(int CONFIG);

void app_model_init(void);

#endif  /* NODE_CONFIG_H__ */