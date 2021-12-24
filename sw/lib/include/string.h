#ifndef __STRING_H__
#define __STRING_H__

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef __SIZE_T_DEFINED
#define __SIZE_T_DEFINED
typedef unsigned long size_t;
#endif

#define NUL '\0'

/**
 * @brief Compares the string pointed to, by str1 to the string 
 * pointed to by str2 at most the first n bytes of str1 and str2.
 *
 * @param s1 string1
 * @param s2 string2
 * @param len number of chars to compare
 * @return int 
 */
int strncmp(char *s1,char *s2,int len);

/**
 * @brief Compares the string pointed to, by str1 to the string 
 * pointed to by str2.
 * 
 * @param s1 string1
 * @param s2 string2
 * @return int 
 */
int strcmp(char *s1, char *s2);

/**
 * @brief Computes the length of the string str up to but not 
 * including the terminating null character.
 * 
 * @param s1 string
 * @return size_t 
 */
size_t strlen(char *s1);


/**
 * @brief Breaks string str into a series of tokens separated by delim.
 * 
 * @param str string
 * @param dptr delimiter
 * @return char* 
 */
char *strtok(char *str,char *dptr);


// memory manipulation

/**
 * @brief copy a block of memory from a location to another
 * 
 * @param dptr destintion pointer
 * @param sptr source pointer
 * @param len length of block (in bytes)
 * @return char* dptr
 */
char *memcpy(char *dptr, char *sptr,int len);


/**
 * @brief Fill a block of memory with given value
 * 
 * @param dptr destintion pointer
 * @param x value to be filled
 * @param len length of the block (in bytes)
 * @return char* 
 */
char *memset(char *dptr, int x, int len);

#endif //__STRING_H__