#include "string.h"

int strncmp(char *s1,char *s2,int len)
{
    while(--len && *s1 && *s2 && (*s1==*s2)) s1++, s2++;
    
    return (*s1-*s2);
}


int strcmp(char *s1, char *s2)
{
    return strncmp(s1,s2,-1);
}


size_t strlen(char *s1)
{
    size_t len;
    
    for(len=0;s1&&*s1++;len++);

    return len;
}

char * strcpy(char *dest, const char *src)
{
    return memcpy(dest, src, strlen(src) + 1);
}


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

char *memcpy(char *dptr, char *sptr,int len)
{
    char *ret = dptr;
    while(len--) 
        *dptr++ = *sptr++;
    return ret;
}


char *memset(char *dptr, int x, int len)
{
    char *ret = dptr;
    while(len--) 
        *dptr++ = x;
    return ret;
}
