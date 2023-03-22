#include <stdio.h>
#include <stdlib.h>

void myprint(int num, int digits)
{
    int arr[4];

    int n;
    int i;
    for(i=0; i<digits; i++)
    {
        arr[i] = num%10;
        num = num/10;
    }

    int flag = 0;
    for(i=digits-1; i>=0; i--)
    {
        if(arr[i]==0 && flag == 0)
            putchar(' ');
        else
        {
            putint(arr[i], 0, DEC);
            flag = 1;
        }
    }
}


/**
 * @briefPrint pascal's triangle
 * 
 */
int main() {
   serial_init(UART_BAUD_115200);

   int rows = 8, coef = 1, space, i, j;

   printf("First %d rows of pascal's triangle...\n", rows);
   
   for (i = 0; i < rows; i++) {
      for (space = 1; space <= rows - i; space++)
         printf("  ");
      for (j = 0; j <= i; j++) {
         if (j == 0 || i == 0)
            coef = 1;
         else
            coef = coef * (i - j + 1) / j;
         
         myprint(coef, 4);
         //print_int(coef, 10);
      }
      putchar('\n');
   }
   return 0;
}