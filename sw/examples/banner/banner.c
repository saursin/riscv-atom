#include "platform.h"
#include <stdio.h>
#include <csr.h>
#include  <gpio.h>
#include <time.h>

char * banner = 
"\n"
"                  .';,.           ....;;;.  \n"
"                 .ll,:o,                ':c,. \n"
"                 .dd;co'                  .cl,  \n"
"                .:o:;,.                     'o:  \n"
"                co.                          .oc  \n"
"               ,o'          .coddoc.          'd,  \n"
"               lc         .lXMMMMMMXl.         ll  \n"
"              .o:         ;KMMMMMMMMK,         :o. \n"
"              .o:         'OMMMMMMMMO.         :o. \n"
"               co.         .o0XNNX0o.         .oc  \n"
"               .o:           ..''..           :o.  \n"
"                'o:                          :o'  \n"
"                 .lc.                      .ll.  \n"
"                   ,lc'                  'cl,   \n"
"                     'cc:,..        ..,:c:'   \n"
"                        .;::::;;;;::::;.    \n"
"                              ....        \n"
"     ____  _________ _______    __         __                 \n"
"    / __ \\/  _/ ___// ____/ |  / /  ____ _/ /_____  ____ ___  \n"
"   / /_/ // / \\__ \\/ /    | | / /  / __ `/ __/ __ \\/ __ `__ \\ \n"
"  / _, _// / ___/ / /___  | |/ /  / /_/ / /_/ /_/ / / / / / /      \n"
" /_/ |_/___//____/\\____/  |___/   \\__,_/\\__/\\____/_/ /_/ /_/  \n"
"/=========By: Saurabh Singh (saurabh.s99100@gmail.com)====/\n\n";

print_isastring(){
    uint32_t misa = CSR_read(CSR_MISA);
    puts("RV32I");
    if(bitcheck(misa, 12))  putchar('M');
    if(bitcheck(misa, 0))   putchar('A');
    if(bitcheck(misa, 5))   putchar('F');
    if(bitcheck(misa, 3))   putchar('D');
    if(bitcheck(misa, 2))   putchar('C');   
}

int main()
{
    serial_init(UART_BAUD_115200);
    puts(banner);
    printf("CPU      : RISC-V Atom @ %d Hz\n", CLK_FREQ);
    puts("Arch     : "); print_isastring(); puts(" - little endian\n");
    printf("CODE RAM : 0x%08x (%d bytes)\n", (unsigned)CODERAM_BASE, (unsigned)CODERAM_SIZE);
    printf("DATA RAM : 0x%08x (%d bytes)\n", (unsigned)DATARAM_BASE, (unsigned)DATARAM_SIZE);
    puts("Exiting...\n");
    return 0;
}
