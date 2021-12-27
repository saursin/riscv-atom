/*
 * C program for Tower of Hanoi using Recursion
 * @see https://www.sanfoundry.com/c-program-tower-of-hanoi-using-recursion/
 */
#include <stdio.h>

 
void towers(int, char, char, char);
 
int main()
{
    // Number of disks
    int num = 3;
      
    printf("The sequence of moves involved in the Tower of Hanoi are :\n");
    towers(num, 'A', 'C', 'B');
    return 0;
}

void towers(int num, char frompeg, char topeg, char auxpeg)
{
    if (num == 1)
    {
        printf(" Move %c disk 1 from peg %c to peg %c \n", 'd', frompeg, topeg);
        return;
    }
    towers(num - 1, frompeg, auxpeg, topeg);

    printf(" Move disk %d from peg %c to peg %c \n", num, frompeg, topeg);

    towers(num - 1, auxpeg, topeg, frompeg);
}