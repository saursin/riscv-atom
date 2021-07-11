#include "../../lib/stdio.h"
#include "../../lib/stdlib.h"

void main()
{
    int i=0;
    for(i=0; i<10; i++)
    {   
        print_int(rand(), 10);
        print_chr('\n');
    }

    srand(0x50505);
    print_chr('\n');

    for(i=0; i<10; i++)
    {   
        print_int(rand(), 10);
        print_chr('\n');
    }

    return;
}