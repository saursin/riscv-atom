#include <stdio.h>
#include <time.h>
#include <serial.h>
#include <stdbool.h>
#include <stdint.h>

// #define VUART

#ifdef ATOMSIM
#include <gpio.h>

bool LEDS[8] = {false, false, false, false, false, false, false, false};

void print_LEDS()
{
    puts("[");
    for(int i=7; i>=0; i--)
    {
        putchar(LEDS[i] ? '@': '-');
    }
    #ifdef VUART
    puts("]\r");
    #else
    puts("]\n");
    #endif
}
#endif


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
    //gpio_reset();
    static uint8_t prev_state = 0;
    static uint8_t curr_state = 0;

    for (int indx= reverse?len:0; reverse?(indx>=0):(indx<len); reverse ? indx--: indx++)
    {
        curr_state = pat[indx];
        
        for (uint8_t led=0; led<8; led++)
        {
            if(bitget(prev_state, led) != bitget(curr_state, led))
            {
                #ifdef ATOMSIM
                LEDS[led] = bitget(curr_state, led);
                #else
                gpio_set(led, bitget(curr_state, led));
                #endif
            }
        }
        sleep(delay);

        #ifdef ATOMSIM
        print_LEDS();
        #endif
        prev_state = curr_state;
    }
}



void main()
{
    int delay;

    #ifndef ATOMSIM
    serial_init(B_9600);
    gpio_init();
    delay = 500;
    #else
    delay = 0;
    #endif

    printf("\nPattern 1: Rolling\n");
    for(int i=0; i<4; i++)
        run_pattern(pat1, sizeof(pat1), 2*delay, false);
    
    printf("\nPattern 2: Zig Zag\n");
    for(int i=0; i<4; i++)
        run_pattern(pat2, sizeof(pat2), 2*delay, false);

    printf("\nPattern 3: Diverge\n");
    for(int i=0; i<4; i++)
        run_pattern(pat3, sizeof(pat3), 2*delay, false);

    printf("\nPattern 3: Converge\n");
    for(int i=0; i<4; i++)
        run_pattern(pat3, sizeof(pat3), 2*delay, true);
    
    printf("\nPattern 4: Sine\n");
    for(int i=0; i<8; i++)
        run_pattern(pat4, sizeof(pat4), delay, false);

    printf("\nPattern 5: Alternate\n");
    for(int i=0; i<4; i++)
        run_pattern(pat5, sizeof(pat5), 10*delay, false);

    return;
}
