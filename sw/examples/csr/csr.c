#include <platform.h>
#include <stdio.h>
#include <serial.h>
#include <time.h>

#define SV \
__asm__ volatile ("rdcycle t5");    \
__asm__ volatile ("rdcycleh t6");   

#define BRK \
__asm__ volatile ("ebreak");

void print_wall_pattern(int height, int width)
{
    for(int j=0; j<height; j++)
    {
        for(int i=0; i<width; i++)
        {
            if(j%2!=0)  // odd
            {
                if(i%4==0)
                    putchar('|');
                else
                    putchar('_');
            }
            else
            {
                if((i+2)%4==0)
                    putchar('|');
                else
                    putchar('_');
            }
        }
        putchar('\n');
    }
}


int main()
{
    serial_init(UART_BAUD_115200);

    clock_t start, end;

    start = cycle();
    print_wall_pattern(20, 80);
    end = cycle();

    puts("\n\n");
    printf("start: %lld\n", start);
    printf("end  : %lld\n", end);
    puts("-----------------------\n");
    printf("diff : %lld\n", end-start);
    puts("-----------------------\n");
    printf("CPS  : %lld\n", (long long) CLK_FREQ);
    return 0;
}