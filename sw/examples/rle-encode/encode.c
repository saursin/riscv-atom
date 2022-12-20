#include <stdio.h>

void encode(char *str, unsigned int length)
{
    for (unsigned int i = 0; i < length; i++) {
 
        // Count occurrences of current character
        int count = 1;
        while (i < length - 1 && str[i] == str[i + 1]) {
            count++;
            i++;
        }
 
        // Print character and its count
        printf("0x%x, 0x%x, ", str[i], count);
    }
    printf("\n");
}

int main()
{
      char logo[1030] =

    "              vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n"
    "                  vvvvvvvvvvvvvvvvvvvvvvvvvvvv\n"
    "rrrrrrrrrrrrr       vvvvvvvvvvvvvvvvvvvvvvvvvv\n"
    "rrrrrrrrrrrrrrrr      vvvvvvvvvvvvvvvvvvvvvvvv\n"
    "rrrrrrrrrrrrrrrrrr    vvvvvvvvvvvvvvvvvvvvvvvv\n"
    "rrrrrrrrrrrrrrrrrr    vvvvvvvvvvvvvvvvvvvvvvvv\n"
    "rrrrrrrrrrrrrrrrrr    vvvvvvvvvvvvvvvvvvvvvvvv\n"
    "rrrrrrrrrrrrrrrr      vvvvvvvvvvvvvvvvvvvvvv  \n"
    "rrrrrrrrrrrrr       vvvvvvvvvvvvvvvvvvvvvv    \n"
    "rr                vvvvvvvvvvvvvvvvvvvvvv      \n"
    "rr            vvvvvvvvvvvvvvvvvvvvvvvv      rr\n"
    "rrrr      vvvvvvvvvvvvvvvvvvvvvvvvvv      rrrr\n"
    "rrrrrr      vvvvvvvvvvvvvvvvvvvvvv      rrrrrr\n"
    "rrrrrrrr      vvvvvvvvvvvvvvvvvv      rrrrrrrr\n"
    "rrrrrrrrrr      vvvvvvvvvvvvvv      rrrrrrrrrr\n"
    "rrrrrrrrrrrr      vvvvvvvvvv      rrrrrrrrrrrr\n"
    "rrrrrrrrrrrrrr      vvvvvv      rrrrrrrrrrrrrr\n"
    "rrrrrrrrrrrrrrrr      vv      rrrrrrrrrrrrrrrr\n"
    "rrrrrrrrrrrrrrrrrr          rrrrrrrrrrrrrrrrrr\n"
    "rrrrrrrrrrrrrrrrrrrr      rrrrrrrrrrrrrrrrrrrr\n"
    "rrrrrrrrrrrrrrrrrrrrrr  rrrrrrrrrrrrrrrrrrrrrr\n"
    "\n"
    "       INSTRUCTION SETS WANT TO BE FREE\n\n";

    serial_init(UART_BAUD_115200);
    encode(logo, 1030);
}