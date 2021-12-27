#include <stdio.h>

int recursiveFactorial(int n)
{
    if(n >= 1)
        return n * recursiveFactorial(n-1);
    else
        return 1;
}

int iterativeFactorial(int n)
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
    printf("Calculating Factorials (iterative):\n");
    
    int i;
    for (i=0; i<10; i++)
    {
        printf("%d! = %d\n", i, iterativeFactorial(i));
    }

    printf("\nCalculating Factorials (Recursive):\n");
    
    for (i=0; i<10; i++)
    {
        printf("%d! = %d\n", i, recursiveFactorial(i));
    }

}