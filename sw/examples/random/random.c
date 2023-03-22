#include <stdio.h>
#include <stdlib.h>

void main()
{
    serial_init(UART_BAUD_115200);

    printf("Printing 10 Random numbers using default seed\n");
    int i=0;
    for(i=0; i<10; i++)
    {   
        printf("\t%d\n", rand());
    }

    srand(0x50505);
    printf("\nPrinting 10 Random numbers using seed %d: \n", 0x50505);

    for(i=0; i<10; i++)
    {   
        printf("\t%d\n", rand());
    }

    return;
}