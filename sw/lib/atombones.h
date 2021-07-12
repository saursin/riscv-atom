#ifndef __ATOMBONES_H__
#define __ATOMBONES_H__

// =================== MEMORY MAP ======================
#define MEM_ROM_BEGIN   0x00000000
#define MEM_ROM_SIZE    64*1024         // 64 KB

#define MEM_RAM_BEGIN   0x00010000
#define MEM_RAM_SIZE    16*1024         // 16 KB


// =========== IO Peripheral Register Addresses ========
#define IO_UART_RX_ADDRESS      0x08000000
#define IO_UART_TX_ADDRESS      0x08000001
#define IO_UART_SREG_ADDRESS    0x08000002

#endif //__ATOMBONES_H__