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


void putint(int64_t n, unsigned ndigits, unsigned base)
{
    if(base < 2 || base > 16) {
        puts("\nERROR (stdio::putint): Unsupported Base\n");
        return;
    }

    if(ndigits > 32)
    {
        puts("\nERROR (stdio::putint): ndigits too large\n");
        return;
    }
   
    if(base == 10 && n < 0) {
        putchar('-');
        n = -n;
    }

    unsigned required_ndigits=1;
    for(int32_t i=n; i/=base; required_ndigits++)
    ;

    if(required_ndigits>ndigits)
        ndigits=required_ndigits;

    char symbols[] = 
    #ifdef HEX_UPPERCASE
        "0123456789ABCDEF";
    #else
        "0123456789abcdef";
    #endif

    char buf[ndigits+1];
    int loc = ndigits;
    buf[loc--] = '\0';

    for(; loc >= 0; loc--) {
        if(loc < (ndigits-required_ndigits)) {
            buf[loc] = '0';
            continue;
        }
        buf[loc] = symbols[n % base];
        n /= base;
    }
    puts(buf);
}


int printf(char *fmt, ...)
{
    va_list ap;
    for(va_start(ap, fmt);*fmt;fmt++)
    {
        if(*fmt=='%')
        {
            fmt++;
            int ndigits=0;
            for(ndigits=0; *fmt >= '0' && *fmt <= '9'; fmt++) {
                ndigits = (*fmt-'0') + (ndigits<<3)+(ndigits<<1); // val = val*10 + digit
            }

            switch(*fmt)
            {
                case 'b':
                    // binary
                    putint(va_arg(ap, int32_t), ndigits, BIN);
                    break;
                
                case 'o':
                    // Octal
                    putint(va_arg(ap, int32_t), ndigits, OCT);
                    break;

                case 'd':
                case 'i':
                    // Signed decimal number
                    putint(va_arg(ap, int32_t), ndigits, DEC);
                    break;
                
                case 'l':
                    if(*(++fmt) == 'l')
                    {
                        // Signed decimal number (long long)
                        if(*(++fmt) == 'd')
                        {
                            // Signed decimal number (long long)
                            putint(va_arg(ap, long long), ndigits, DEC);
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
                    break;

                case 'x':
                case 'X':
                    // unsigned hexadecimal (lowercase)
                    putint(va_arg(ap, int32_t), ndigits, HEX);
                    break;
                
                case 'p':
                    // Pointer address
                    puts("0x"); putint(va_arg(ap, uint32_t), 8, HEX);
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
            putint(base_addr+i, 8, HEX); puts(": ");
        }
        
        // print byte
        putint(0xff & buf[i], 2, HEX);  putchar(' ');
        
        // extra space at word boundry
        if(i%bpw == bpw-1)
            putchar(' ');
        
        if(i%(wpl*bpw) == (wpl*bpw-1)) // end of line 
            putchar('\n');
    }
}