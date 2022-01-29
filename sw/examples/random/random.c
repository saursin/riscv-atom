#include <stdio.h>
#include <stdlib.h>

void main()
{
    int i=0;
    for(i=0; i<10; i++)
    {   
        putint(rand(), 10, false);
        putchar('\n');
    }

    srand(0x50505);
    putchar('\n');

    for(i=0; i<10; i++)
    {   
        putint(rand(), 10, false);
        putchar('\n');
    }

    return;
}