#ifndef __STDLIB_H___
#define __STDLIB_H___

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

#endif //__STDLIB_H__