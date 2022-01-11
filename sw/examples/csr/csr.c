#include <stdio.h>
#include <time.h>

// #define SV \
// __asm__ volatile ("rdcycle t5");    \
// __asm__ volatile ("rdcycleh t6");   

// #define BRK \
// __asm__ volatile ("ebreak");



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
    clock_t start, end;

    start = clock();

    print_wall_pattern(20, 80);

    end = clock();


    puts("\n\n");
    printf("start: %d\n", start);
    printf("end  : %d\n", end);
    printf("-----------------------\n");
    printf("diff : %d\n", end-start);
    printf("-----------------------\n");
    printf("CPS  : %d\n", CLOCKS_PER_SEC);
    printf("Time  : %d\n", ((end - start)) / CLOCKS_PER_SEC);
}