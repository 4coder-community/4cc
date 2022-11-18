/***
*xwcscoll.c - Collate wide-character locale strings
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Compare two wchar_t strings using the locale LC_COLLATE information.
*
*******************************************************************************/


#include <cruntime.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <mtdll.h>
#include <setlocal.h>
#include <mtdll.h>
#include <errno.h>
#include <awint.h>
#include <xlocinfo.h>   /* for _Collvec, _Wcscoll */

/***
*static int _Wmemcmp(s1, s2, n) - compare wchar_t s1[n], s2[n]
*
*Purpose:
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

static int __CLRCALL_PURE_OR_CDECL _Wmemcmp(
        const wchar_t *s1,
        const wchar_t *s2,
        int n
        )
{
        for (; 0 < n; ++s1, ++s2, --n)
             if (*s1 != *s2)
               return (*s1 < *s2 ? -1 : +1);
        return (0);
}

/***
*int _Wcscoll() - Collate wide-character locale strings
*
*Purpose:
*       Compare two wchar_t strings using the locale LC_COLLATE information.
*       In the C locale, wcscmp() is used to make the comparison.
*
*Entry:
*       const wchar_t *_string1 = pointer to beginning of the first string
*       const wchar_t *_end1    = pointer past end of the first string
*       const wchar_t *_string2 = pointer to beginning of the second string
*       const wchar_t *_end2    = pointer past end of the second string
*       const _Collvec *ploc = pointer to locale info
*
*Exit:
*       -1 = first string less than second string
*        0 = strings are equal
*        1 = first string greater than second string
*       This range of return values may differ from other *cmp/*coll functions.
*
*Exceptions:
*       _NLSCMPERROR    = error
*       errno = EINVAL
*
*******************************************************************************/

_CRTIMP2_PURE int __CLRCALL_PURE_OR_CDECL _Wcscoll (
        const wchar_t *_string1,
        const wchar_t *_end1,
        const wchar_t *_string2,
        const wchar_t *_end2,
        const _Collvec *ploc
        )
{
        int n1 = (int)(_end1 - _string1);
        int n2 = (int)(_end2 - _string2);
        int ret=0;
        const wchar_t *locale_name;

        if (ploc == 0)
            locale_name = ___lc_locale_name_func()[LC_COLLATE];
        else
        {
            locale_name = ploc->_LocaleName;
        }

        if (locale_name == NULL)
        {
            int ans;
            ans = _Wmemcmp(_string1, _string2, n1 < n2 ? n1 : n2);
            ret=(ans != 0 || n1 == n2 ? ans : n1 < n2 ? -1 : +1);
        }
        else
        {
            if (0 == (ret = __crtCompareStringW(locale_name,
                                                SORT_STRINGSORT,
                                                _string1,
                                                n1,
                                                _string2,
                                                n2)))
            {
                errno = EINVAL;
                ret=_NLSCMPERROR;
            }
            else
            {
                ret-=2;
            }
        }

        return ret;
}

#ifdef MRTDLL
_CRTIMP2_PURE int __CLRCALL_PURE_OR_CDECL _Wcscoll (
        const unsigned short *_string1,
        const unsigned short *_end1,
        const unsigned short *_string2,
        const unsigned short *_end2,
        const _Collvec *ploc
        )
    {
    return _Wcscoll(
            (const wchar_t *)_string1,
            (const wchar_t *)_end1,
            (const wchar_t *)_string2,
            (const wchar_t *)_end2,
            ploc);
    }
#endif  /* MRTDLL */
