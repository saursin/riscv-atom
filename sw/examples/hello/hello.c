#include "../../lib/atomrv.h"

void main()
{
    volatile char* tx = (volatile char*) STDIO_TX_ADDRESS;
    volatile char* tx_ack = (volatile char*) STDIO_TX_ACK_ADDRESS;

    const char* hello = "Hello RISC-V!\n";

    while (*hello) {
        *tx = *hello;
        *tx_ack = 1;
        *tx_ack = 0;
        hello++;
    }
}
