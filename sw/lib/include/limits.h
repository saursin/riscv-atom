#ifndef __LIMITS_H__
#define __LIMITS_H__

#define LONG_BIT 32
#define LONG_MAX 0x7fffffffL
#define LLONG_MAX 0x7fffffffffffffffLL

#if '\0' - 1 > 0
#define CHAR_MIN 0
#define CHAR_MAX 255
#else
#define CHAR_MIN (-128)
#define CHAR_MAX 127
#endif

/* Some universal constants... */

/* Number of bits in byte */
#define CHAR_BIT 8

/* Minimum value of signed char */
#define SCHAR_MIN (-128)

/* Maximum value of signed char */
#define SCHAR_MAX 127

/* Maximum value of unsigned char */
#define UCHAR_MAX 255

/* Minimum value of signed short (-32768) */
#define SHRT_MIN (-1 - 0x7fff)  

/* Maximum value of signed short (-32767) */
#define SHRT_MAX 0x7fff

/* Maximum value of unsigned short (65535) */
#define USHRT_MAX 0xffff

/* Minimum value of signed int (-2,147,483,648) */
#define INT_MIN (-1 - 0x7fffffff)

/* Maximum value of signed int (2,147,483,647) */
#define INT_MAX 0x7fffffff

/* Maximum value of unsigned int (4,294,967,295) */
#define UINT_MAX 0xffffffffU

/* Minimum value of signed long */
#define LONG_MIN (-LONG_MAX - 1)

/* Maximum value of unsigned long */
#define ULONG_MAX (2UL * LONG_MAX + 1)

/* Minimum value of signed long long */
#define LLONG_MIN (-LLONG_MAX - 1)

/* Maximum value of unsigned long long */
#define ULLONG_MAX (2ULL * LLONG_MAX + 1)

/* Maximum multibyte length of a character across all locales */
#define MB_LEN_MAX 4

#endif // __LIMITS_H__
