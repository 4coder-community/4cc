/***
*calloc_impl.c - implementation of _calloc_impl
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Defines the _calloc_impl() function.
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

void * __cdecl _calloc_impl (size_t num, size_t size, int * errno_tmp)
{
        size_t  size_orig;
        void *  pvReturn;

        /* ensure that (size * num) does not overflow */
        if (num > 0)
        {
            _VALIDATE_RETURN_NOEXC((_HEAP_MAXREQ / num) >= size, ENOMEM, NULL);
        }
        size_orig = size = size * num;


        /* force nonzero size */
        if (size == 0)
            size = 1;

        for (;;)
        {
            pvReturn = NULL;

            if (size <= _HEAP_MAXREQ)
            {
                if (pvReturn == NULL)
                    pvReturn = HeapAlloc(_crtheap, HEAP_ZERO_MEMORY, size);
            }

            if (pvReturn || _newmode == 0)
            {
                RTCCALLBACK(_RTC_Allocate_hook, (pvReturn, size_orig, 0));
                if (pvReturn == NULL)
                {
                    if ( errno_tmp )
                        *errno_tmp = ENOMEM;
                }
                return pvReturn;
            }

            /* call installed new handler */
            if (!_callnewh(size))
            {
                if ( errno_tmp )
                    *errno_tmp = ENOMEM;
                return NULL;
            }

            /* new handler was successful -- try to allocate again */
        }

}
