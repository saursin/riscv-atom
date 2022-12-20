#include "platform.h"
#include <stdio.h>

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

int main()
{
    serial_init(UART_BAUD_115200);
    puts(banner);
    printf("ROM size: %d\tbytes\t(%d KB)\n", MEM_ROM_SIZE, MEM_ROM_SIZE/1024);
    printf("RAM size: %d\tbytes\t(%d KB)\n", MEM_RAM_SIZE, MEM_RAM_SIZE/1024);
    puts("exiting...\n");
    return 0;
}
