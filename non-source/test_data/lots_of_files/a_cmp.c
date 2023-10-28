/***
*a_cmp.c - A version of CompareString.
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Use either CompareStringA or CompareStringW depending on which is
*       available
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <dbgint.h>
#include <stdlib.h>
#include <locale.h>
#include <awint.h>
#include <dbgint.h>
#include <malloc.h>
#include <awint.h>
#include <mtdll.h>
#include <setlocal.h>

/***
*int __cdecl strncnt - count characters in a string, up to n.
*
*Purpose:
*       Internal local support function. Counts characters in string before NULL.
*       If NULL not found in n chars, then return n.
*
*Entry:
*       const char *string   - start of string
*       int n                - byte count
*
*Exit:
*       returns number of bytes from start of string to
*       NULL (exclusive), up to n.
*
*Exceptions:
*
*******************************************************************************/

static int __cdecl strncnt (
        const char *string,
        int cnt
        )
{
        int n = cnt;
        char *cp = (char *)string;

        while (n-- && *cp)
            cp++;

        return cnt - n - 1;
}

/***
*int __cdecl __crtCompareStringA - Get type information about an ANSI string.
*
*Purpose:
*       Internal support function. Assumes info in ANSI string format. Tries
*       to use NLS API call CompareStringA if available and uses CompareStringW
*       if it must. If neither are available it fails and returns 0.
*
*Entry:
*       LPCWSTR LocaleName  - locale context for the comparison.
*       DWORD   dwCmpFlags  - see NT\Chicago docs
*       LPCSTR  lpStringn   - multibyte string to be compared
*       int     cchCountn   - char (byte) count (NOT including NULL)
*                             (-1 if NULL terminated)
*       int     code_page   - for MB/WC conversion. If 0, use __lc_codepage
*
*Exit:
*       Success: 1 - if lpString1 <  lpString2
*                2 - if lpString1 == lpString2
*                3 - if lpString1 >  lpString2
*       Failure: 0
*
*Exceptions:
*
*******************************************************************************/

static int __cdecl __crtCompareStringA_stat(
        _locale_t plocinfo,
        LPCWSTR  LocaleName,
        DWORD    dwCmpFlags,
        LPCSTR   lpString1,
        int      cchCount1,
        LPCSTR   lpString2,
        int      cchCount2,
        int      code_page
        )
{
    /*
     * CompareString will compare past NULL. Must find NULL if in string
     * before cchCountn chars.
     */

    if (cchCount1 > 0)
        cchCount1 = strncnt(lpString1, cchCount1);
    else if ( cchCount1 < -1 )
        return FALSE;
    if (cchCount2 > 0)
        cchCount2 = strncnt(lpString2, cchCount2);
    else if ( cchCount2 < -1 )
        return FALSE;


    int buff_size1;
    int buff_size2;
    wchar_t *wbuffer1;
    wchar_t *wbuffer2;
    int retcode = 0;

    /*
     * Use __lc_codepage for conversion if code_page not specified
     */

    if (0 == code_page)
        code_page = plocinfo->locinfo->lc_codepage;

    /*
     * Special case: at least one count is zero
     */

    if (!cchCount1 || !cchCount2)
    {
        unsigned char *cp;  // char pointer
        CPINFO cpInfo;      // struct for use with GetCPInfo

        /* both strings zero */
        if (cchCount1 == cchCount2)
            return 2;

        /* string 1 greater */
        if (cchCount2 > 1)
            return 1;

        /* string 2 greater */
        if (cchCount1 > 1)
            return 3;

        /*
         * one has zero count, the other has a count of one
         * - if the one count is a naked lead byte, the strings are equal
         * - otherwise it is a single character and they are unequal
         */

        if (GetCPInfo(code_page, &cpInfo) == FALSE)
            return 0;

        _ASSERTE(cchCount1==0 && cchCount2==1 || cchCount1==1 && cchCount2==0);

        /* string 1 has count of 1 */
        if (cchCount1 > 0)
        {
            if (cpInfo.MaxCharSize < 2)
                return 3;

            for ( cp = (unsigned char *)cpInfo.LeadByte ;
                  cp[0] && cp[1] ;
                  cp += 2 )
                if ( (*(unsigned char *)lpString1 >= cp[0]) &&
                     (*(unsigned char *)lpString1 <= cp[1]) )
                    return 2;

            return 3;
        }

        /* string 2 has count of 1 */
        if (cchCount2 > 0)
        {
            if (cpInfo.MaxCharSize < 2)
            return 1;

            for ( cp = (unsigned char *)cpInfo.LeadByte ;
                  cp[0] && cp[1] ;
                  cp += 2 )
                if ( (*(unsigned char *)lpString2 >= cp[0]) &&
                     (*(unsigned char *)lpString2 <= cp[1]) )
                    return 2;

            return 1;
        }
    }

    /*
     * Convert strings and return the requested information.
     */

    /* find out how big a buffer we need (includes NULL if any) */
    if ( 0 == (buff_size1 = MultiByteToWideChar( code_page,
                                                 MB_PRECOMPOSED |
                                                    MB_ERR_INVALID_CHARS,
                                                 lpString1,
                                                 cchCount1,
                                                 NULL,
                                                 0 )) )
        return 0;

    /* allocate enough space for chars */
    wbuffer1 = (wchar_t *)_calloca( buff_size1, sizeof(wchar_t) );
    if ( wbuffer1 == NULL ) {
        return 0;
    }

    /* do the conversion */
    if ( 0 == MultiByteToWideChar( code_page,
                                   MB_PRECOMPOSED,
                                   lpString1,
                                   cchCount1,
                                   wbuffer1,
                                   buff_size1 ) )
        goto error_cleanup;

    /* find out how big a buffer we need (includes NULL if any) */
    if ( 0 == (buff_size2 = MultiByteToWideChar( code_page,
                                                 MB_PRECOMPOSED |
                                                    MB_ERR_INVALID_CHARS,
                                                 lpString2,
                                                 cchCount2,
                                                 NULL,
                                                 0 )) )
        goto error_cleanup;

    /* allocate enough space for chars */
    wbuffer2 = (wchar_t *)_calloca( buff_size2, sizeof(wchar_t) );
    if ( wbuffer2 == NULL ) {
        goto error_cleanup;
    }

    /* do the conversion */
    if ( 0 != MultiByteToWideChar( code_page,
                                   MB_PRECOMPOSED,
                                   lpString2,
                                   cchCount2,
                                   wbuffer2,
                                   buff_size2 ) )
    {
        retcode = __crtCompareStringEx( LocaleName,
                                  dwCmpFlags,
                                  wbuffer1,
                                  buff_size1,
                                  wbuffer2,
                                  buff_size2);
    }

    _freea(wbuffer2);

error_cleanup:
    _freea(wbuffer1);

    return retcode;
}

extern "C" int __cdecl __crtCompareStringA(
        _locale_t plocinfo,
        LPCWSTR  LocaleName,
        DWORD    dwCmpFlags,
        LPCSTR   lpString1,
        int      cchCount1,
        LPCSTR   lpString2,
        int      cchCount2,
        int      code_page
        )
{
    _LocaleUpdate _loc_update(plocinfo);

    return __crtCompareStringA_stat(
            _loc_update.GetLocaleT(),
            LocaleName,
            dwCmpFlags,
            lpString1,
            cchCount1,
            lpString2,
            cchCount2,
            code_page
            );
}
