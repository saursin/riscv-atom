#include "../../lib/atombones.h"
#include "../../lib/gpio.h"

#define UART_D_REG_ADDR 0x08000000
#define UART_S_REG_ADDR 0x08000001
#define UART_CD_REG_ADDR 0x08000004

#define DELAY_SCALING_FACTOR 10000
void delay(long count)
{
    long i=0;
    while(i++<count*DELAY_SCALING_FACTOR);
    return;
}

void sendChar(char c)
{
    while(1)
    {
        // check if Tx is busy
        if((*((volatile char*) UART_S_REG_ADDR) & 0x02) >> 1)
            continue;

        *((volatile char*) UART_D_REG_ADDR) = c;
        break;
    }
}


void initUART()
{
    *((volatile int*) UART_CD_REG_ADDR) = 1248;//= 1
}

void print(const char *p)
{
	while (*p)
    {
		sendChar(*(p++));
        delay(1);
    }
}

void main()
{
    initUART();
    gpio_init();
    while(1)
    {   
        gpio_reset();
        delay(10);
        gpio_set(0, GPIO_HIGH);
        delay(10);
        gpio_set(1, GPIO_HIGH);
        delay(10);
        gpio_set(2, GPIO_HIGH);
        delay(10);
        gpio_set(3, GPIO_HIGH);
        delay(10);
        gpio_set(4, GPIO_HIGH);
        delay(10);
        gpio_set(5, GPIO_HIGH);
        delay(10);
        gpio_set(6, GPIO_HIGH);
        delay(10);
        gpio_set(7, GPIO_HIGH);
        delay(10);
        
        const char hello[] = "Hello RISC-V!\n";
        print(hello);
        delay(5);
    }
    return;
}
