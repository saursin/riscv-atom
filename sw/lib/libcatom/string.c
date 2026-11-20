#include <string.h>

#define NULL_CHAR '\0'

int strncmp(char *s1, char *s2, int len)
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

char * strcpy(char *dest, char *src)
{
    return memcpy(dest, src, strlen(src) + 1);
}


char *strtok(char *str, char *dptr)
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
        if(strncmp(tmp, dptr, dlen)==0)
        {
            *tmp=NULL_CHAR;
            nxt = tmp+1;
            return ret;
        }
        tmp++;
    }
    nxt = NULL;
    return ret;
}


// memory manipulation

void *memcpy(void *dptr, const void *sptr, size_t len)
{
    char *d = dptr;
    const char *s = sptr;
    while(len--) 
        *d++ = *s++;
    return dptr;
}


void *memset(void *dptr, int x, size_t len)
{
    char *d = dptr;
    while(len--) 
        *d++ = x;
    return dptr;
}


int memcmp(const void *s1, const void *s2, size_t len) {
    const unsigned char *a = s1, *b = s2;
    for(; len--; a++, b++)
        if(*a != *b)
            return *a - *b;
    return 0;
}


char *strchr(const char *s, int c) {
    for(;; s++) {
        if(*s == (char)c)
            return (char *)s;
        if(!*s)
            return NULL;
    }
}
