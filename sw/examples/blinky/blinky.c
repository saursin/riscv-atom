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
    0b00010000,
    0b01000000,
    0b10000000
};

uint8_t pat5 [] = 
{
    0b01010101,
    0b10101010
};


uint8_t pat6 [] = 
{
    0b00000001,
    0b00000100,
    0b00000010,
    0b00001000,
    0b00000100,
    0b00010000,
    0b00001000,
    0b00100000,
    0b00010000,
    0b01000000,
    0b00100000,
    0b10000000
};

uint8_t pat7 [] = 
{
    0b00000001,
    0b00000011,
    0b00000111,
    0b00001110,
    0b00011100,
    0b00111000,
    0b01110000,
    0b11100000,
    0b11000000,
    0b10000000,
    0b00000000
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

void soft_pwm(uint8_t dutycycle, uint32_t duration)
{
    clock_t tend = cycle() + (duration * (CLK_FREQ/1000));
    while(cycle() < tend) {
        gpio_writew(0x1ff);
        for(uint16_t i=0; i<255; i++)
        {
            if(i>dutycycle)
                gpio_writew(0x00);
        }
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

    while(1) {
        puts("Pattern 1: Rolling\n");
        for(int i=0; i<8; i++)
            run_pattern(pat1, sizeof(pat1), 100, false);
        
        puts("Pattern 2: Zig Zag\n");
        for(int i=0; i<8; i++)
            run_pattern(pat2, sizeof(pat2), 100, false);

        puts("Pattern 3: Diverge\n");
        for(int i=0; i<8; i++)
            run_pattern(pat3, sizeof(pat3), 100, false);

        puts("Pattern 3: Converge\n");
        for(int i=0; i<8; i++)
            run_pattern(pat3, sizeof(pat3), 100, true);
        
        puts("Pattern 4: Sine\n");
        for(int i=0; i<16; i++)
            run_pattern(pat4, sizeof(pat4), 100, false);

        puts("Pattern 5: Alternate\n");
        for(int i=0; i<10; i++)
            run_pattern(pat5, sizeof(pat5), 100, false);
        
        puts("Pattern 6: 2 step forward, 1 step backward\n");
        for(int i=0; i<10; i++)
            run_pattern(pat6, sizeof(pat6), 100, false);
        
        puts("Pattern 7: loading\n");
        for(int i=0; i<10; i++)
            run_pattern(pat7, sizeof(pat7), 100, false);
        

        puts("Pattern 8: Soft PWM\n");
        for(int i=0; i<5; i++){
            for(int d=0; d<255; d+=5)
                soft_pwm(d, 30);
            for(int d=255; d>=0; d-=5)
                soft_pwm(d, 30);
        }
    }
    return;
}
