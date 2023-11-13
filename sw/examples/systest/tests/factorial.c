void iterative_factorial(int arr[], int N)
{
    for(int n=0; n<N; n++){
        int i, fact=1;
        for(i=1; i<= n; ++i)
        {
            fact *= i;
        }
        arr[n] = fact;
    }
}

int _fact(int n){
    if(n >= 1)
        return n * _fact(n-1);
    else
        return 1;
}

void recursive_factorial(int arr[], int n) {
    for(int i=0; i<n; i++){
        arr[i] = _fact(i);
    }
}

int test_factorial(){
    int NUM = 20;
    int iter_fact[NUM];
    int recr_fact[NUM];

    // compute using loops
    iterative_factorial(iter_fact, NUM);
    puts("Using loop = ");
    print_arr(iter_fact, NUM);
    putchar('\n');

    // compute using recursion
    recursive_factorial(recr_fact, NUM);
    puts("Using recursion = ");
    print_arr(recr_fact, NUM);
    putchar('\n');

    for(int i=0; i<NUM; i++){
        if(iter_fact[i] != recr_fact[i]){
            return 1;
        }
    }
    return 0;
}