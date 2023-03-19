#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdbool.h>
#include "serial.h"

// Send Carrige Return before Linefeed
#define SEND_CR_BEFORE_LF

#define EOF (-1)

// Bases to be used with putint
#define BIN 2
#define OCT 8
#define DEC 10
#define HEX 16


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
 * @param n integer
 * @param ndigits number of digits to display (use 0 for auto ndigits)
 * @param base base
 */
void putint(int64_t n, unsigned ndigits, unsigned base);


/**
 * @brief Printf function
 * 
 * @param fmt format specifier string (with placeholders)
 * @param ... values for placeholders
 * @return int 
 */
int printf(char *fmt, ...);


/**
 * @brief Prints buffer in hex
 * 
 * @param buf pointer to buffer
 * @param len length of buffer
 * @param base_addr base address of buffer
 */
void dumphexbuf(uint8_t *buf, unsigned len, unsigned base_addr);

#endif // __STDIO_H__