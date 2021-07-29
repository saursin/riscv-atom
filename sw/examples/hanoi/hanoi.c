/*
 * C program for Tower of Hanoi using Recursion
 * @see https://www.sanfoundry.com/c-program-tower-of-hanoi-using-recursion/
 */
#include "../../lib/atombones.h"
#include "../../lib/stdio.h"

 
void towers(int, char, char, char);
 
int main()
{
    // Number of disks
    int num = 3;
      
    print_str("The sequence of moves involved in the Tower of Hanoi are :\n");
    towers(num, 'A', 'C', 'B');
    return 0;
}

void towers(int num, char frompeg, char topeg, char auxpeg)
{
    if (num == 1)
    {
        print_str(" Move disk 1 from peg "); print_chr(frompeg); print_str(" to peg "); 
        print_chr(topeg);print_str("\n");
        return;
    }
    towers(num - 1, frompeg, auxpeg, topeg);

    print_str(" Move disk "); print_int(num, 10); print_str(" from peg "); print_chr(frompeg); 
    print_str(" to peg ");print_chr(topeg); print_str("\n");

    towers(num - 1, auxpeg, topeg, frompeg);
}