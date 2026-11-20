#pragma once
#include <stddef.h>

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
char *strtok(char *str, char *dptr);

/**
 * @brief Copy string
 * 
 * @param dest destination ptr
 * @param src source ptr
 * @return char* destination ptr
 */
char * strcpy (char *dest, char *src);


// memory manipulation

/**
 * @brief copy a block of memory from a location to another
 * 
 * @param dptr destintion pointer
 * @param sptr source pointer
 * @param len length of block (in bytes)
 * @return void* dptr
 */
void *memcpy(void *dptr, const void *sptr, size_t len);


/**
 * @brief Fill a block of memory with given value
 * 
 * @param dptr destintion pointer
 * @param x value to be filled
 * @param len length of the block (in bytes)
 * @return void* dptr
 */
void *memset(void *dptr, int x, size_t len);


/**
 * @brief Compare two blocks of memory
 *
 * @param s1 block 1
 * @param s2 block 2
 * @param len length of the blocks (in bytes)
 * @return int <0, 0 or >0 as the first differing byte of s1 is less than, equal to or greater than that of s2
 */
int memcmp(const void *s1, const void *s2, size_t len);


/**
 * @brief Find the first occurrence of a character in a string
 *
 * @param s string
 * @param c character
 * @return char* pointer to it in s (the terminating null counts), or NULL
 */
char *strchr(const char *s, int c);
