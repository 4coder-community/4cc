/* _Towlower -- convert wchar_t to lower case for Microsoft */
#include <xlocinfo.h>
#include <wchar.h>
#include <awint.h>
#include <mtdll.h>
#include <setlocal.h>

_C_STD_BEGIN
_CRTIMP2_PURE wchar_t __CLRCALL_PURE_OR_CDECL _Towlower(wchar_t _Ch, const _Ctypevec *_Ctype)
	{	/* convert element to lower case */
	wchar_t _Res = _Ch;

	if (_Ch == WEOF)
		;
	else if (_Ctype->_LocaleName == NULL && _Ch < 256)
		{	/* handle ASCII character in C locale */
		if (L'A' <= _Ch && _Ch <= L'Z')
			_Res = (wchar_t)(_Ch - L'A' + L'a');
		}
	else if (__crtLCMapStringW(_Ctype->_LocaleName, LCMAP_LOWERCASE,
			&_Ch, 1, &_Res, 1) == 0)
		_Res = _Ch;
	return (_Res);
	}

#ifdef MRTDLL
_CRTIMP2_PURE unsigned short __CLRCALL_PURE_OR_CDECL _Towlower(unsigned short _Ch, const _Ctypevec *_Ctype)
    {
    return _Towlower((wchar_t) _Ch, _Ctype);
    }
#endif
_C_STD_END

/*
 * Copyright (c) 1992-2007 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 V5.03:0009 */
