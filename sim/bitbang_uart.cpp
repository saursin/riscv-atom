#include "bitbang_uart.hpp"
#include <stdint.h>
#include <iostream>

//#define DBG

#ifdef DBG
#define D(x) x
#else
#define D(x)
#endif

BitbangUART::BitbangUART(bool * rx_pin, bool * tx_pin, unsigned fr):
    rx_pin(rx_pin),
    tx_pin(tx_pin),
    FR(fr)
{}


void BitbangUART::rx_eval()
{
    // Vars
    static bool prev_rx = false;       // prev tx pin value
    static UART_State state = IDLE;    // current state
    static int wait_cyc = 0;           // number of cycles to wait
    static int got_bits = 0;           // number of bits received
    static uint8_t byte = 0;           // data byte

    // Sample rx
    bool rx = *rx_pin;

    if(wait_cyc){
        D(printf("rx: wait %d\n", wait_cyc);)
        wait_cyc--;
    } else {
        switch(state) {
        case IDLE:
            // wait for falling edge on tx
            if (prev_rx==true && rx == false) {
                state = START_BIT;
                byte = 0;
                got_bits = 0;
                D(printf("rx: detected negedge on tx\n");)
                wait_cyc = FR/2-1;
            }
            break;
        
        case START_BIT:
            // check start bit
            if (rx == false) {
                D(printf("rx: start_bit: ok\n");)
                state = DATA_BITS;
                wait_cyc = FR-1;
            }
            else {
                D(printf("rx err: no start bit\n");)
                state = IDLE;
            }
            break;
        
        case DATA_BITS:
            byte = (byte >> 1) | (((uint8_t)rx) << 7);
            D(printf("rx: pin=%d; got_bits=%d; byte: %02x\n", rx, got_bits, byte);)
            got_bits++;

            if(got_bits == 8) {
                state = STOP_BIT;
            }
            else {
                state = DATA_BITS;  // loop
            }
            wait_cyc = FR-1;
            break;
        
        case STOP_BIT:
            // check stop bit
            if(rx==true) {
                D(printf("rx: Recieved Byte! = 0x%02x\n", byte);)
                rx_fifo.push(byte);
                wait_cyc = FR/2-1;
                state = IDLE;
            }
            else {
                printf("rx: Framing error, '%c' (0x%02x)\n", byte, byte);
                state = IDLE;
            }
            break;

        default:
            state = IDLE;

        }

    prev_rx = rx;
    }
}

void BitbangUART::tx_eval()
{
    static uint8_t byte = 0;
    static int wait_cyc = 0;           // number of cycles to wait
    static UART_State state = IDLE;    // current state
    static int sent_bits = 0;          // number of bits sent

    static bool txval = true;

    if(wait_cyc){
        D(printf("tx: wait %d\n", wait_cyc);)
        wait_cyc--;
    } else {
        switch(state) {
        case IDLE:
            txval = true;
            if(!tx_fifo.empty())
            {
                byte = tx_fifo.front();
                tx_fifo.pop();
                D(printf("tx: got byte, started transmitting\n");)
                sent_bits = 0;
                state = START_BIT;
            }
            break;
        
        case START_BIT:
            txval = false;
            state = DATA_BITS;
            wait_cyc = FR-1;
            break;
        
        case DATA_BITS:
            txval = (byte & 0x1) != 0;
            byte = byte >> 1;
            D(printf("tx: pin=%d; sent_bits=%d; byte: %02x\n", txval, sent_bits, byte);)
            sent_bits++;

            if(sent_bits == 8) {
                state = STOP_BIT;
            }
            else {
                state = DATA_BITS;  // loop
            }
            wait_cyc = FR-1;
            break;
        
        case STOP_BIT:
            txval = true;
            wait_cyc = FR-1;
            state = IDLE;
            break;

        default:
            state = IDLE;

        }
    }

    *tx_pin = txval;
}
