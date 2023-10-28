/***
*calloc.c - allocate storage for an array from the heap
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Defines the calloc() function.
*
*******************************************************************************/

#include <malloc.h>
#include <string.h>
#include <winheap.h>
#include <windows.h>
#include <internal.h>
#include <mtdll.h>
#include <dbgint.h>
#include <rtcsup.h>

void * __cdecl _calloc_impl (size_t num, size_t size, int * errno_tmp);

/***
*void *calloc(size_t num, size_t size) - allocate storage for an array from
*       the heap
*
*Purpose:
*       Allocate a block of memory from heap big enough for an array of num
*       elements of size bytes each, initialize all bytes in the block to 0
*       and return a pointer to it.
*
*Entry:
*       size_t num  - number of elements in the array
*       size_t size - size of each element
*
*Exit:
*       Success:  void pointer to allocated block
*       Failure:  NULL
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

void * __cdecl _calloc_base (size_t num, size_t size)
{
    int errno_tmp = 0;
    void * pv = _calloc_impl(num, size, &errno_tmp);

    if ( pv == NULL && errno_tmp != 0 && _errno())
    {
        errno = errno_tmp; // recall, #define errno *_errno()
    }
    return pv;
}
