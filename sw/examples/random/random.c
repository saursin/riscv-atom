#include <stdio.h>
#include <stdlib.h>

void main()
{
    printf("Printing 10 Random numbers using seed %d: \n", rand_seed);
    int i=0;
    for(i=0; i<10; i++)
    {   
        putchar('\t');
        putint(rand(), 10, false);
        putchar('\n');
    }

    srand(0x50505);
    printf("\nPrinting 10 Random numbers using seed %d: \n", 0x50505);

    for(i=0; i<10; i++)
    {   
        putchar('\t');
        putint(rand(), 10, false);
        putchar('\n');
    }

    return;
}