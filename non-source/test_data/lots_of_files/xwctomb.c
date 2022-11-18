/***
*xwctomb.c - Convert wide character to multibyte character, with locale.
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Convert a wide character into the equivalent multibyte character.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdlib.h>
#include <mtdll.h>
#include <errno.h>
#include <limits.h>             /* for MB_LEN_MAX */
#include <string.h>             /* for memcpy */
#include <stdio.h>              /* for EOF */
#include <xlocinfo.h>           /* for _Cvtvec, _Wcrtomb */
#include <locale.h>
#include <setlocal.h>
#include <internal.h>
#include <mbctype.h>

/***
*int _Wcrtomb() - Convert wide character to multibyte character.
*
*Purpose:
*       Convert a wide character into the equivalent multi-byte character,
*       according to the specified LC_CTYPE category, or the current locale.
*       [ANSI].
*
*       NOTE:  Currently, the C libraries support the "C" locale only.
*              Non-C locale support now available under _INTL switch.
*Entry:
*       char *s             = pointer to multibyte character
*       wchar_t wchar       = source wide character
*       mbstate_t *pst      = pointer to state (not used)
*       const _Cvtvec *ploc = pointer to locale info
*
*Exit:
*       Returns:
*      -1 (if error) or number of bytes comprising converted mbc
*
*Exceptions:
*
*******************************************************************************/

/* Retained for backward compatibility of DLL exports only */
_CRTIMP2_PURE int __CLRCALL_PURE_OR_CDECL __Wcrtomb_lk
        (
        char *s,
        wchar_t wchar,
        mbstate_t *pst,
        const _Cvtvec *ploc
        )
{
        return _Wcrtomb(s, wchar, pst, ploc);
}

_CRTIMP2_PURE int __CLRCALL_PURE_OR_CDECL _Wcrtomb
        (
        char *s,
        wchar_t wchar,
        mbstate_t *pst,
        const _Cvtvec *ploc
        )
{
        if (ploc->_Isclocale)
        {
            if ( wchar > 255 )  /* validate high byte */
            {
                errno = EILSEQ;
                return -1;
            }

            *s = (char) wchar;
            return sizeof(char);
        } else {
            int size;
            BOOL defused = 0;
                        _Cvtvec cvtvec;

            if (ploc == 0)
            {
                cvtvec = _Getcvt();
                ploc = &cvtvec;
            }

            if ( ((size = WideCharToMultiByte(ploc->_Page,
                                              0,
                                              &wchar,
                                              1,
                                              s,
                                              ploc->_Mbcurmax,
                                              NULL,
                                              &defused)) == 0) ||
                 (defused) )
            {
                errno = EILSEQ;
                return -1;
            }

            return size;
        }
}

#ifdef MRTDLL
_CRTIMP2_PURE int __CLRCALL_PURE_OR_CDECL _Wcrtomb
        (
        char *s,
        unsigned short wchar,
        mbstate_t *pst,
        const _Cvtvec *ploc
        )
    {
    return _Wcrtomb(s,(wchar_t) wchar, pst, ploc);
    }
#endif  /* MRTDLL */

/***
*_Cvtvec _Getcvt() - get conversion info for current locale
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

_CRTIMP2_PURE _Cvtvec __CLRCALL_PURE_OR_CDECL _Getcvt()
{
        _Cvtvec cvt = {0};
        int idx;

        cvt._Page = ___lc_codepage_func();
        cvt._Mbcurmax = ___mb_cur_max_func();
        cvt._Isclocale = ___lc_locale_name_func()[LC_CTYPE] == NULL;

        if (!cvt._Isclocale)
            for (idx = 0; idx < 256; ++idx)
                if (_ismbblead(idx))
                    cvt._Isleadbyte[idx >> 3] |= 1 << (idx & 7);
        return (cvt);
}
