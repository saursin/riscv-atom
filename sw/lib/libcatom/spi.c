#include <time.h>  // for sleep()
#include <gpio.h>

#include "spi.h"

// GPIO Pins
int SPI_MOSI = 1;
int SPI_MISO = 2;
int SPI_SCK = 3;

// NOTE: Any GPIO pin can be used as SPI_CHIP_SELECT, its user's 
// responsibility to prperly set SPI_CS pin before and after calling 
// transfer functions

////////////////////////////////////////////////////////////////////////
// Timing
#define T_CLK_HIGH 1
#define T_CLK_LOW 1
#define T_WAIT_AFTER_BYTE 1

///////////////////////////////////////////////////////////////
// Utility Functions

// Get kth bit of number x
int _spi_bitget(int x, int k)
{
	return (x & (0x1 << k))>0;
}

//////////////////////////////////////////////////////////////
// API Functions
void spi_init()
{
    // Set PinModes
	gpio_setmode(SPI_MOSI, OUTPUT);
	gpio_setmode(SPI_MISO, INPUT);
	gpio_setmode(SPI_SCK, OUTPUT);
	
	// Write init states
	gpio_write(SPI_MOSI, LOW);
	gpio_write(SPI_SCK, LOW);
}


char spi_transfer(char b)
{
	char r = 0;
	for(int i=7; i>=0; i--)
	{        
        // Falling edge of clock : shift out new data on MOSI
        gpio_write(SPI_SCK, LOW);
		gpio_write(SPI_MOSI, _spi_bitget(b, i) ? HIGH : LOW);
		sleep(T_CLK_LOW);

        // read available data on MISO
		r = ((r << 1) | gpio_read(SPI_MISO));

        // Rising edge of clock
		gpio_write(SPI_SCK, HIGH);
		sleep(T_CLK_HIGH);
	}
	return r;
}


char *spi_transfer_buf(char *send_buf, char *recv_buf, unsigned int len)
{
    for(unsigned int i=0; i<len; i++)
    {
        *send_buf++ = spi_transfer(*recv_buf++);
    }
    return recv_buf;
}