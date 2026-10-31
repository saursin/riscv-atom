#pragma once
#include <stdint.h>

// Address offset at which the program image resides in flash
#define FLASH_IMG_OFFSET    0x000c0000

// Flashboot: copies data from flash to memory; 0 on success
int flashboot(uint8_t *dest, uint32_t src_addr, uint32_t len);
