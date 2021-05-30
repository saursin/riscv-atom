#ifndef __STDIO_H___
#define __STDIO_H___

#include "atomrv.h"

#include "math.h"

void putchar(char c)
{
    volatile char* tx = (volatile char*) STDIO_TX_ADDRESS;
    volatile char* tx_ack = (volatile char*) STDIO_TX_ACK_ADDRESS;
    *tx = c;

    // Toggle tx_ack
    *tx_ack = (char) 1;
    *tx_ack = (char) 0;
}

char getchar()
{
    volatile char* rx = (volatile char*) STDIO_RX_ADDRESS;
    return *rx;
}

void putstring(char* str)
{
    volatile char* tx = (volatile char*) STDIO_TX_ADDRESS;
    while (*str) {
        putchar(*str);
        str++;
    }
}

void printnumber(int i)
{
    if(i/10!=0)
    {
        putchar(i%10);
        printnumber((i-i%10)/10);
    }
    else if((i/10==0) && (i%10!=0) && (i>0))
        putchar(i%10);
    else if((i/10==0) && (i%10!=0) && (i<=0))
        putchar(-i%10);
}

#endif // __STDIO_H___
