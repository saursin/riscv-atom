#pragma once

#include <platform.h>
#include <file.h>
#include <serial.h>

#ifndef EOF
    #define EOF (-1)
#endif

#ifndef NULL
    #define NULL ((void*)(0))
#endif

typedef Device_t FILE;

// Standard file descriptors
#define stdin		&stddev[DEV_STDIN]
#define stdout		&stddev[DEV_STDOUT]
#define stderr		&stddev[DEV_STDERR]


/**
 * @brief Writes a char to file
 * 
 * @param c char
 * @param f file
 * @return int char
 */
int fputc(int c, FILE *f);

/**
 * @brief Prints a char to stdout
 * @param c char
 */
int putchar(char c);

/**
 * @brief Write a string to file
 * 
 * @param str string
 * @param f file
 * @return int 
 */
int fputs(const char *str, FILE *f);

/**
 * @brief Write a string to stdout
 * 
 * @param str string
 * @return int 
 */
int puts(const char *str);

/**
 * @brief Get next char from file, advance position
 * 
 * @param f 
 * @return int 
 */
int fgetc(FILE *f);

/**
 * @brief Get a character from stdin
 * @return int 
 */
int getchar();

/**
 * @brief Read a string from file until n chars are read or '\n' is read or EOF is read.
 * 
 * @param str string
 * @param n max number of chars to read (including '\n')
 * @param f file
 * @return char* string
 */
char *fgets(char *str, int n, FILE *f);

/**
 * @brief Read a string from stdin till either the '\n' is read or EOF is read
 * 
 * @param str string
 * @return char* string
 */
char * gets(char *str);


/**
 * Supported Format specifiers for printf functions
 * - %d, %i, %u, %x, %b(optional), %o
 * - %p, %c, %s, %f(optional)
 * 
 * - fmt specifiers can be used with 'l', 'll', 'z' prefix to 
 *   parse the number as long, long long and size_t respectively.
 * - fmt specifiers can include padding
 */

/**
 * @brief Writes formatted string to file
 * @param file file
 * @param fmt format string
 * @return int number of chars written
 */
int	fprintf (FILE * file, const char * fmt, ...)
        __attribute__((__format__ (__printf__, 2, 3)));

/**
 * @brief Writes formatted string to stdout
 * @param fmt format string
 * @return int number of chars written
 */
int	printf (const char * fmt, ...)
        __attribute__((__format__ (__printf__, 1, 2)));


// ========== Non Standard Functions ==========

/**
 * @brief Prints a char buffer in hexdump style
 * 
 * @param buf buffer
 * @param len length of buffer
 * @param base_addr base address
 */
void dumphexbuf(char *buf, unsigned len, unsigned base_addr);