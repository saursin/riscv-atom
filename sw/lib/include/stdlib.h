#pragma once
#include <stddef.h>

// Define Exit codes
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

// Define Rand Max
#define RAND_MAX ((1U << 31) - 1)

/**
 * @brief Exit normally
 * @param status exit status
 */
void exit(int status) __attribute__((noreturn));

/**
 * @brief Exit abnormally
 * 
 */
void abort() __attribute__((noreturn));

///////////////////////////////////////////////////////////////////
// Type Conversion 

/**
 * @brief Parse integer from string containing an integer
 * 
 * @param s1 string
 * @return int integer value
 */
int atoi(char *s1);

/**
 * @brief Parse integer from string containing hex value of integer
 * 
 * @param s1 hex string
 * @return int integer value
 */
int xtoi(char *s1);

///////////////////////////////////////////////////////////////////
// Random Number Generater

/**
 * @brief Seed random number generator
 * 
 * @param x seed
 */
void srand(int x);


/**
 * @brief Generate random number using a Linear Congruntial Generator algorithm
 * 
 * @return int random integer
 */
int rand();

///////////////////////////////////////////////////////////////////
// Memory Related

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


/**
 * @brief Dynamically allocate aa block of memory
 * 
 * @param block_size size of block to be allocated (in bytes)
 * @return void* pointer to the allocated block of memory
 */
void *malloc(size_t block_size);


/**
 * @brief Frees dynamically allocated block of memory
 * 
 * @param ptr pointer to the block of memory to be freed
 */
void free(void* ptr);

