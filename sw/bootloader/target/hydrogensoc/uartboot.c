#include <platform.h>
#include <serial.h>
#include <stdio.h>
#include <time.h>
#include <utils.h>
#include <mmio.h>

#include "common.h"
#include "uartboot.h"

// Xmodem timeout
#define XMODEM_WAIT_TIMEOUT_SEC 120

// Getchar timeout
#ifdef SIM
#define GETCHAR_TIMEOUT_MS 10
#else
#define GETCHAR_TIMEOUT_MS 50
#endif

#define XMODEM_PACKET_SZ 128

// Register Offsets
#define UART_REG_RBR 0x00
#define UART_REG_THR 0x00
#define UART_REG_LCR 0x04
#define UART_REG_LSR 0x08

//////////////////////////////////////////////////////////
// UART Driver Functions

// Get a char from uart (with timeout)
static int _inbyte(uint64_t timeout_ms)
{
    clock_t t_end = cycle() + ((timeout_ms*CLK_FREQ)/1000);
	while (!bitcheck(REG32(UART_ADDR, UART_REG_LSR), 0)) {
		if (cycle() > t_end)
			return -1;  // Timeout
	}
	return (int) REG8(UART_ADDR, UART_REG_RBR);
}

// Sends a char to uart
static void _outbyte(unsigned char c)
{
    while(!bitcheck(REG32(UART_ADDR, UART_REG_LSR), 1)) {
        asm volatile("");
        // udelay(1000);
    }
    REG8(UART_ADDR, UART_REG_THR) = c;
}

// Flushes uart rxbuf
static void flush_input()
{
    while(bitcheck(REG32(UART_ADDR, UART_REG_LSR), 0))
        REG8(UART_ADDR, UART_REG_RBR);
}


// Flushes uart txbuf
static void flush_output()
{
    while(!bitcheck(REG32(UART_ADDR, UART_REG_LCR), 1))
        asm volatile("");
}


//////////////////////////////////////////////////////////////
// CRC Functions

const uint16_t crc16tab[256]= {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
	0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
	0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
	0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
	0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
	0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
	0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
	0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
	0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
	0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
	0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
	0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
	0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};
  
static uint16_t crc16_ccitt(const void *buf, register int len)
{
	register uint16_t crc = 0;
	while(len--)
        crc = ((crc<<8)&0xff00) ^ crc16tab[((crc>>8)&0xff) ^ (*(char *)buf++ & 0x00FF)];
    return crc;
}

//////////////////////////////////////////////////////////////
// Xmodem Functions

static int get_packet(uint8_t pkt_num, uint8_t *buf)
{
    uint8_t n = _inbyte(GETCHAR_TIMEOUT_MS);
    uint8_t ni = _inbyte(GETCHAR_TIMEOUT_MS);
    for(uint8_t i=0; i<XMODEM_PACKET_SZ; i++)
        buf[i] = _inbyte(GETCHAR_TIMEOUT_MS);

    uint16_t chk_got = ((_inbyte(GETCHAR_TIMEOUT_MS)) << 8)
                        | (_inbyte(GETCHAR_TIMEOUT_MS));
    uint16_t chk_calc = crc16_ccitt(buf, 128);
    
    // check crc
    if((n == pkt_num) && (ni == (255-n))
     && (chk_got == chk_calc))
        return SUCCESS;
    return FAILURE;
}

xmod_status xmodemReceive(uint8_t * bf, uint32_t len)
{
    // Clear recieve buffer
    flush_input();

    // Flush output buffer
    flush_output();

    // SPAM till SOH
    bool got_soh = false;
    clock_t t_end = cycle() + (((clock_t) XMODEM_WAIT_TIMEOUT_SEC)*((clock_t) CLK_FREQ));
    while (cycle() < t_end)
    {
        #ifndef SIM
            sleep_ms(1000);
        #endif
        _outbyte('C');
        int c = _inbyte(GETCHAR_TIMEOUT_MS);
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
        else {
           return UNKCHAR;   // unknown char
        }
    }

    if(!got_soh) {
        _outbyte(CAN);
        _outbyte(CAN);
        _outbyte(CAN);
        return TIMEOUT;   // connection timeout
    }


    int packetnum = 0;
    while(packetnum<len/128)
    {
        if(packetnum!=0)
        {
            // check for start
            int c = _inbyte(GETCHAR_TIMEOUT_MS);
            if(c==SOH){}
            else if (c==EOT) {
                _outbyte(ACK);
                return S_EOT;
            }
            else if(c==CAN) {
                _outbyte(ACK);
                return S_TERM;
            }
            else
                return UNKCHAR;

        }
        
        uint8_t *pkt_ptr = (bf+packetnum*128);

        // get packet
        xmod_status s = get_packet(packetnum+1, pkt_ptr);
        
        // acknowledge
        _outbyte(s==SUCCESS ? ACK : NAK);

        packetnum++;
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
            P(puts("Successful!");)
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