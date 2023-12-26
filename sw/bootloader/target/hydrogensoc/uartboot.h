#pragma once
#include <stdint.h>

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

typedef enum {
    OK      = 0,
    NOK     = 1,
    S_EOT   = 2,
    S_TERM  = 3,
    R_TERM  = 4,
    UNKCHAR = 5,
    TIMEOUT = 6,
    SUCCESS = 7,
    FAILURE = 8
} xmod_status;

xmod_status xmodemReceive(uint8_t * bf, uint32_t len);

// UART Boot: Copies data from uart stream to memory using XMODEM protocol
void uartboot(uint8_t *dest, uint32_t len);