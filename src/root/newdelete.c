
/* Copyright (c) 2000-2014 by Digital Mars
 * All Rights Reserved, written by Walter Bright
 * http://www.digitalmars.com
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
 * https://github.com/D-Programming-Language/dmd/blob/master/src/root/rmem.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__has_feature)
# if __has_feature(address_sanitizer)
# define USE_ASAN_NEW_DELETE
# endif
#elif defined(__SANITIZE_ADDRESS__)
# define USE_ASAN_NEW_DELETE
#endif

#if !defined(USE_ASAN_NEW_DELETE)

#if 1

void *allocmemory(size_t m_size);

void * operator new(size_t m_size)
{
#ifdef free
    return malloc(m_size);
#else
    return allocmemory(m_size);
#endif
}

void operator delete(void *p)
{
#ifdef free
//    free(p);
#endif
}

#else

void * operator new(size_t m_size)
{
    void *p = malloc(m_size);
    if (p)
        return p;
    printf("Error: out of memory\n");
    exit(EXIT_FAILURE);
    return p;
}

void operator delete(void *p)
{
    free(p);
}

#endif

#endif

#ifdef free

extern "C"
{
    void* __cdecl gc_malloc(size_t sz, unsigned int attr, void* ti);
    void* __cdecl gc_calloc(size_t sz, unsigned int attr, void* ti);
    void* __cdecl gc_realloc(void* p, size_t sz, unsigned int attr, void* ti);
    void* __cdecl gc_free(void* p);

    __declspec(noalias) __declspec(restrict) void* __cdecl _d_gc_malloc(size_t sz)
    {
        return gc_malloc(sz, 0, NULL);
    }
    __declspec(noalias) __declspec(restrict) void* __cdecl _d_gc_calloc(size_t cnt, size_t sz)
    {
        if (cnt * sz == 0)
            return "";
        return gc_calloc(cnt * sz, 0, NULL);
    }
    __declspec(noalias) __declspec(restrict) void* __cdecl _d_gc_realloc(void* p, size_t sz)
    {
        return gc_realloc(p, sz, 0, NULL);
    }

    __declspec(noalias) void __cdecl _d_gc_free(void *p)
    {
        gc_free(p);
    }
}
#endif

