// Program exit: exit() runs the functions registered with atexit() and the destructors
// (.fini_array), then returns to the bootloader (_exit, start.S). Startup is in libc_start.c.
#include <stdlib.h>
#include <stdio.h>

// Functions registered with atexit(); exit() runs them in reverse order of registration
#define ATEXIT_MAX  32      // the minimum the C standard requires
static void (*atexit_fns[ATEXIT_MAX])(void);
static int atexit_count;

int atexit(void (*fn)(void)) {
    if(atexit_count == ATEXIT_MAX)
        return -1;
    atexit_fns[atexit_count++] = fn;
    return 0;
}

extern void (*__fini_array_start[])(void);     // linker script
extern void (*__fini_array_end[])(void);

void exit(int status) {
    while(atexit_count > 0)
        atexit_fns[--atexit_count]();
    for(void (**f)(void) = __fini_array_end; f > __fini_array_start; )
        (*--f)();
    _exit(status);
}

void abort() {
    puts("Abort called...\n");
    exit(EXIT_FAILURE);
}
