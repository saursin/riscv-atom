#include "atomrv.h"

void putchr(char c)
{
    volatile char* tx = (volatile char*) STDIO_TX_ADDRESS;
    volatile char* tx_ack = (volatile char*) STDIO_TX_ACK_ADDRESS;
    *tx = c;

    // Toggle tx_ack
    *tx_ack = (char) 1;
    *tx_ack = (char) 0;
}

char getchr()
{
    volatile char* rx = (volatile char*) STDIO_RX_ADDRESS;
    return *rx;
}

void prints(char* str)
{
    volatile char* tx = (volatile char*) STDIO_TX_ADDRESS;
    while (*str) {
        putchr(*str);
        str++;
    }
}