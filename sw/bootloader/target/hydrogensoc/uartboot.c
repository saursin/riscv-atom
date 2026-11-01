#include <stdio.h>
#include <xmodem.h>

#include "common.h"
#include "uartboot.h"

// the boot LED blinks while XMODEM waits and receives
void xmodem_activity(void)
{
    bootled_toggle();
}

int uartboot(uint8_t *dest, uint32_t len){
    P(puts("uartboot: waiting for a program (XMODEM)\n");)
    int n = xmodem_recv(dest, len);
    P(puts("\nuartboot: ");)
    if(n > 0) {
        P(puts("Successful!\n");)
        return 0;
    }
    P(puts(n == XMODEM_ERR_TIMEOUT ? "Timeout\n" : n == XMODEM_ERR_CANCEL ? "Terminated by host\n" : "Failed\n");)
    return RCODE_UARTBOOT_FAILED;
}
