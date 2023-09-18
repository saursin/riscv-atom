#include "platform.h"
#include <stdio.h>
#include <csr.h>

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

void get_isa(char * buf){
    uint32_t misa = CSR_read(CSR_MISA);
    char misa_fmt[] = ""
    for(int i=0; i<32; i++) {

    }
}

int main()
{
    serial_init(UART_BAUD_115200);
    puts(banner);
    printf("CPU      : RISC-V Atom @ %d Hz\n", CLK_FREQ);
    puts("Arch     : RV32I - little endian\n");
    printf("CODE RAM : 0x%8x (%d bytes)\n", MEM_ROM_ADDR, MEM_ROM_SIZE);
    printf("DATA RAM : 0x%8x (%d bytes)\n", MEM_RAM_ADDR, MEM_RAM_SIZE);
    puts("Exiting...\n");
    return 0;
}
