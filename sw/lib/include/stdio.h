#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdbool.h>

// Send Carrige Return before Linefeed
#define SEND_CR_BEFORE_LF

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef __SIZE_T_DEFINED
#define __SIZE_T_DEFINED
typedef unsigned long size_t;
#endif

#define EOF (-1)


/**
 * @brief Reads a character from stdin
 * 
 * @return int character
 */
int getchar(void);

/**
 * @brief Writes a character to stdout.
 * 
 * @param chr character
 */
void putchar(char chr);

/**
 * @brief reads a line from stdin and stores it into the string pointed to by str
 * 
 * @param str string pointer
 * @param bufsize max size of string object
 * @param echo if enabed, sends the recieved char to stdout
 * @param prompt print prompt before actual string
 * 
 * @return str string pointer
 */
char *gets(char * str, int bufsize, bool echo, char * prompt);


/**
 * @brief writes a string to stdout.
 * 
 * @param ptr pointer to the string 
 */
void puts(char *ptr);


/**
 * @brief Writes an integer (signed) to stdout
 * 
 * @param n number
 * @param base base
 * @param uppercase upprtcase flag
 */
void putint(long long n, int base, bool upprcase);


/**
 * @brief Sends a hexadecimal value to std out
 * 
 * @param val integer value
 * @param digits no of digits to print
 * @param uppercase uppercase flag
 */
void puthex(unsigned int val, int digits, bool uppercase);


/**
 * @brief Printf function
 * 
 * @param fmt format specifier string (with placeholders)
 * @param ... values for placeholders
 * @return int 
 */
int printf(char *fmt,...);


#endif // __STDIO_H__