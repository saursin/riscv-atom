#pragma once
#include <stdint.h>

// Flashboot: Copied data from flash to memory
void flashboot(uint8_t *dest, uint32_t src_addr, uint32_t len);
