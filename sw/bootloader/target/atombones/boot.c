#include "common.h"

#include <stdint.h>

extern uint32_t __approm_start;

void * platform_init() {
    // Jump to start of approm
    return (void *) &__approm_start;
}