#ifndef FLASH_WRITE_H__
#define FLASH_WRITE_H__

void set_node_config(uint32_t config);

uint32_t get_node_config(void);

void clear_node_config(void);

void node_config_flash_init(void);

#endif  /* FLASH_WRITE_H__ */