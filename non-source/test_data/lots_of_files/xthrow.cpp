// exception handling support functions
#include <new>
#include <stdexcept>

_STD_BEGIN
_CRTIMP2_PURE _NO_RETURN(__CLRCALL_PURE_OR_CDECL _Xbad_alloc())
	{	// report a bad_alloc error
	_THROW_NCEE(_XSTD bad_alloc, _EMPTY_ARGUMENT);

	}

_CRTIMP2_PURE _NO_RETURN(__CLRCALL_PURE_OR_CDECL _Xinvalid_argument(_In_z_ const char *_Message))
	{	// report an invalid_argument error
	_THROW_NCEE(invalid_argument, _Message);
	}

_CRTIMP2_PURE _NO_RETURN(__CLRCALL_PURE_OR_CDECL _Xlength_error(_In_z_ const char *_Message))
	{	// report a length_error
	_THROW_NCEE(length_error, _Message);
	}

_CRTIMP2_PURE _NO_RETURN(__CLRCALL_PURE_OR_CDECL _Xout_of_range(_In_z_ const char *_Message))
	{	// report an out_of_range error
	_THROW_NCEE(out_of_range, _Message);
	}

_CRTIMP2_PURE _NO_RETURN(__CLRCALL_PURE_OR_CDECL _Xoverflow_error(_In_z_ const char *_Message))
	{	// report an overflow error
	_THROW_NCEE(overflow_error, _Message);
	}

_CRTIMP2_PURE _NO_RETURN(__CLRCALL_PURE_OR_CDECL _Xruntime_error(_In_z_ const char *_Message))
	{	// report a runtime_error
	_THROW_NCEE(runtime_error, _Message);
	}
_STD_END

 #include <functional>

_STD_BEGIN
_CRTIMP2_PURE _NO_RETURN(__CLRCALL_PURE_OR_CDECL _Xbad_function_call())
	{	// report a bad_function_call error
	_THROW_NCEE(bad_function_call, 0);
	}
_STD_END

 #if _HAS_EXCEPTIONS
 #include <regex>

_STD_BEGIN
_CRTIMP2_PURE _NO_RETURN(__CLRCALL_PURE_OR_CDECL _Xregex_error(regex_constants::error_type _Code))
	{	// report a regex_error
	_THROW_NCEE(regex_error, _Code);
	}
_STD_END
 #endif /* _HAS_EXCEPTIONS */

/*
 * Copyright (c) 1992-2012 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V6.00:0009 */
