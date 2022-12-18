#include "platform.h"
#include <stdint.h>
#include "gpio.h"
#include "serial.h"
#include "time.h"
#include "mmio.h"

extern uint32_t __approm_start;
extern uint32_t __approm_size;

typedef void (*fnc_ptr)(void);

// Get kth bit of number x
#define _spi_bitget(x, k)  ((x & (0x1 << k))>0)

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

void puthex(unsigned int val, int digits)
{
	for (int i = (4*digits)-4; i >= 0; i -= 4)
        putchar("0123456789abcdef"[(val >> i) % 16]);
}

void dumphexbuf(uint8_t *buf, unsigned len, unsigned base_addr)
{
    const int bpw = 4; // bytes per word
    const int wpl = 4; // words per line

    for (unsigned i=0; i<len; i++) {
        // print address at the start of the line
        if(i%(wpl*bpw) == 0) {
            puthex(base_addr+i, 8); puts(": ");
        }
        
        // print byte
        puthex(0xff & buf[i], 2); putchar(' ');
        
        // extra space at word boundry
        if(i%bpw == bpw-1)
            putchar(' ');
        
        if(i%(wpl*bpw) == (wpl*bpw-1)) // end of line 
            putchar('\n');
    }
    putchar('\n');
}

//********************** Bitbang SPI **********************
// Timing
#define T_WAIT_AFTER_CS_LOW 0
#define T_WAIT_BEFORE_CS_HIGH 0
#define T_CLK_HIGH 0
#define T_CLK_LOW 0
#define T_WAIT_AFTER_BYTE 0

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

void spi_select(struct SPI_Config * cfg)
{
	gpio_write(cfg->cs_pin, LOW);
	sleep_us(T_WAIT_AFTER_CS_LOW);
}

void spi_deselect(struct SPI_Config * cfg)
{
	sleep_us(T_WAIT_BEFORE_CS_HIGH);
    gpio_write(cfg->cs_pin, HIGH);
}

char spi_transfer(struct SPI_Config * cfg, char b)
{
	char r = 0;
	for(int i=7; i>=0; i--)
	{        
        // Falling edge of clock : shift out new data on MOSI
        gpio_write(cfg->sck_pin, LOW);
		gpio_write(cfg->mosi_pin, _spi_bitget(b, i) ? HIGH : LOW);
		sleep_us(T_CLK_LOW);

        // read available data on MISO
		r = ((r << 1) | gpio_read(cfg->miso_pin));

        // Rising edge of clock
		gpio_write(cfg->sck_pin, HIGH);
		sleep_us(T_CLK_HIGH);
	}
	return r;
}

char *spi_transfer_buf(struct SPI_Config * cfg, char *send_buf, char *recv_buf, unsigned int len)
{
    for(unsigned int i=0; i<len; i++)
        *send_buf++ = spi_transfer(cfg, *recv_buf++);
    return recv_buf;
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
    spi_select(cfg);
    spi_transfer(cfg, FLASH_CMD_READ);
    spi_transfer(cfg, addr >> 16);
	spi_transfer(cfg, addr >> 8);
	spi_transfer(cfg, addr);
    for(uint32_t i=0; i<len; i++)
		*(buf++) = spi_transfer(cfg, 0x00);
	spi_deselect(cfg);
	return rval;
}


//***************************************************
int main()
{
    // Initialize
    const int led_pin = 0;
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
    gpio_setmode(led_pin, OUTPUT);


    // Initialize UART
    REG32(UART_ADDR, UART_REG_DIV) = 102;   
    uint32_t lcr = 0;
    lcr |= UART_REG_LCR_RXEN;
    lcr |= UART_REG_LCR_TXEN;
    REG32(UART_ADDR, UART_REG_LCR) = lcr;
    // clear any garbage from data register by reading it once
    REG32(UART_ADDR, UART_REG_RBR);

    // putchar(0x1b); putchar('c');  // clear screen
    puts("*** FlashBoot ***\n");
    puts("Reading... ");
    // Blink thrice (indicates start of copy operation)
    led_blink(led_pin, 3, 50);

    // Read & Copy
    const uint32_t flash_start_addr = 0x00000000;
    // flash_read(&spi_cfg, img, flash_start_addr, 1024);
    flash_read(&spi_cfg, (uint8_t *)&__approm_start, flash_start_addr, 12*1024);

    // Blink single (indicates finish of copy operation)
    puts("done\n");
    led_blink(led_pin, 1, 500);
    sleep_ms(1000);

    // dumphexbuf((uint8_t *)&__approm_start, 1024, 0x000000);
    // dumphexbuf(img, 1024, 0x000000);
    
    // Jump to user application
    puts("Jumping to user application\n-------------------------------------------------------\n");
    fnc_ptr app_main = (fnc_ptr)(&__approm_start);
    app_main();

    // UNREACHABLE
    gpio_setmode(led_pin, OUTPUT);
    while(1){
        gpio_write(led_pin, HIGH);
        puts("FLASHBOOT_ERR: UNREACHABLE\n");
    }
}