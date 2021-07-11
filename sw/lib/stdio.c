#include "stdio.h"
#include "uart.h"
//#include "math.h"

// ================= output functions =======================

/**
 * @brief Sends a character to std out
 * 
 * @param c char
 */
void print_chr(const char c)
{
    uart_send(c);
}


/**
 * @brief Sends a string to std out
 * 
 * @param str pointer to c string
 */
void print_str(const char* str)
{
    while (*str) 
    {
        uart_send(*str++);
    }
}



void print_int(long int n, int base)
{
        // If number is smaller than 0, put a - sign
        // and change number to positive
        if (base == 10 && n < 0) {
            print_chr('-');
            n = -n;
        }
    
        // Remove the last digit and recur
        switch(base)
        {
            case 2:     if (n/2)
                            print_int(n/2, base);
                        break;

            case 8:     if (n/8)
                            print_int(n/8, base);
                        break;

            case 10:    if (n/10)
                            print_int(n/10, base);
                        break;

            case 16:    if (n/16)
                            print_int(n/16, base);
                        break;
        }
        
        // Print the last digit
        switch(base)
        {
            case 2:     print_chr(n%2 + '0');
                        break;

            case 8:     print_chr(n%8 + '0');
                        break;

            case 10:    print_chr(n%10 + '0');
                        break;

            case 16:    if(n%16 > 9)
                            print_chr((n%16)-10 + 'a');
                        else
                            print_chr(n%16 + '0');
                        break;
        }
}



// /**
//  * @brief Sends a decimal value to std out
//  * 
//  * @param val integer value
//  */
// void print_dec(unsigned int val)
// {
// 	char buffer[10];
// 	char *p = buffer;
// 	while (val || p == buffer) 
//     {
// 		*(p++) = val % 10;
// 		val = val / 10;
// 	}
// 	while (p != buffer) {
//         print_chr('0' + *(--p));
// 	}
// }

// void putdx(unsigned i, int mode) // mode1 = dec, mode0 = hex
// {
//     char *hex="0123456789abcdef";

//     int dbd[] = { 1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1, 0 };
//     int dbx[] = { 16777216, 65536, 256, 1, 0 };

//     int *db = mode ? dbd : dbx;

//     if(mode && i<0)
//     {
//         print_chr('-');
//         i=-1;
//     }

//     int j,k,l;

//     for(j=0,k=24;(l=db[j]);j++,k-=8)
//     {
//         if(l==1 || i>=l)
//         {
//             if(mode)
//             {
//                 print_chr(hex[(i/l)%10]);
//             }
//             else
//             {
//                 print_chr(hex[(i>>(k+4))&15]);
//                 print_chr(hex[(i>>k)&15]);
//             }
//         }
//     }
// }

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
        print_chr("0123456789ABCDEF"[(val >> i) % 16]);
    }
}

// ================= output functions =======================


/**
 * @brief read a character from std in
 * 
 * @return char 
 */
/*char scan_chr()
{
    return uart_recieve()
}*/


/**
 * @brief Read a string from std in
 * 
 * @param str address of a string
 */
/*void scan_str(char * str)
{
    char ch = 0;
    while(ch != '\n')
    {
        ch = *str = *((volatile char*) IO_UART_RX_ADDRESS);
        str++;
    }
}*/

