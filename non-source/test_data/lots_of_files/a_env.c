/***
*a_env.c - A version of GetEnvironmentStrings.
*
*       Copyright (c) Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       Use GetEnvironmentStringsW if available, otherwise use A version.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <setlocal.h>
#include <awint.h>
#include <dbgint.h>

/***
*LPVOID __cdecl __crtGetEnvironmentStringsA - Get normal environment block
*
*Purpose:
*       Internal support function. Since GetEnvironmentStrings returns in OEM
*       and we want ANSI (note that GetEnvironmentVariable returns ANSI!) and
*       SetFileApistoAnsi() does not affect it, we have no choice but to
*       obtain the block in wide character and convert to ANSI.
*
*Entry:
*       VOID
*
*Exit:
*       LPVOID - pointer to environment block
*
*Exceptions:
*
*******************************************************************************/

LPVOID __cdecl __crtGetEnvironmentStringsA(
        VOID
        )
{
        wchar_t *wEnv;
        wchar_t *wTmp;
        char *aEnv = NULL;
        int nSizeW;
        int nSizeA;

        /* obtain wide environment block */

        if ( NULL == (wEnv = GetEnvironmentStringsW()) )
            return NULL;

        /* look for double null that indicates end of block */
        wTmp = wEnv;
        while ( *wTmp != L'\0' ) {
            if ( *++wTmp == L'\0' )
                wTmp++;
        }

        /* calculate total size of block, including all nulls */
        nSizeW = (int)(wTmp - wEnv + 1);

        /* find out how much space needed for multi-byte environment */
        nSizeA = WideCharToMultiByte(   CP_ACP,
                                        0,
                                        wEnv,
                                        nSizeW,
                                        NULL,
                                        0,
                                        NULL,
                                        NULL );

        /* allocate space for multi-byte string */
        if ( (nSizeA == 0) ||
             ((aEnv = (char *)_malloc_crt(nSizeA)) == NULL) )
        {
            FreeEnvironmentStringsW( wEnv );
            return NULL;
        }

        /* do the conversion */
        if ( !WideCharToMultiByte(  CP_ACP,
                                    0,
                                    wEnv,
                                    nSizeW,
                                    aEnv,
                                    nSizeA,
                                    NULL,
                                    NULL ) )
        {
            _free_crt( aEnv );
            aEnv = NULL;
        }

        FreeEnvironmentStringsW( wEnv );
        return aEnv;
}
