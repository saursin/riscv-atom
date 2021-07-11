#include "../../lib/stdio.h"

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
            print_chr(' ');
        else
        {
            print_int(arr[i], 10);
            flag = 1;
        }
    }
}


/**
 * @briefPrint pascal's triangle
 * 
 */
int main() {
   int rows = 8, coef = 1, space, i, j;

   print_str("First "); print_int(rows, 10); print_str(" rows of pascal's triangle...\n");
   
   for (i = 0; i < rows; i++) {
      for (space = 1; space <= rows - i; space++)
         print_str("  ");
      for (j = 0; j <= i; j++) {
         if (j == 0 || i == 0)
            coef = 1;
         else
            coef = coef * (i - j + 1) / j;
         
         myprint(coef, 4);
         //print_int(coef, 10);
      }
      print_chr('\n');
   }
   return 0;
}