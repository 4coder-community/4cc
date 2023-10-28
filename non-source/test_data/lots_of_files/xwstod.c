/* _WStod function */
#include <stdlib.h>
#include <wchar.h>
#include "xmath.h"
#include "xxwctype.h"
#include "xxdftype.h"
_C_STD_BEGIN
 #if !defined(MRTDLL)
_C_LIB_DECL
 #endif /* defined(MRTDLL) */

_CRTIMP2_PURE FTYPE __CLRCALL_PURE_OR_CDECL _WStodx(const CTYPE *s, CTYPE **endptr, long pten,
	int *perr)
	#include "xxstod.h"

_CRTIMP2_PURE FTYPE __CLRCALL_PURE_OR_CDECL _WStod(const CTYPE *s, CTYPE **endptr, long pten)
	{	/* convert string, discard error code */
	return (_WStodx(s, endptr, pten, 0));
	}
 #if !defined(MRTDLL)
_END_C_LIB_DECL
 #endif /* !defined(MRTDLL) */
_C_STD_END

/*
 * Copyright (c) 1992-2012 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V6.00:0009 */
