#include "stdio.h"
#include "atomrv.h"

// ================= output functions =======================

/**
 * @brief Sends a character to std out
 * 
 * @param c char
 */
void print_chr(const char c)
{
    *((volatile char*) STDIO_TX_ADDRESS) = c;

    // Toggle tx_ack
    *((volatile char*) STDIO_TX_ACK_ADDRESS) = (char) 1;
    *((volatile char*) STDIO_TX_ACK_ADDRESS) = (char) 0;
}


/**
 * @brief Sends a string to std out
 * 
 * @param str pointer to c string
 */
void print_str(const char* str)
{
    while (*str) {
        *((volatile char*) STDIO_TX_ADDRESS) = *str++;

        // Toggle tx_ack
        *((volatile char*) STDIO_TX_ACK_ADDRESS) = (char) 1;
        *((volatile char*) STDIO_TX_ACK_ADDRESS) = (char) 0;
    }
}


/**
 * @brief Sends a decimal value to std out
 * 
 * @param val integer value
 */
void print_dec(unsigned int val)
{
	char buffer[10];
	char *p = buffer;
	while (val || p == buffer) {
		*(p++) = val % 10;
		val = val / 10;
	}
	while (p != buffer) {
        *((volatile char*) STDIO_TX_ADDRESS) = '0' + *(--p);

        // Toggle tx_ack
        *((volatile char*) STDIO_TX_ACK_ADDRESS) = (char) 1;
        *((volatile char*) STDIO_TX_ACK_ADDRESS) = (char) 0;
	}
}


/**
 * @brief Sends a hexadecimal value to std out
 * 
 * @param val 
 * @param digits 
 */
void print_hex(unsigned int val, int digits)
{
	for (int i = (4*digits)-4; i >= 0; i -= 4)
    {
        *((volatile char*) STDIO_TX_ADDRESS) = "0123456789ABCDEF"[(val >> i) % 16];

        // Toggle tx_ack
        *((volatile char*) STDIO_TX_ACK_ADDRESS) = (char) 1;
        *((volatile char*) STDIO_TX_ACK_ADDRESS) = (char) 0;
    }
}

// ================= output functions =======================


/**
 * @brief read a character from std in
 * 
 * @return char 
 */
char scan_chr()
{
    return *((volatile char*) STDIO_RX_ADDRESS);
}


/**
 * @brief Read a string from std in
 * 
 * @param str address of a string
 */
void scan_str(char * str)
{
    char ch = 0;
    while(ch != '\n')
    {
        ch = *str = *((volatile char*) STDIO_RX_ADDRESS);
        str++;
    }
}

