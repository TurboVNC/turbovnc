int
ffsl(unsigned long i)
{
    int j = 0;
    if (i)
    {
        for (j = 1; (i & 1) == 0; j++)
	    i >>= 1;
    }
    return j;
}
