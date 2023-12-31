#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#define BACKSPACE 0x7f

// Enable printf %b format specifier (non standard) 
#define PRINTF_ENFMT_BIN

// Enable printf %f format specifer
#define PRINTF_ENFMT_FLOAT

// Enable printf %n format specifer
#define PRINTF_ENFMT_N


#define get_va_unum(_args, _lcount) \
        ((_lcount >= 2) ?   va_arg(_args, unsigned long long) : \
        ((_lcount == 1) ?   va_arg(_args, unsigned long) : \
                            va_arg(_args, unsigned))) \

#define get_va_num(_args, _lcount) \
        ((_lcount >= 2) ?   va_arg(_args, long long) : \
        ((_lcount == 1) ?   va_arg(_args, long) : \
                            va_arg(_args, int))) \


#define __char_is_num(x)    (((x) >= '0') && ((x) <= '9'))


//////////////////////////////////////////////////////////////////////////
// Helper Functions

static int __fputc(FILE *f, const char ch){
    if(ch == '\n' && (f == stdout  || f == stderr))
        __fputc(f, '\r'); 
    f->write((char*)&ch, 1);
    return 1;
}

static int __fputs(FILE *f, const char * s){
    int rv = 0;
    while (*s){
        rv += __fputc(f, *(s++));
    }
    return rv;
}

static int __fgetc(FILE *f){
    char c;
    f->read(&c, 1);
    return c;
}

static int __fprint_ull(FILE *f, unsigned long long n, unsigned base, char padc, int padn) {
    if(base < 2 || base > 16) {
        // invalid inputs
        return -1;
    }

    int ret = 0;
    char buf[65];   // sufficient to store unsigned long long in base 2
    int indx = 0;
    do {
        int rem = n % base;
        n = n / base;
        buf[indx++] = "0123456789abcdef"[rem];
    } while(n > 0U);

    // print padding
    if(padn > 0){
        while(indx < padn){
            __fputc(f, padc);
            ret++; padn--;
        }
    }

    // print buffer
    while(--indx >= 0){
        __fputc(f, buf[indx]);
        ret++;
    }
    return ret;
}

#ifdef PRINTF_ENFMT_FLOAT
static int __fltprint(FILE *f, double flt, char padc, int padd, int padf)
{
	int ret = 0;
	unsigned long long d = (unsigned long long) flt;
	double frac = flt - (double) d;
	ret = __fprint_ull(f, d, 10, padc, padd);
    if(padf>0){
        __fputc(f, '.');
	    ret++;
        while(padf--) {
            frac *= 10.0;
        }
        d = (unsigned long long) frac;
        ret += __fprint_ull(f, d, 10, '0', 0);
    }
	return ret;
}
#endif

//////////////////////////////////////////////////////////////////////////

int fputc(int c, FILE* f)
{
    __fputc(f, c);
    return c;
}

int putchar(char c) {
    fputc(c, stdout);
    return 1;
}

int fputs(const char *str, FILE *f)
{
    __fputs(f, str);
    return 0;
}

int puts(const char *str){
    fputs(str, stdout);
    return 0;
}

int fgetc(FILE *f) {
    return __fgetc(f);
}

int getchar(){
    return fgetc(stdin);
}

char *fgets(char * str, int num, FILE *f)
{
    char *ptr = str;
    char c;

    while(--num)
    {
        // read character
        c = __fgetc(f);

        if (c==BACKSPACE) // backspace
        {
            if(str!=ptr)    // String is not empty
            {   
                // Clear last character in string
                *(--ptr) = 0;
            }
            num++;
        }

        else if(c=='\n' || c=='\r') // ENTER key
        {
            break;
        }
        else
        {
            *(ptr++) = c;
        }

    }

    *(ptr) = '\0';
    return str;
}

char * gets(char *str){
    return fgets(str, __INT_MAX__, stdin);
}


int vfprintf(FILE *f, const char * fmt, va_list args){
    long long num_ll;
    unsigned long long num_ull;
    double num_f;
    uintptr_t ptr;

    char padc;
    int padn;
    int padf;
    int lcount;
    unsigned nwritten = 0;

    while (*fmt != '\0') {
        lcount = 0;

        if (*fmt == '%') {
            fmt++; // skip over %

            // Parse padchar (padc), width (padn) and precision (padf)
            padc = ' ';
            padn = 0;
            padf = 6;
            if (*fmt == '0') {
                padc = '0';
                fmt++;
            }
            while(1){
                if(!__char_is_num(*fmt))
                    break;
                padn = (*fmt-'0') + (padn << 3) + (padn << 1);  // padn = padn*10 + digit
                fmt++;
            }
            if(*fmt == '.') {
                fmt++; // skip over .
                padf = 0;
                while(1){
                    if(!__char_is_num(*fmt))
                        break;
                    padf = (*fmt-'0') + (padf << 3) + (padf << 1);  // padn = padn*10 + digit
                    fmt++;
                    if(padn > padf + 1)
                        padn -= (padf+1);
                }
            }

__vprintf_loop:
            switch(*fmt) {
                // int as decimal value
                case 'i':
                case 'd':
                    num_ll = get_va_num(args, lcount);
                    if(num_ll < 0){
                        __fputc(f, '-'); nwritten++;
                        num_ull = (unsigned long long) -num_ll;
                        padn--;
                    } else {
                        num_ull = (unsigned long long) num_ll;
                    }
                    nwritten += __fprint_ull(f, num_ull, 10, padc, padn);
                    break;
                
                // unsigned int as decimal
                case 'u':
                    num_ull = get_va_unum(args, lcount);
                    nwritten += __fprint_ull(f, num_ull, 10, padc, padn);
                    break;
                
                // unsigned int as hexadecimal
                case 'x':
                case 'X':
                    num_ull = get_va_unum(args, lcount);
                    nwritten += __fprint_ull(f, num_ull, 16, padc, padn);
                    break;

            #ifdef PRINTF_ENFMT_BIN
                // binary
                case 'b':
                    num_ull = get_va_unum(args, lcount);
                    nwritten += __fprint_ull(f, num_ull, 2, padc, padn);
                    break;
            #endif

                // octal
                case 'o':
                    num_ull = get_va_unum(args, lcount);
                    nwritten += __fprint_ull(f, num_ull, 8, padc, padn);
                    break;
                
                // length specifier 'l'
                case 'l':
                    lcount++;
                    fmt++;
                    goto __vprintf_loop;

                // length specifier 'z'
                case 'z':
					if (sizeof(size_t) == 8U)
						lcount = 2;
					fmt++;
					goto __vprintf_loop;
                    
                // pointer
                case 'p':
                    ptr = (uintptr_t) va_arg(args, void*);
                    __fputs(f, "0x");
                    nwritten += 2 + __fprint_ull(f, ptr, 16, padc, padn);
                    break;

                // char
                case 'c':
                    __fputc(f, va_arg(args, int)); nwritten++;
                    break;

                // string
                case 's':
                    nwritten += __fputs(f, va_arg(args, char*));
                    break;

            #ifdef PRINTF_ENFMT_N
                // store nwritten
                case 'n':
                    *(va_arg(args, int*)) = nwritten;
                    break;
            #endif

            #ifdef PRINTF_ENFMT_FLOAT
                // Float
                case 'f':
                    // floats are always promoted to double in va_args
					num_f = va_arg(args, double);
					if(num_f < 0) {
						__fputc(f, '-');
						num_f *= -1.0;
						padn--;
					}
					nwritten += __fltprint(f, num_f, padc, padn, padf);
                    break;
            #endif // PRINTF_ENFMT_FLOAT

                case '%':
                    __fputc(f, '%'); nwritten++;
                    break;

                default:
                    // Unsupported format specifier
                    return -1;
            }
            fmt++;
            continue;
        }

        __fputc(f, *fmt);
        fmt++;
        nwritten++;
    }
    return nwritten;
}


int fprintf(FILE *f, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int rv = vfprintf(f, fmt, va);
    va_end(va);
    return rv;
}


int printf(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int rv = vfprintf(stdout, fmt, va);
    va_end(va);
    return rv;
}


void dumphexbuf(char *buf, unsigned len, unsigned base_addr)
{
    const int bpw = 4; // bytes per word
    const int wpl = 4; // words per line

    for (unsigned i=0; i<len; i++) {
        // print address at the start of the line
        if(i%(wpl*bpw) == 0) {
            __fprint_ull(stdout, base_addr+i, 16, '0', 8);
            puts(": ");
        }
        
        // print byte
        __fprint_ull(stdout, 0xff & buf[i], 16, '0', 2);
        putchar(' ');
        
        // extra space at word boundry
        if(i%bpw == bpw-1)
            putchar(' ');
        
        if(i%(wpl*bpw) == (wpl*bpw-1)) // end of line 
            putchar('\n');
    }
}