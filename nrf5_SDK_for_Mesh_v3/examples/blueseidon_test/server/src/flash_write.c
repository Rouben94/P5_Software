/**
 * flash_write.c 
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

#include "flash_write.h"

/* Core */
#include "mesh_stack.h"

/* Provisioning and configuration */
#include "mesh_app_utils.h"

/* Logging and RTT */
#include "log.h"

/* Example specific includes */
#include "flash_manager.h" // For storing custom data in flash.

typedef struct
{
    uint32_t config_bit_mask;
    uint32_t config_lpn_bit_mask;
} node_configuration_t;

static node_configuration_t node_config;

#define NODE_CONFIGURATION_ENTRY_HANDLE (0x0500)
#define FLASH_PAGE_COUNT (1)

/**** Flash handling ****/
static flash_manager_t m_flash_manager;

static void app_flash_manager_add(void);

static void flash_write_complete(const flash_manager_t *p_manager, const fm_entry_t *p_entry, fm_result_t result)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Flash write complete\n");

    /* If we get an AREA_FULL then our calculations for flash space required are buggy. */
    NRF_MESH_ASSERT(result != FM_RESULT_ERROR_AREA_FULL);

    /* We do not invalidate in this module, so a NOT_FOUND should not be received. */
    NRF_MESH_ASSERT(result != FM_RESULT_ERROR_NOT_FOUND);
    if (result == FM_RESULT_ERROR_FLASH_MALFUNCTION)
    {
        ERROR_CHECK(NRF_ERROR_NO_MEM);
    }
}

static void flash_invalidate_complete(const flash_manager_t *p_manager, fm_handle_t handle, fm_result_t result)
{
    /* This application does not expect invalidate complete calls. */
    ERROR_CHECK(NRF_ERROR_INTERNAL);
}

typedef void (*flash_op_func_t)(void);
static void flash_manager_mem_available(void *p_args)
{
    ((flash_op_func_t)p_args)(); /*lint !e611 Suspicious cast */
}

static void flash_remove_complete(const flash_manager_t *p_manager)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Flash remove complete\n");
}

static void app_flash_manager_add(void)
{

    static fm_mem_listener_t flash_add_mem_available_struct = {
        .callback = flash_manager_mem_available,
        .p_args = app_flash_manager_add};

    const uint32_t *start_address;
    uint32_t allocated_area_size;
    ERROR_CHECK(mesh_stack_persistence_flash_usage(&start_address, &allocated_area_size));

    flash_manager_config_t manager_config;
    manager_config.write_complete_cb = flash_write_complete;
    manager_config.invalidate_complete_cb = flash_invalidate_complete;
    manager_config.remove_complete_cb = flash_remove_complete;
    manager_config.min_available_space = WORD_SIZE;
    manager_config.p_area = (const flash_manager_page_t *)((uint32_t)start_address - PAGE_SIZE * FLASH_PAGE_COUNT);
    manager_config.page_count = FLASH_PAGE_COUNT;
    uint32_t status = flash_manager_add(&m_flash_manager, &manager_config);
    if (NRF_SUCCESS != status)
    {
        flash_manager_mem_listener_register(&flash_add_mem_available_struct);
        __LOG(LOG_SRC_APP, LOG_LEVEL_ERROR, "Unable to add flash manager for node configuration\n");
    }
}

static bool load_app_data(void)
{
    uint32_t length = sizeof(node_config);
    uint32_t status = flash_manager_entry_read(&m_flash_manager,
        NODE_CONFIGURATION_ENTRY_HANDLE,
        &node_config,
        &length);
    if (status != NRF_SUCCESS)
    {
        memset(&node_config, 0x00, sizeof(node_config));
    }

    return (status == NRF_SUCCESS);
}

static uint32_t store_app_data(void)
{
    fm_entry_t *p_entry = flash_manager_entry_alloc(&m_flash_manager, NODE_CONFIGURATION_ENTRY_HANDLE, sizeof(node_config));
    static fm_mem_listener_t flash_add_mem_available_struct = {
        .callback = flash_manager_mem_available,
        .p_args = store_app_data};

    if (p_entry == NULL)
    {
        flash_manager_mem_listener_register(&flash_add_mem_available_struct);
    }
    else
    {
        node_configuration_t *p_nw_state = (node_configuration_t *)p_entry->data;
        memcpy(p_nw_state, &node_config, sizeof(node_config));
        flash_manager_entry_commit(p_entry);
    }

    return NRF_SUCCESS;
}

static void clear_app_data(void)
{
    memset(&node_config, 0x00, sizeof(node_config));

    if (flash_manager_remove(&m_flash_manager) != NRF_SUCCESS)
    {
        /* Register the listener and wait for some memory to be freed up before we retry. */
        static fm_mem_listener_t mem_listener = {.callback = flash_manager_mem_available,
            .p_args = clear_app_data};
        flash_manager_mem_listener_register(&mem_listener);
    }
}

void set_node_config(uint32_t config)
{
    node_config.config_bit_mask = config;
    ERROR_CHECK(store_app_data());
}

uint32_t get_node_config(void)
{
    load_app_data();
    return node_config.config_bit_mask;
}

void clear_node_config(void)
{
    clear_app_data();
}

void node_config_flash_init(void)
{
    app_flash_manager_add();
}