#include "stdio.h"
#include "serial.h"
#include <stdarg.h>
#include <stdbool.h>

///////////////////////////////////////////////////////////////////
// getchar & Putchar use Low-level Serial ports
int getchar(void)
{
    return (int) serial_read();
}


void putchar(char chr)
{
    #ifdef SEND_CR_BEFORE_LF
    // convert '\n' to "\r\n"
    if(chr == '\n') 
        serial_write('\r');
    #endif
    serial_write(chr);
}

///////////////////////////////////////////////////////////////////
// Other Functions use getchar and putchar

char *gets(char * str, int bufsize, bool echo, char * prompt)
{
    char *ptr = str;
    char c;

    while(--bufsize)
    {
        // read character
        c = getchar();

        if (c==0x7f) // backspace
        {
            if(str!=ptr)    // String is not empty
            {   
                // Clear last character in string
                *(--ptr) = 0;

                if(echo)
                {
                    // Send carrige return
                    putchar('\r');

                    if(prompt!=NULL) 
                        puts(prompt);

                    // RePrint line with last character deleted
                    char * i = str; 
                    while(i!=ptr+1)
                    {
                        putchar(*(i++));
                    }
                    putchar(' ');   // erase last char
                    
                    // RePrint to move cursor to proper location
                    
                    // Send carrige return
                    putchar('\r');
                    if(prompt!=NULL) 
                        puts(prompt);

                    i = str;
                    while(i!=ptr+1)
                    {
                        putchar(*(i++));
                    }
                }
            }
            bufsize++;
        }

        else if(c=='\n' || c=='\r') // ENTER key
        {
            break;
        }
        else
        {
            *(ptr++) = c;
            if (echo)
                putchar(c);
        }

    }
    if(echo) 
        putchar('\n'); // \n
    *(ptr) = '\0';
    return str;
}


void puts(char *ptr)
{
    while(*ptr) 
        putchar(*ptr++);
}


void putint(long long n, int base, bool uppercase)
{
    // If number is smaller than 0, put a - sign
        // and change number to positive
        if (base == 10 && n < 0) {
            putchar('-');
            n = -n;
        }
    
        // Remove the last digit and recur
        switch(base)
        {
            case 2:     if (n/2)
                            putint(n/2, base, uppercase);
                        break;

            case 8:     if (n/8)
                            putint(n/8, base, uppercase);
                        break;

            case 10:    if (n/10)
                            putint(n/10, base, uppercase);
                        break;

            case 16:    if (n/16)
                            putint(n/16, base, uppercase);
                        break;
        }
        
        // Print the last digit
        switch(base)
        {
            case 2:     putchar(n%2 + '0');
                        break;

            case 8:     putchar(n%8 + '0');
                        break;

            case 10:    putchar(n%10 + '0');
                        break;

            case 16:    if(n%16 > 9)
                            putchar((n%16)-10 + (uppercase ? 'A' : 'a'));
                        else
                            putchar(n%16 + '0');
                        break;
        }
}


void puthex(unsigned int val, int digits, bool uppercase)
{
	for (int i = (4*digits)-4; i >= 0; i -= 4)
    {
        if(uppercase)
            putchar("0123456789ABCDEF"[(val >> i) % 16]);
        else
            putchar("0123456789abcdef"[(val >> i) % 16]);
    }
}


int printf(char *fmt,...)
{
    va_list ap;

    for(va_start(ap, fmt);*fmt;fmt++)
    {
        if(*fmt=='%')
        {
            fmt++;
            switch(*fmt)
            {
                case 'b':
                    // binary
                    putint(va_arg(ap, long int), 2, false);
                    break;
                
                case 'o':
                    // Octal
                    putint(va_arg(ap, long int), 8, false);
                    break;

                case 'd':
                case 'i':
                    // Signed decimal number
                    putint(va_arg(ap, long int), 10, false);
                    break;
                
                case 'l':
                    if(*(++fmt) == 'l')
                    {
                        // Signed decimal number (long long)
                        if(*(++fmt) == 'd')
                        {
                            // Signed decimal number (long long)
                            putint(va_arg(ap, long long), 10, false);
                            break;
                        }
                        else
                        {
                            putchar('l');
                            putchar(*fmt);
                            break;
                        }                        
                        break;
                    }
                    else
                    {
                        putchar('l');
                        putchar(*fmt);
                        break;
                    }
               
                case 'x':
                    // unsigned hexadecimal (lowercase)
                    putint(va_arg(ap, long int), 16, false);
                    break;

                case 'X':
                    // unsigned hexadecimal (uppercase)
                    putint(va_arg(ap, long int), 16, true);
                    break;
                
                case 'p':
                    // Pointer address
                    puts("0x"); puthex(va_arg(ap, unsigned int), 8, false);
                    break;

                case 'c':
                    // character
                    putchar(va_arg(ap,int));
                    break;
                
                case 's':
                    // string
                    puts(va_arg(ap, char *));
                    break;

                default:
                    putchar(*fmt);
            }           
        }
        else putchar(*fmt);
    }

    va_end(ap);

    return 0;
}

void dumphexbuf(uint8_t *buf, unsigned len, unsigned base_addr)
{
    const int bpw = 4; // bytes per word
    const int wpl = 4; // words per line

    for (unsigned i=0; i<len; i++) {
        // print address at the start of the line
        if(i%(wpl*bpw) == 0) {
            puthex(base_addr+i, 8, false); puts(": ");
        }
        
        // print byte
        puthex(0xff & buf[i], 2, false); putchar(' ');
        
        // extra space at word boundry
        if(i%bpw == bpw-1)
            putchar(' ');
        
        if(i%(wpl*bpw) == (wpl*bpw-1)) // end of line 
            putchar('\n');
    }
    putchar('\n');
}