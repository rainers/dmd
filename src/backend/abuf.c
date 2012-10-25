
#include "abuf.h"

static char __file__[] = __FILE__;      /* for tassert.h                */
#include "tassert.h"

#include <string.h>

TypeInfo_Abuf ti_abuf;

const char* TypeInfo_Abuf::toString()
{
    return "Abuf";
}

hash_t TypeInfo_Abuf::getHash(void *p)
{
    Abuf a = *(Abuf *)p;

    hash_t hash = 0;
    for (size_t i = 0; i < a.length; i++)
        hash = hash * 11 + a.buf[i];

    return hash;
}

int TypeInfo_Abuf::equals(void *p1, void *p2)
{
    Abuf a1 = *(Abuf*)p1;
    Abuf a2 = *(Abuf*)p2;

    return a1.length == a2.length &&
        memcmp(a1.buf, a2.buf, a1.length) == 0;
}

int TypeInfo_Abuf::compare(void *p1, void *p2)
{
    Abuf a1 = *(Abuf*)p1;
    Abuf a2 = *(Abuf*)p2;

    if (a1.length == a2.length)
        return memcmp(a1.buf, a2.buf, a1.length);
    else if (a1.length < a2.length)
        return -1;
    else
        return 1;
}

size_t TypeInfo_Abuf::tsize()
{
    return sizeof(Abuf);
}

void TypeInfo_Abuf::swap(void *p1, void *p2)
{
    assert(0);
}

