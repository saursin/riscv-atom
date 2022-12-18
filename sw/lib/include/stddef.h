#ifndef __STDDEF__
#define __STDDEF__

#ifndef NULL
#define NULL    ((void *) 0x0)
#endif

#ifndef __SIZE_T_DEFINED
#define __SIZE_T_DEFINED
typedef unsigned long long size_t;
#endif

#endif // __STDDEF__