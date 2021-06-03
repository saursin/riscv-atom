#ifndef __STDIO_H___
#define __STDIO_H___

// ================= output functions =======================

/**
 * @brief Sends a character to std out
 * 
 * @param c char
 */
void print_chr(const char c);


/**
 * @brief Sends a string to std out
 * 
 * @param str pointer to c string
 */
void print_str(const char* str);


/**
 * @brief Sends a decimal value to std out
 * 
 * @param val integer value
 */
//void print_dec(unsigned int val);


void print_int(long int n, int base);

void print_hex(unsigned int val, int digits);

// ================= output functions =======================


/**
 * @brief read a character from std in
 * 
 * @return char 
 */
char scan_chr();


/**
 * @brief Read a string from std in
 * 
 * @param str address of a string
 */
void scan_str(char * str);

#endif // __STDIO_H__