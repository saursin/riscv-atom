#include "../../lib/stdio.h"

int main()
{
    int no_of_terrms = 5, i;
    int pprev = 0;
    int prev = 1;
    int curr = 0;

    putstring("Fibonacci sequence upto 5 digits: \n");

    for (i=0; i<no_of_terrms; i++)
    {
        curr = pprev + prev;
        pprev = prev;
        prev = curr;
        putchar((char)curr+48);
        putchar('\n');
    }
}