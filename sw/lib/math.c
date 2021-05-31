#include "math.h"
#include "stdio.h"

/**
 * @brief Computes multiplication of two ints
 * @see http://f.javier.io/rep/books/The%20Elements%20of%20Computing%20Systems.pdf [pg-292]
 * @param x num1
 * @param y num2
 * @return int product
 */
int __mulsi3(int x, int y)
{
    // Shift and Add Multiplication
    int sum = 0;

    for(int j=0; j<32; j++)
    {
        if((0x00000001 << j) & y)
            sum += x;
        x = x << 1;
    }
}


/**
 * @brief Computes division of two ints
 * @see http://f.javier.io/rep/books/The%20Elements%20of%20Computing%20Systems.pdf [pg-292]
 * @param x dividend
 * @param y divisor
 * @return int quotient
 */
int __divsi3(int x, int y)
{
    if(y == 0)
    {
        print_str("!EXCEPTION: Divide by zero\n");
        _asm("j _exit");
    }// Divide by zero exception
    
    if(y > x)
        return 0;

    int q = _divsi3(x, y << 1);
    if((x - (__mulsi3(q, y) << 1)) < y)
        return q << 1;
    else
        return (q << 1) + 1;
}


/**
 * @brief Computes remainder of division of two ints
 * 
 * @param x dividend
 * @param y divisor
 * @return int remainder
 */
int __modsi3(int x, int y)
{
    if(y == 0)
    {
        print_str("!EXCEPTION: Divide by zero\n");
        _asm("j _exit");
    }// Divide by zero exception

    int i=0;
    while (x>y)
    {
        x = x-y;
        i++;
    }
    return x;
}


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
        exit(1);
    }// Divide by zero exception

    int y = 0;
    for(int j=15; j>=0; j--)
        if(__mulsi3((y + (1 << j)), (y + (1 << j))) <= x)
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
