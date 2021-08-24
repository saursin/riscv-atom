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
        {
            continue;
        }

        *((volatile char*) UART_D_REG_ADDR) = c;
        break;
    }
}

void initUART()
{
    *((volatile int*) UART_CD_REG_ADDR) = 1248;
}

void print(const char *p)
{
	while (*p)
    {
		sendChar(*(p++));
        delay(1);
    }
}

char * banner = 
"\n"
"                  .';,.           ....;;;.  \n"
"                 .ll,:o,                ':c,. \n"
"                 .dd;co'                  .cl,  \n"
"                .:o:;,.                     'o:  \n"
"                co.                          .oc  \n"
"               ,o'          .coddoc.          'd,  \n"
"               lc         .lXMMMMMMXl.         ll  \n"
"              .o:         ;KMMMMMMMMK,         :o. \n"
"              .o:         'OMMMMMMMMO.         :o. \n"
"               co.         .o0XNNX0o.         .oc  \n"
"               .o:           ..''..           :o.  \n"
"                'o:                          :o'  \n"
"                 .lc.                      .ll.  \n"
"                   ,lc'                  'cl,   \n"
"                     'cc:,..        ..,:c:'   \n"
"                        .;::::;;;;::::;.    \n"
"                              ....        \n"
"     ____  _________ _______    __         __                 \n"
"    / __ \\/  _/ ___// ____/ |  / /  ____ _/ /_____  ____ ___  \n"
"   / /_/ // / \\__ \\/ /    | | / /  / __ `/ __/ __ \\/ __ `__ \\ \n"
"  / _, _// / ___/ / /___  | |/ /  / /_/ / /_/ /_/ / / / / / /      \n"
" /_/ |_/___//____/\\____/  |___/   \\__,_/\\__/\\____/_/ /_/ /_/  \n"
"/=========By: Saurabh Singh (saurabh.s99100@gmail.com)====/\n\n";



void main()
{
    initUART();
    gpio_init();
    const char hello[] = "Hello RISC-V!\n";

    while(1)
    {   
        gpio_reset();
        delay(20);
        gpio_set(0, GPIO_HIGH);
        print(banner);
        gpio_set(1, GPIO_HIGH);
        delay(10);
        gpio_set(2, GPIO_HIGH);
        print(hello);
        gpio_set(3, GPIO_HIGH);
        delay(10);
    }
    return;
}
