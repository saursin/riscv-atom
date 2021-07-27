#include "../../lib/atombones.h"
#include "../../lib/stdio.h"

int recursiveFactorial(int n)
{
    if(n >= 1)
        return n * recursiveFactorial(n-1);
    else
        return 1;
}

int linearFactorial(int n)
{
    int i, fact=1;
    for(i=1; i<= n; ++i)
    {
        fact *= i;
    }
    return fact;
}

int main()
{ 
    print_str("Calculating Factorials (iterative):\n");
    
    int i;
    for (i=0; i<10; i++)
    {
        print_int(i, 10); print_str("! = "); print_int(linearFactorial(i), 10); print_chr('\n');
    }

    print_str("\nCalculating Factorials (Recursive):\n");
    
    for (i=0; i<10; i++)
    {
        print_int(i, 10); print_str("! = "); print_int(recursiveFactorial(i), 10); print_chr('\n');
    }

}