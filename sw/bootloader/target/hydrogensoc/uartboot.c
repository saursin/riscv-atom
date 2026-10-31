#include <platform.h>
#include <serial.h>
#include <stdio.h>
#include <time.h>

#include "common.h"
#include "uartboot.h"

// Xmodem timeout
#define XMODEM_WAIT_TIMEOUT_SEC 120

// Getchar timeout
#define GETCHAR_TIMEOUT_MS 50

// Wait for the sender after each 'C'
#define C_INTERVAL_MS 1000

#define XMODEM_PACKET_SZ 128

//////////////////////////////////////////////////////////////
// CRC Functions

// CRC-16/XMODEM (poly 0x1021)
static uint16_t crc16_ccitt(const uint8_t *buf, int len)
{
    uint16_t crc = 0;
    while(len--) {
        crc ^= (uint16_t)(*buf++) << 8;
        for(int i = 0; i < 8; i++)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}

//////////////////////////////////////////////////////////////
// Xmodem Functions

// Receives the rest of a packet (after SOH) into buf. SUCCESS: packet pkt_num; REPEAT: an
// intact copy of the previous packet; FAILURE: corrupted, or out of sequence.
static int get_packet(uint8_t pkt_num, uint8_t *buf)
{
    uint8_t n = serial_read_timeout(GETCHAR_TIMEOUT_MS);
    uint8_t ni = serial_read_timeout(GETCHAR_TIMEOUT_MS);
    for(uint8_t i=0; i<XMODEM_PACKET_SZ; i++)
        buf[i] = serial_read_timeout(GETCHAR_TIMEOUT_MS);

    uint16_t chk_got = ((serial_read_timeout(GETCHAR_TIMEOUT_MS)) << 8)
                        | (serial_read_timeout(GETCHAR_TIMEOUT_MS));
    uint16_t chk_calc = crc16_ccitt(buf, 128);

    if((ni != (uint8_t)(255-n)) || (chk_got != chk_calc))
        return FAILURE;
    if(n == pkt_num)
        return SUCCESS;
    if(n == (uint8_t)(pkt_num-1))
        return REPEAT;
    return FAILURE;
}

xmod_status xmodemReceive(uint8_t * bf, uint32_t len)
{
    // Clear the receive buffer, finish sending the prompt
    while(serial_read_timeout(0) != -1)
        ;
    serial_flush();

    // SPAM till SOH
    bool got_soh = false;
    clock_t t_end = cycle() + (((clock_t) XMODEM_WAIT_TIMEOUT_SEC)*((clock_t) platform_clk()));
    while (cycle() < t_end)
    {
        serial_write('C');
        bootled_toggle();   // blink while waiting for the sender
        int c = serial_read_timeout(C_INTERVAL_MS);
        if (c == (int)-1)
            continue;   // not heard from sender
      
        if(c == (int)SOH) {
            got_soh = true;
            break;
        }
        else if(c==EOT)
            return S_EOT;   // normal end of transmission
        else if(c==CAN)
            return S_TERM;  // terminated by host
        // anything else is line noise (e.g. from a terminal connecting): keep waiting
    }

    if(!got_soh) {
        serial_write(CAN);
        serial_write(CAN);
        serial_write(CAN);
        return TIMEOUT;   // connection timeout
    }


    int packetnum = 0;
    bool have_soh = true;   // the first packet's SOH ended the handshake
    while(packetnum<len/128)
    {
        if(!have_soh)
        {
            // check for start
            int c = serial_read_timeout(GETCHAR_TIMEOUT_MS);
            if(c==SOH){}
            else if (c==EOT) {
                serial_write(ACK);
                return S_EOT;
            }
            else if(c==CAN) {
                serial_write(ACK);
                return S_TERM;
            }
            else
                return UNKCHAR;

        }
        
        uint8_t *pkt_ptr = (bf+packetnum*128);

        // get packet; only a good one advances. A bad one is asked for again (NAK) once the
        // line is quiet; a repeat of the previous one, sent because our ACK got lost, is
        // acknowledged again and dropped (the next packet overwrites it).
        xmod_status s = get_packet(packetnum+1, pkt_ptr);
        have_soh = false;
        if(s == FAILURE) {
            while(serial_read_timeout(GETCHAR_TIMEOUT_MS) != -1)    // until the line is quiet
                ;
            serial_write(NAK);
        } else {
            serial_write(ACK);
            if(s == SUCCESS)
                packetnum++;
        }
        bootled_toggle();
    }

    boot_panic(RCODE_UNREACHABLE, "Unreachable");
    return FAILURE;
}

// UART Boot
void uartboot(uint8_t *dest, uint32_t len){
    P(puts("uartboot: Send a binary file using xmodem...\n");)

    xmod_status status = xmodemReceive(dest, len);

    P(puts("\nuartboot: ");)
    switch(status){
        case S_EOT:
            P(puts("Successful!\n");)
            return;

        case S_TERM:
            P(puts("Terminated by host");)
            break;

        case UNKCHAR: 
            P(puts("Got unknown character");)
            break;
        
        case TIMEOUT:
            P(puts("Timeout");)
            break;

        default:
            P(putchar('?');)
            break;
    }
    
    P(putchar('\n');)
    boot_panic(RCODE_UARTBOOT_FAILED, "Uartboot failed");
}