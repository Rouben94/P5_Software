static static app_level_server_t level_server[] = { 
servers_switch{APP_SWITCH_SERVER_DEF_ENTRY(
    bla,
    APP_CONFIG_FORCE_SEGMENTATION,
    APP_CONFIG_MIC_SIZE,
    app_switch_server_1_set_cb,
    app_switch_server_1_get_cb),

    APP_SWITCH_SERVER_DEF_ENTRY(
    bla,
    APP_CONFIG_FORCE_SEGMENTATION,
    APP_CONFIG_MIC_SIZE,
    app_switch_server_1_set_cb,
    app_switch_server_1_get_cb)
    };