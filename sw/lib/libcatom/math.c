#include <stdio.h>
#include <math.h>

// see http://f.javier.io/rep/books/The%20Elements%20of%20Computing%20Systems.pdf [pg-293]
int sqrt(int x)
{
    if(x < 0)
    {
        puts("!EXCEPTION: Sqrt of -ve number undefined\n");
        asm("j _exit");
    }// Divide by zero exception

    int y = 0;
    for(int j=15; j>=0; j--)
        if((y + (1 << j)) * (y + (1 << j)) <= x)
            y = y + (1 << j);
    return y;
}


int abs(int x)
{
    return (x < 0) ? -x : x;
}


int min(int x, int y)
{
    return (x <= y) ? x : y;
}


int max(int x, int y)
{
    return (x >= y) ? x : y;
}

float pow(float x, int y)
{
    float temp;
    if( y == 0)
       return 1;
    temp = pow(x, y/2);       
    if (y%2 == 0)
        return temp*temp;
    else
    {
        if(y > 0)
            return x*temp*temp;
        else
            return (temp*temp)/x;
    }
}  