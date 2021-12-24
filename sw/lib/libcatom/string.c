#include "string.h"


/**
 * @brief Compares the string pointed to, by str1 to the string 
 * pointed to by str2 at most the first n bytes of str1 and str2.
 *
 * @param s1 string1
 * @param s2 string2
 * @param len number of chars to compare
 * @return int 
 */
int strncmp(char *s1,char *s2,int len)
{
    while(--len && *s1 && *s2 && (*s1==*s2)) s1++, s2++;
    
    return (*s1-*s2);
}


/**
 * @brief Compares the string pointed to, by str1 to the string 
 * pointed to by str2.
 * 
 * @param s1 string1
 * @param s2 string2
 * @return int 
 */
int strcmp(char *s1, char *s2)
{
    return strncmp(s1,s2,-1);
}


/**
 * @brief Computes the length of the string str up to but not 
 * including the terminating null character.
 * 
 * @param s1 string
 * @return size_t 
 */
size_t strlen(char *s1)
{
    size_t len;
    
    for(len=0;s1&&*s1++;len++);

    return len;
}


/**
 * @brief Breaks string str into a series of tokens separated by delim.
 * 
 * @param str string
 * @param dptr delimiter
 * @return char* 
 */
char *strtok(char *str,char *dptr)
{
    static char *nxt = NULL;

    int dlen = strlen(dptr);
    char *tmp;

         if(str) tmp=str;
    else if(nxt) tmp=nxt;
    else return NULL;
    
    char *ret=tmp;

    while(*tmp)
    {
        if(strncmp(tmp,dptr,dlen)==0)
        {
            *tmp=NUL;
            nxt = tmp+1;
            return ret;
        }
        tmp++;
    }
    nxt = NULL;
    return ret;
}


// memory manipulation

/**
 * @brief copy a block of memory from a location to another
 * 
 * @param dptr destintion pointer
 * @param sptr source pointer
 * @param len length of block (in bytes)
 * @return char* dptr
 */
char *memcpy(char *dptr, char *sptr,int len)
{
    char *ret = dptr;
    while(len--) 
        *dptr++ = *sptr++;
    return ret;
}


/**
 * @brief Fill a block of memory with given value
 * 
 * @param dptr destintion pointer
 * @param x value to be filled
 * @param len length of the block (in bytes)
 * @return char* 
 */
char *memset(char *dptr, int x, int len)
{
    char *ret = dptr;
    while(len--) 
        *dptr++ = x;
    return ret;
}
