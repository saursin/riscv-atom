// Startup, the C part: start.S sets up the stack, .data, .bss and the trap vector, then jumps
// here with the program's main. Sets up the heap and stdio (UART), runs the constructors, then
// main, then exit() with its result (exit.c: exit functions and destructors, then back to the
// bootloader).
#include <stdlib.h>
#include <serial.h>

void heap_init(void);                               // heap.c

extern void (*__init_array_start[])(void);          // linker script
extern void (*__init_array_end[])(void);

void __libc_start_main(int (*main)(int argc, char **argv)) {
    // Initialize lib
    heap_init();
    serial_init(UART_BAUD_115200);

    // Call constructors
    for(void (**f)(void) = __init_array_start; f < __init_array_end; f++)
        (*f)();
    
    // Call main
    exit(main(0, 0));
}
