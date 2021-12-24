#include "stdlib.h"

/**
 * @brief Parse integer from string containing an integer
 * 
 * @param s1 string
 * @return int integer value
 */
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


/**
 * @brief Parse integer from string containing hex value of integer
 * 
 * @param s1 hex string
 * @return int integer value
 */
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


/**
 * @brief Seed random number generator
 * 
 * @param x seed
 */
void srand(int x)
{
    rand_seed = x;
}


/**
 * @brief Generate random number using a Linear Congruntial Generator algorithm
 * 
 * @return int random integer
 */
int rand()
{
    return rand_seed = (rand_seed * 1103515245 + 12345) & RAND_MAX;
}
