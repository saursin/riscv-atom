#include <stdio.h>

void main()
{
    serial_init(UART_BAUD_115200);
    char hello[] = "Hello World!\n      -- from C\n\n";
    puts(hello);
    return;
}
