#include <stdlib.h>
#include <stdio.h>

void exit(const int status) {
    while(1) {
        asm volatile("mv a0, %[retcode];" : : [retcode] "r" (status));
        asm volatile("j _exit");
    }
}

void abort() {
    puts("Abort called...\n");
    exit(EXIT_FAILURE);
}


int atoi(char *s1)
{
    int ret,sig;
    
    for(sig=ret=0;s1&&*s1;s1++) 
    {
        if(*s1=='-') 
            sig=1;
        else 
            ret = *s1-'0'+(ret<<3)+(ret<<1); // val = val*10+int(*s1)
    }
    
    return sig ? -ret : ret;
}


int xtoi(char *s1)
{
    int ret;
    
    for(ret=0;s1&&*s1;s1++) 
    {
        if(*s1<='9')
            ret = *s1-'0'+(ret<<4); // val = val*16+int(*s1)
        else
            ret = 10+(*s1&0x5f)-'A'+(ret<<4); // val = val*16+int(toupper(*s1))
    }
    
    return ret;
}

static int rand_seed = 54973;   // default seed


void srand(int x)
{
    rand_seed = x;
}


int rand()
{
    return rand_seed = (rand_seed * 1103515245 + 12345) & RAND_MAX;
}
