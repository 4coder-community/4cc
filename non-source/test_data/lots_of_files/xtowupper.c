/* _Towupper -- convert wchar_t to upper case for Microsoft */
#include <xlocinfo.h>
#include <wchar.h>
#include <awint.h>
#include <mtdll.h>
#include <setlocal.h>

_C_STD_BEGIN
_CRTIMP2_PURE wchar_t __CLRCALL_PURE_OR_CDECL _Towupper(wchar_t _Ch,
	const _Ctypevec *_Ctype)
	{	/* convert element to upper case */
	wchar_t _Res = _Ch;

	if (_Ch == WEOF)
		;
	else if (_Ctype->_LocaleName == NULL && _Ch < 256)
		{	/* handle ASCII character in C locale */
		if (L'a' <= _Ch && _Ch <= L'z')
			_Res = (wchar_t)(_Ch - L'a' + L'A');
		}
	else if (__crtLCMapStringW(_Ctype->_LocaleName, LCMAP_UPPERCASE,
			&_Ch, 1, &_Res, 1) == 0)
		_Res = _Ch;
	return (_Res);
	}
#ifdef MRTDLL
_CRTIMP2_PURE unsigned short __CLRCALL_PURE_OR_CDECL _Towupper(unsigned short _Ch,
	const _Ctypevec *_Ctype)
    {
    return _Towupper((wchar_t) _Ch, _Ctype);
    }
#endif
_C_STD_END

/*
 * Copyright (c) 1992-2007 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 V5.03:0009 */
