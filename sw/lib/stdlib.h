#ifndef __STDLIB_H___
#define __STDLIB_H___

// ==================== Type Conversions =============================
/**
 * @brief String to Integer
 * 
 * @param s1 string
 * @return int value
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
 * @brief Hex String to integer
 * 
 * @param s1 hex string
 * @return int integer
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

//==================== RANDOM NUMBER GENERATION =============================
#define RAND_MAX ((1U << 31) - 1)

int rand_seed = 54973;

inline void srand(int x)
{
    rand_seed = x;
}

// Linear Congruntial Generator
inline int rand() 
{
    return rand_seed = (rand_seed * 1103515245 + 12345) & RAND_MAX;
}

// ====================== Memory Management ===============================
char *memcpy(char *dptr,char *sptr,int len)
{
    char *ret = dptr;

    while(len--) *dptr++ = *sptr++;

    return ret;
}

char *memset(char *dptr, int c, int len)
{
    char *ret = dptr;
    
    while(len--) *dptr++ = c;
    
    return ret;
}



#endif //__STDLIB_H__