#pragma once

#include <stdint.h>

extern uint32_t __approm_start;

/**
 * @brief Platform specific initialization
 * 
 * @return void* address to jump to after boot
 */
void * platform_init() {
    // Jump to start of approm
    return (void *) &__approm_start;
}