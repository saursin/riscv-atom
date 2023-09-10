#include "platform.h"
#include <stdint.h>
#include "gpio.h"
#include "serial.h"
#include "time.h"
#include "mmio.h"
#include "spi.h"

//--------------------- Settings -----------------------------
// Address offset at which image resides
#define FLASH_IMG_OFFSET    0x000c0000

// Size of Image
#define FLASH_IMG_SIZE      28 * 1024   // 28 KB

// Enable/Disable UART
#define ENABLE_UART

// number of flashes to indicate entering of bootloader
#define START_LED_FLASHES   3

// number of flashes to indicate entering of user application
#define END_LED_FLASHES   1

// Clear screen at start of bootloader
#define CLS_AT_START

//------------------------------------------------------------
#define STRINGIFY(s) #s
#define EXPAND_AND_STRINGIFY(s) STRINGIFY(s)

#ifdef ENABLE_UART
#define D(x) x
#else
#define D(x)
#endif

extern uint32_t __approm_start;
extern uint32_t __approm_size;

typedef void (*fnc_ptr)(void);

//********************** Tiny STDIO **********************
#ifdef ENABLE_UART
void putchar(char c)
{
    if(c == '\n')
        serial_write('\r');
    serial_write(c);
}

void puts(char *ptr)
{
    while (*ptr)
        putchar(*ptr++);
}
#endif

// ********************** LED **********************
void led_blink(int pin, int count, int delay)
{
    for(int i=0; i<count; i++) 
    {
        gpio_write(pin, HIGH);
        sleep_ms(delay);
        gpio_write(pin, LOW);
        sleep_ms(delay);
    }
}

//********************** FLASH **********************
#define FLASH_CMD_READ 0x03
#define SPI_REG_SCKDIV_OFFSET   0x00
#define SPI_REG_SCTRL_OFFSET    0x04
#define SPI_REG_TDATA_OFFSET    0x08
#define SPI_REG_RDATA_OFFSET    0x0c
#define SPI_REG_CSCTRL_OFFSET   0x10
#define SPI_REG_DCTRL_OFFSET    0x14

void flash_copy(uint8_t *buf, uint32_t addr, uint32_t len)
{
    struct SPI_Config cfg = {
        .base_addr = SPI_ADDR,
        .enable = true,
        .pha = false,
        .pol = false,
        .lsb_first = false,
        .baudrate = 1000000,
        .device_num = 0,
        .cs_mode = CSMODE_DISABLE,
        .post_cs_low_delay = 1,
        .pre_cs_high_delay = 1,
        .loopback_enable=false
    };
    spi_init(&cfg);

    // select
    REG32(cfg.base_addr, SPI_REG_CSCTRL_OFFSET) = bitset(REG32(cfg.base_addr, SPI_REG_CSCTRL_OFFSET), 24+cfg.device_num);
    spi_transfer(&cfg, FLASH_CMD_READ);
    spi_transfer(&cfg, addr >> 16);
	spi_transfer(&cfg, addr >> 8);
	spi_transfer(&cfg, addr);

    while(len--) {
        REG8(cfg.base_addr, SPI_REG_TDATA_OFFSET) = 0x0;
        while(REG32(SPI_ADDR, SPI_REG_SCTRL_OFFSET) >> 31)
            asm volatile("");
        *(buf++) = REG8(cfg.base_addr, SPI_REG_RDATA_OFFSET);
    }

    // deselect
    REG32(cfg.base_addr, SPI_REG_CSCTRL_OFFSET) = bitclear(REG32(cfg.base_addr, SPI_REG_CSCTRL_OFFSET), 24+cfg.device_num);
    cfg.cs_mode = CSMODE_AUTO;
    spi_init(&cfg);
}

int main()
{
    // ********** Initialize **********
    // Initialize status led gpio pin
    const int led_pin = 0;
    gpio_setmode(led_pin, OUTPUT);
    led_blink(led_pin, START_LED_FLASHES, 50);      // Blink LED START_LED_FLASHES times (signal entering of bootloader)

    #ifdef ENABLE_UART
    // Initialize UART
    REG32(UART_ADDR, UART_REG_DIV) = 102;   
    uint32_t lcr = 0;
    lcr |= UART_REG_LCR_RXEN;
    lcr |= UART_REG_LCR_TXEN;
    REG32(UART_ADDR, UART_REG_LCR) = lcr;
    // clear any garbage from data register by reading it once
    REG32(UART_ADDR, UART_REG_RBR);
    #endif

    // Print header
    #ifdef CLS_AT_START
    D(putchar(0x1b); putchar('c');)  // clear screen
    #endif
    D(puts("***** FlashBoot *****\n");)

    // ********** Read & Copy **********
    D(puts("Copying from " EXPAND_AND_STRINGIFY(FLASH_IMG_OFFSET));)
    flash_copy((uint8_t *)&__approm_start, FLASH_IMG_OFFSET, FLASH_IMG_SIZE);

    // ********** Jump to user application **********
    led_blink(led_pin, END_LED_FLASHES, 50);        // Blink single (signal jump to user code)
       
    D(puts("Jumping to user code\n");)
    D(puts("---------------------\n");)
    fnc_ptr app_main = (fnc_ptr)(&__approm_start);
    app_main();

    // ** UNREACHABLE **
    D(puts("Err: unreachable\n");)
    while(1) {
        asm volatile("");
    }
}