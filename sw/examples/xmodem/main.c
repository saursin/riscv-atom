#include "xmodem.h"
#include "platform.h"
#include "string.h"
#include "stdio.h"

int main()
{
    char bf [4098];
    memset(bf, '\0', sizeof(bf));

    int s = xmodemReceive(bf, 4098);

    puts("\nGot:\n");
    puts(bf);
}