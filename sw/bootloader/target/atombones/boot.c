#include "common.h"

#include <stdint.h>

extern uint32_t __approm_start;

void * platform_boot(int kind, int arg) {
    if(kind == BOOT_REENTRY)
        return NULL;    // a program exited: no console, stop
    
    // the program is already in RAM, return address
    return (void *) &__approm_start;
}