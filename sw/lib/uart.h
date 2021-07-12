#ifndef __UART_H__
#define __UART_H__

#include "atombones.h"

/*
    Uart Status register format
    b0: Tx_valid
    b1: Rx_ack
    b2: 
    b3: 
    b4:
    b5:
    b6: 
    b7: 
*/


int uart_send(char c)
{
    *((volatile char*) IO_UART_TX_ADDRESS) = c;
    *((volatile char*) IO_UART_SREG_ADDRESS) = 0;
    *((volatile char*) IO_UART_SREG_ADDRESS) = 1;
    return 1;
}

char uart_recieve()
{
    return *((volatile char*) IO_UART_TX_ADDRESS);
    *((volatile char*) IO_UART_SREG_ADDRESS) = 0;
    *((volatile char*) IO_UART_SREG_ADDRESS) = 2;
}

char uart_status()
{
    return *((volatile char*) IO_UART_SREG_ADDRESS);
}

#endif //__UART_H__