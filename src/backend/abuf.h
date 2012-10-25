#ifndef ABUF_H
#define ABUF_H 1

#include "tinfo.h"

struct Abuf
{
    const unsigned char *buf;
    size_t length;
};

struct TypeInfo_Abuf : TypeInfo
{
    const char* toString();
    hash_t getHash(void *p);
    int equals(void *p1, void *p2);
    int compare(void *p1, void *p2);
    size_t tsize();
    void swap(void *p1, void *p2);
};

extern TypeInfo_Abuf ti_abuf;

#endif // ABUF_H

