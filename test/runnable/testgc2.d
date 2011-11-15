// PERMUTE_ARGS:

module testgc2;

import core.stdc.stdio;
import core.exception : OutOfMemoryError;

/*******************************************/

void test1()
{
  version (none)
  {
  }
  else
  {
    printf("This should not take a while\n");
    try
    {
	long[] l = new long[ptrdiff_t.max];
	assert(0);
    }
    catch (OutOfMemoryError o)
    {
    }

    printf("This may take a while\n");
    try
    {
	byte[] b = new byte[size_t.max / 3];
	version (Windows)
	    assert(0);
    }
    catch (OutOfMemoryError o)
    {
    }
  }
}

/*******************************************/

void main()
{
    test1();

    printf("Success\n");
}


