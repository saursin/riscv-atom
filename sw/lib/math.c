#include "math.h"
#include "stdio.h"

/**
 * @brief Computes square root of given number
 * @see http://f.javier.io/rep/books/The%20Elements%20of%20Computing%20Systems.pdf [pg-293]
 * @param x num
 * @return int sqrt
 */
int sqrt(int x)
{
    if(x < 0)
    {
        print_str("!EXCEPTION: Sqrt of -ve number undefined\n");
        asm("j _exit");
    }// Divide by zero exception

    int y = 0;
    for(int j=15; j>=0; j--)
        if((y + (1 << j)) * (y + (1 << j)) <= x)
            y = y + (1 << j);
    return y;
}


/**
 * @brief Returns |x|
 * 
 * @param x int
 * @return int absolute value
 */
int abs(int x)
{
    if(x < 0)
        return -x;
    else
        return x;
}


/**
 * @brief Returns lesser of two ints (if equal, returns num1)
 * 
 * @param x num1
 * @param y num2
 * @return int min(num1, num2)
 */
int min(int x, int y)
{
    if(x <= y)
        return x;
    else
        return y;
}


/**
 * @brief Returns greater of two ints (if euqal, returns num1)
 * 
 * @param x num1
 * @param y num2
 * @return int max(num1, num2)
 */
int max(int x, int y)
{
    if(x >= y)
        return x;
    else
        return y;
}
