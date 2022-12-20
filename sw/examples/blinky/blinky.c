#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <gpio.h>

uint8_t pat1 [] = 
{
    0b00010001,
    0b00100010,
    0b01000100,
    0b10001000,
    0b00010001,
    0b00100010,
    0b01000100,
    0b10001000
};

uint8_t pat2 [] = 
{
    0b00000001,
    0b00000010,
    0b00000100,
    0b00001000,
    0b10000000,
    0b01000000,
    0b00100000,
    0b00010000
};

uint8_t pat3 [] = 
{
    0b00011000,
    0b00100100,
    0b01000010,
    0b10000001,
    0b00000000
};

uint8_t pat4 [] = 
{
    0b01000000,
    0b00001000,
    0b00000010,
    0b00000001,
    0b00000010,
    0b00001000,
    0b01000000,
    0b10000000,
};

uint8_t pat5 [] = 
{
    0b01010101,
    0b10101010
};


bool bitget(uint8_t byte, uint8_t n)
{
    return (byte >> n) & 0x1;
}

void run_pattern(uint8_t * pat, int len, uint32_t delay, bool reverse)
{
    for (int indx= reverse?len:0; reverse?(indx>=0):(indx<len); reverse ? indx--: indx++)
    {
        gpio_writew(pat[indx]);
        sleep_ms(delay);
    }
}



void main()
{
    serial_init(UART_BAUD_115200);

    gpio_setmode(0, OUTPUT);
    gpio_setmode(1, OUTPUT);
    gpio_setmode(2, OUTPUT);
    gpio_setmode(3, OUTPUT);
    gpio_setmode(4, OUTPUT);
    gpio_setmode(5, OUTPUT);
    gpio_setmode(6, OUTPUT);
    gpio_setmode(7, OUTPUT);

    puts("Pattern 1: Rolling\n");
    for(int i=0; i<4; i++)
        run_pattern(pat1, sizeof(pat1), 200, false);
    
    puts("Pattern 2: Zig Zag\n");
    for(int i=0; i<4; i++)
        run_pattern(pat2, sizeof(pat2), 200, false);

    puts("Pattern 3: Diverge\n");
    for(int i=0; i<4; i++)
        run_pattern(pat3, sizeof(pat3), 200, false);

    puts("Pattern 3: Converge\n");
    for(int i=0; i<4; i++)
        run_pattern(pat3, sizeof(pat3), 200, true);
    
    puts("Pattern 4: Sine\n");
    for(int i=0; i<8; i++)
        run_pattern(pat4, sizeof(pat4), 200, false);

    puts("Pattern 5: Alternate\n");
    for(int i=0; i<4; i++)
        run_pattern(pat5, sizeof(pat5), 200, false);

    gpio_reset();
    return;
}
