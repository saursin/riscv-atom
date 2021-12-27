#include <stdio.h>

void printFibonacci(int n){    
    static int pprev=0, prev=1, curr;    
    if(n>0){    
        curr = prev + pprev;
        pprev = prev;
        prev = curr;
        putint(curr, 10);
        putchar('\n');
        printFibonacci(n-1);    
    }    
}    

int main()
{ 
    int no_of_terms = 32, i;
    int pprev = 0;
    int prev = 1;
    int curr = 0;

    printf("First %d terms of Fibonacci Series (Using Loops):\n", no_of_terms);

    for (i=0; i<no_of_terms; i++)
    {
        curr = pprev + prev;
        pprev = prev;
        prev = curr;
        putint(curr, 10);
        putchar('\n');
    }

    printf("\n\nFirst %d terms of Fibonacci Series (Using Recursion):\n", no_of_terms);
    printFibonacci(no_of_terms);
}