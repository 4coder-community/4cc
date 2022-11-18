/***
*a_loc.c - A versions of GetLocaleInfo.
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Use either GetLocaleInfoA or GetLocaleInfoW depending on which is
*       available
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <awint.h>
#include <dbgint.h>
#include <malloc.h>
#include <locale.h>
#include <mtdll.h>
#include <setlocal.h>

/***
*int __cdecl __crtGetLocaleInfoA - Get locale info and return it as an ASCII
*       string
*
*Purpose:
*       Internal support function. Assumes info in ANSI string format. Tries
*       to use NLS API call GetLocaleInfoA if available (Chicago) and uses
*       GetLocaleInfoA if it must (NT). If neither are available it fails and
*       returns 0.
*
*Entry:
*       LPCWSTR  LocaleName  - locale context for the comparison.
*       LCTYPE   LCType      - see NT\Chicago docs
*       LPSTR    lpLCData    - pointer to memory to return data
*       int      cchData     - char (byte) count of buffer (including NULL)
*                              (if 0, lpLCData is not referenced, size needed
*                              is returned)
*       int      code_page   - for MB/WC conversion. If 0, use __lc_codepage
*
*Exit:
*       Success: the number of characters copied (including NULL).
*       Failure: 0
*
*Exceptions:
*
*******************************************************************************/

static int __cdecl __crtGetLocaleInfoA_stat(
        _locale_t plocinfo,
        const wchar_t* LocaleName,
        LCTYPE  LCType,
        LPSTR   lpLCData,
        int     cchData
        )
{
    int retval = 0;
    int code_page;
    int buff_size;
    wchar_t *wbuffer;

    /*
     * Use __lc_codepage for conversion
     */

    code_page = plocinfo->locinfo->lc_codepage;

    /* find out how big buffer needs to be */
    if (0 == (buff_size = __crtGetLocaleInfoEx(LocaleName, LCType, NULL, 0)))
        return 0;

    /* allocate buffer */
    wbuffer = (wchar_t *)_calloca( buff_size, sizeof(wchar_t) );
    if ( wbuffer == NULL ) {
        return 0;
    }

    /* get the info in wide format */
    if (0 == __crtGetLocaleInfoEx(LocaleName, LCType, wbuffer, buff_size))
        goto error_cleanup;

    /* convert from Wide Char to ANSI */
    if (0 == cchData)
    {
        /* convert into local buffer */
        retval = WideCharToMultiByte( code_page,
                                      0,
                                      wbuffer,
                                      -1,
                                      NULL,
                                      0,
                                      NULL,
                                      NULL );
    }
    else {
        /* convert into user buffer */
        retval = WideCharToMultiByte( code_page,
                                      0,
                                      wbuffer,
                                      -1,
                                      lpLCData,
                                      cchData,
                                      NULL,
                                      NULL );
    }

error_cleanup:
    _freea(wbuffer);

    return retval;
}

extern "C" int __cdecl __crtGetLocaleInfoA(
        _locale_t plocinfo,
        const wchar_t*  LocaleName,
        LCTYPE  LCType,
        LPSTR   lpLCData,
        int     cchData
        )
{
    _LocaleUpdate _loc_update(plocinfo);

    return __crtGetLocaleInfoA_stat(
            _loc_update.GetLocaleT(),
            LocaleName,
            LCType,
            lpLCData,
            cchData
            );
}
