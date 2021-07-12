#include "../../lib/atombones.h"
#include "../../lib/stdio.h"

void printFibonacci(int n){    
    static int pprev=0, prev=1, curr;    
    if(n>0){    
        curr = prev + pprev;
        pprev = prev;
        prev = curr;
        print_int(curr, 10);
        print_chr('\n');
        printFibonacci(n-1);    
    }    
}    

int main()
{ 
    int no_of_terms = 32, i;
    int pprev = 0;
    int prev = 1;
    int curr = 0;

    print_str("First ");
    print_int(no_of_terms, 10);
    print_str(" terms of Fibonacci Series (Using Loops):\n");  

    for (i=0; i<no_of_terms; i++)
    {

        curr = pprev + prev;
        pprev = prev;
        prev = curr;
        print_int(curr, 10);

        print_chr('\n');
    }

    print_str("\n\nFirst ");
    print_int(no_of_terms, 10);
    print_str(" terms of Fibonacci Series (Using Recursion):\n");
    printFibonacci(no_of_terms);
}