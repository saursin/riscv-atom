#include "platform.h"
#include <stdint.h>
#include "gpio.h"
#include "serial.h"
#include "time.h"
#include "mmio.h"

//--------------------- Settings -----------------------------
// Address offset at which image resides
#define FLASH_IMG_OFFSET    0x000c0000

// Size of Image
#define FLASH_IMG_SIZE      12 * 1024   // 12 KB

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


#ifdef ENABLE_UART
//********************** Tiny STDIO **********************
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


//********************** Bitbang SPI **********************
// Get kth bit of number x
#define _spi_bitget(x, k)  ((x & (0x1 << k))>0)

// Timing
struct SPI_Config {
    uint8_t cs_pin;
    uint8_t sck_pin;
    uint8_t miso_pin;
    uint8_t mosi_pin;
};

void spi_init(struct SPI_Config * cfg)
{
    // Set PinModes
    gpio_setmode(cfg->cs_pin, OUTPUT);
    gpio_setmode(cfg->sck_pin, OUTPUT);
	gpio_setmode(cfg->mosi_pin, OUTPUT);
	gpio_setmode(cfg->miso_pin, INPUT);

    // Write init states
    gpio_write(cfg->cs_pin, HIGH);
    gpio_write(cfg->sck_pin, LOW);
	gpio_write(cfg->mosi_pin, LOW);
}

char spi_transfer(struct SPI_Config * cfg, char b)
{
	char r = 0;
	for(int i=7; i>=0; i--)
	{        
        // Falling edge of clock : shift out new data on MOSI
        gpio_write(cfg->sck_pin, LOW);
		gpio_write(cfg->mosi_pin, _spi_bitget(b, i) ? HIGH : LOW);

        // read available data on MISO
		r = ((r << 1) | gpio_read(cfg->miso_pin));

        // Rising edge of clock
		gpio_write(cfg->sck_pin, HIGH);
	}
	return r;
}


//********************** LED **********************
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

uint8_t *flash_read(struct SPI_Config * cfg, uint8_t *buf, uint32_t addr, uint32_t len)
{
	uint8_t *rval = buf;

    // Deassert CS (Start Txn)
	gpio_write(cfg->cs_pin, LOW);

    // Send CMD
    spi_transfer(cfg, FLASH_CMD_READ);

    // Send Addr
    spi_transfer(cfg, addr >> 16);
	spi_transfer(cfg, addr >> 8);
	spi_transfer(cfg, addr);

    // Get Data
    for(uint32_t i=0; i<len; i++)
		*(buf++) = spi_transfer(cfg, 0x00);
	
    // Assert CS (Finish Txn)
    gpio_write(cfg->cs_pin, HIGH);
	return rval;
}


//***************************************************
int main()
{
    // ********** Initialize **********
    // Initialize status led gpio pin
    const int led_pin = 0;
    gpio_setmode(led_pin, OUTPUT);
    led_blink(led_pin, START_LED_FLASHES, 50);      // Blink LED START_LED_FLASHES times (signal entering of bootloader)

    // Initialize SPI
    /* struct SPI_Config spi_cfg = {    // doesn't work
        .cs_pin = 4,
        .sck_pin = 7,
        .mosi_pin = 5,
        .miso_pin = 6
    }; */

    struct SPI_Config spi_cfg;
    spi_cfg.cs_pin = 12;
    spi_cfg.sck_pin = 15;
    spi_cfg.mosi_pin = 13;
    spi_cfg.miso_pin = 14;

    spi_init(&spi_cfg);

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
    D(puts("********** FlashBoot **********\n");)


    // ********** Read & Copy **********
    D(puts("Reading from " EXPAND_AND_STRINGIFY(FLASH_IMG_OFFSET) " - ");)
    flash_read(&spi_cfg, (uint8_t *)&__approm_start, FLASH_IMG_OFFSET, FLASH_IMG_SIZE);
    D(puts("Done!\n");)


    // ********** Jump to user application **********
    led_blink(led_pin, END_LED_FLASHES, 50);                     // Blink single (signal jump to user code)
    D(puts("Jumping to user application\n------------------------------\n"););
    fnc_ptr app_main = (fnc_ptr)(&__approm_start);
    app_main();


    // ** UNREACHABLE **
    gpio_setmode(led_pin, OUTPUT);
    while(1){
        gpio_write(led_pin, HIGH);
        D(puts("FLASHBOOT_ERR: UNREACHABLE\n");)
        sleep_ms(100);
    }
}