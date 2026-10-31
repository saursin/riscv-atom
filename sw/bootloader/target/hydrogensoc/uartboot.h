#pragma once
#include <stdint.h>

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

// Printed right before the XMODEM handshake; scripts/atomterm starts an upload when it sees it
#define UARTBOOT_MARKER "![XMODREQ]!"

typedef enum {
    OK      = 0,
    NOK     = 1,
    S_EOT   = 2,
    S_TERM  = 3,
    R_TERM  = 4,
    UNKCHAR = 5,
    TIMEOUT = 6,
    SUCCESS = 7,
    FAILURE = 8,
    REPEAT  = 9     // the previous packet again (the sender missed our ACK)
} xmod_status;

xmod_status xmodemReceive(uint8_t * bf, uint32_t len);

// CRC-16/XMODEM (poly 0x1021) of len bytes
uint16_t crc16_ccitt(const uint8_t *buf, int len);

// Toggle the boot LED (progress indicator), defined in boot.c
void bootled_toggle();

// UART Boot: copies data from the uart stream to memory using XMODEM; 0 on success
int uartboot(uint8_t *dest, uint32_t len);