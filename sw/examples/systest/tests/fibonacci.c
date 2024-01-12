#include <stdio.h>

void iterative_fibonacci(int arr[], int n){
    int pprev = 0;
    int prev = 1;
    int curr = 0;
    
    for (int i=0; i<n; i++)
    {
        if(i<2){
            arr[i] = i;
            continue;
        }
        
        curr = pprev + prev;
        pprev = prev;
        arr[i] = prev = curr;
    }
}

int _fib(int n){
    if(n < 2){
        return n;
    }
    return _fib(n - 1) + _fib(n - 2);
}

void recursive_fibonacci(int arr[], int n) {
    for(int i=0; i<n; i++){
        arr[i] = _fib(i);
    }
}

int test_fibonacci() {
    const int NUM = 20;
    int iter_fib[NUM];
    int recr_fib[NUM];

    // compute using loops
    iterative_fibonacci(iter_fib, NUM);
    puts("Using loop = ");
    print_arr(iter_fib, NUM);
    putchar('\n');

    // compute using recursion
    recursive_fibonacci(recr_fib, NUM);
    puts("Using recursion = ");
    print_arr(recr_fib, NUM);
    putchar('\n');

    for(int i=0; i<NUM; i++){
        if(iter_fib[i] != recr_fib[i]){
            return 1;
        }
    }
    return 0;
}