// #include <time.h>  // for sleep()
#include "gpio.h"
#include "spi_bitbang.h"

#define _spi_bitget(x, k)  ((x & (0x1 << k))>0)

void spib_init(struct SPIB_Config * cfg)
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


char spib_transfer(struct SPIB_Config * cfg, char b)
{
	char r = 0;
	for(int i=7; i>=0; i--)
	{        
        // Falling edge of clock : shift out new data on MOSI
		// Potential-Performance upgrade: do these operations in one function call
        gpio_write(cfg->sck_pin, LOW);
		gpio_write(cfg->mosi_pin, _spi_bitget(b, i) ? HIGH : LOW);
		
        // read available data on MISO
		r = ((r << 1) | gpio_read(cfg->miso_pin));

        // Rising edge of clock
		gpio_write(cfg->sck_pin, HIGH);
	}
	return r;
}


char *spib_transfer_buf(struct SPIB_Config * cfg, char *send_buf, char *recv_buf, unsigned int len)
{
	if(cfg->auto_cs)
		gpio_write(cfg->cs_pin, LOW);

    for(unsigned int i=0; i<len; i++) {
        *send_buf++ = spib_transfer(cfg, *recv_buf++);
	}
	
	if(cfg->auto_cs)
		gpio_write(cfg->cs_pin, HIGH);
		
	return recv_buf;
}


void spib_start(struct SPIB_Config * cfg)
{
	gpio_write(cfg->cs_pin, LOW);
}


void spib_end(struct SPIB_Config * cfg)
{
	gpio_write(cfg->cs_pin, HIGH);
}