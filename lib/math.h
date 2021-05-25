int mult(int x, int y)
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
}