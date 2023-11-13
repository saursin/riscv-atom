#pragma once

#define _ATTRIBUTE(x)	__attribute__((x))
#define _WEAK		    _ATTRIBUTE(weak)
#define _UNUSED		    _ATTRIBUTE(unused)
#define _INLINE		    _ATTRIBUTE(always_inline)
#define _NOINLINE	    _ATTRIBUTE(noinline)
#define _ALIGN(x)	    _ATTRIBUTE(aligned(x))
#define _SECTION(x)	    _ATTRIBUTE(section(x))
#define _DEPRICATE	    _ATTRIBUTE(depricated)
#define _ALIAS(x)	    _ATTRIBUTE(alias(x))
#define _FALLTHROUGH	_ATTRIBUTE(fallthrough)
#define _NORETURN	    _ATTRIBUTE(noreturn)
#define _NAKED		    _ATTRIBUTE(naked)
#define EXPORT_C(x)	    extern "C" x; x

#define STRINGIFY(s) #s
#define EXPAND_AND_STRINGIFY(s) STRINGIFY(s)

// Bitmanip Macros
#define bitset(data, nbit)      ((data) |  (1<<(nbit)))
#define bitclear(data, nbit)    ((data) & ~(1<<(nbit)))
#define bitflip(data, nbit)     ((data) ^  (1<<(nbit)))
#define bitcheck(data, nbit)    ((data) &  (1<<(nbit)))
// #define bitsetv(data, nbit, val)(((data) &= ~(1<<(nbit))) | (val << nbit))

