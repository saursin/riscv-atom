#pragma once
#include <stdint.h>

// Toggle the boot LED (progress indicator), defined in boot.c
void bootled_toggle();

// UART Boot: copies a program from the uart stream to memory (XMODEM); 0 on success
int uartboot(uint8_t *dest, uint32_t len);
