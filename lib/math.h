#ifndef __MATH_H__
#define __MATH_H__

/*int mult(int x, int y)
{
    int prod, i=0;
    for(i=0; i<y; i++)
    {
        prod+=x;
    }
    return prod;
}

int pow(int x)
{
    return mult(x, x);
}*/

int __modsi3(int x, int y)
{
    int i=0;
    while (x>y)
    {
        x = x-y;
        i++;
    }
    return x;
}

int __divsi3(int x, int y)
{
    int i=0;
    while (x>y)
    {
        x = x-y;
        i++;
    }
    return i;
}


#endif //__MATH_H__
