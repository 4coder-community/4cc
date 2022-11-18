/***
*complex.h - definitions and declarations for complex math library
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file contains constant definitions and external subroutine
*       declarations for the complex math subroutine library.
*
*       [Public]
*
****/

#pragma once

#ifndef _COMPLEX
#define _COMPLEX

#include <crtdefs.h>

/*
 * Currently, all MS C compilers for Win32 platforms default to 8 byte
 * alignment.
 */
#pragma pack(push,_CRT_PACKING)

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define _DCOMPLEX_(re, im)  _Cbuild(re, im)
#define _FCOMPLEX_(re, im)  _FCbuild(re, im)
#define _LCOMPLEX_(re, im)  _LCbuild(re, im)

#ifndef _C_COMPLEX_T
#define _C_COMPLEX_T

typedef struct _C_double_complex
{	/* double complex */
    double _Val[2];
} _C_double_complex;

typedef struct _C_float_complex
{	/* float complex */
    float _Val[2];
} _C_float_complex;

typedef struct _C_ldouble_complex
{	/* long double complex */
    long double _Val[2];
} _C_ldouble_complex;

#endif /* _C_COMPLEX_T */

typedef _C_double_complex  _Dcomplex;
typedef _C_float_complex   _Fcomplex;
typedef _C_ldouble_complex _Lcomplex;

#define _Complex_I  _FCbuild(0.0F, 1.0F)

_CRTIMP double __cdecl cimag(_In_ _Dcomplex _Z);
_CRTIMP double __cdecl creal(_In_ _Dcomplex _Z);
_CRTIMP float __cdecl cimagf(_In_ _Fcomplex _Z);
_CRTIMP float __cdecl crealf(_In_ _Fcomplex _Z);
_CRTIMP long double __cdecl cimagl(_In_ _Lcomplex _Z);
_CRTIMP long double __cdecl creall(_In_ _Lcomplex _Z);

/* MACROS */

#define I  _Complex_I

/* FUNCTIONS */
_CRTIMP double __cdecl cabs(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl cacos(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl cacosh(_In_ _Dcomplex _Z);
_CRTIMP double __cdecl carg(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl casin(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl casinh(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl catan(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl catanh(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl ccos(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl ccosh(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl cexp(_In_ _Dcomplex _Z);
_CRTIMP double __cdecl cimag(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl clog(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl clog10(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl conj(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl cpow(_In_ _Dcomplex _X, _In_ _Dcomplex _Y);
_CRTIMP _Dcomplex __cdecl cproj(_In_ _Dcomplex _Z);
_CRTIMP double __cdecl creal(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl csin(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl csinh(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl csqrt(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl ctan(_In_ _Dcomplex _Z);
_CRTIMP _Dcomplex __cdecl ctanh(_In_ _Dcomplex _Z);
_CRTIMP double __cdecl norm(_In_ _Dcomplex _Z);

_CRTIMP float __cdecl cabsf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl cacosf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl cacoshf(_In_ _Fcomplex _Z);
_CRTIMP float __cdecl cargf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl casinf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl casinhf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl catanf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl catanhf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl ccosf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl ccoshf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl cexpf(_In_ _Fcomplex _Z);
_CRTIMP float __cdecl cimagf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl clogf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl clog10f(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl conjf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl cpowf(_In_ _Fcomplex _X, _In_ _Fcomplex _Y);
_CRTIMP _Fcomplex __cdecl cprojf(_In_ _Fcomplex _Z);
_CRTIMP float __cdecl crealf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl csinf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl csinhf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl csqrtf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl ctanf(_In_ _Fcomplex _Z);
_CRTIMP _Fcomplex __cdecl ctanhf(_In_ _Fcomplex _Z);
_CRTIMP float __cdecl normf(_In_ _Fcomplex _Z);

_CRTIMP long double __cdecl cabsl(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl cacosl(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl cacoshl(_In_ _Lcomplex _Z);
_CRTIMP long double __cdecl cargl(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl casinl(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl casinhl(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl catanl(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl catanhl(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl ccosl(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl ccoshl(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl cexpl(_In_ _Lcomplex _Z);
_CRTIMP long double __cdecl cimagl(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl clogl(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl clog10l(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl conjl(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl cpowl(_In_ _Lcomplex _X, _In_ _Lcomplex _Y);
_CRTIMP _Lcomplex __cdecl cprojl(_In_ _Lcomplex _Z);
_CRTIMP long double __cdecl creall(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl csinl(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl csinhl(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl csqrtl(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl ctanl(_In_ _Lcomplex _Z);
_CRTIMP _Lcomplex __cdecl ctanhl(_In_ _Lcomplex _Z);
_CRTIMP long double __cdecl norml(_In_ _Lcomplex _Z);

_CRTIMP _Dcomplex __cdecl _Cbuild(_In_ double _Re, _In_ double _Im);
_CRTIMP _Dcomplex __cdecl _Cmulcc(_In_ _Dcomplex _X, _In_ _Dcomplex _Y);
_CRTIMP _Dcomplex __cdecl _Cmulcr(_In_ _Dcomplex _X, _In_ double _Y);

_CRTIMP _Fcomplex __cdecl _FCbuild(_In_ float _Re, _In_ float _Im);
_CRTIMP _Fcomplex __cdecl _FCmulcc(_In_ _Fcomplex _X, _In_ _Fcomplex _Y);
_CRTIMP _Fcomplex __cdecl _FCmulcr(_In_ _Fcomplex _X, _In_ float _Y);

_CRTIMP _Lcomplex __cdecl _LCbuild(_In_ long double _Re, _In_ long double _Im);
_CRTIMP _Lcomplex __cdecl _LCmulcc(_In_ _Lcomplex _X, _In_ _Lcomplex _Y);
_CRTIMP _Lcomplex __cdecl _LCmulcr(_In_ _Lcomplex _X, _In_ long double _Y);

#ifdef __cplusplus
}

extern "C++"
{
    // double complex OVERLOADS
    inline _Dcomplex __CRTDECL acos(_In_ _Dcomplex _X) throw()
    {	// compute cacos
        return (cacos(_X));
    }

    inline _Dcomplex __CRTDECL acosh(_In_ _Dcomplex _X) throw()
    {	// compute cacosh
        return (cacosh(_X));
    }

    inline _Dcomplex __CRTDECL asin(_In_ _Dcomplex _X) throw()
    {	// compute casin
        return (casin(_X));
    }

    inline _Dcomplex __CRTDECL asinh(_In_ _Dcomplex _X) throw()
    {	// compute casinh
        return (casinh(_X));
    }

    inline _Dcomplex __CRTDECL atan(_In_ _Dcomplex _X) throw()
    {	// compute catan
        return (catan(_X));
    }

    inline _Dcomplex __CRTDECL atanh(_In_ _Dcomplex _X) throw()
    {	// compute catanh
        return (catanh(_X));
    }

    inline _Dcomplex __CRTDECL cos(_In_ _Dcomplex _X) throw()
    {	// compute ccos
        return (ccos(_X));
    }

    inline _Dcomplex __CRTDECL cosh(_In_ _Dcomplex _X) throw()
    {	// compute ccosh
        return (ccosh(_X));
    }

    inline _Dcomplex __CRTDECL proj(_In_ _Dcomplex _X) throw()
    {	// compute cproj
        return (cproj(_X));
    }

    inline _Dcomplex __CRTDECL exp(_In_ _Dcomplex _X) throw()
    {	// compute cexp
        return (cexp(_X));
    }

    inline _Dcomplex __CRTDECL log(_In_ _Dcomplex _X) throw()
    {	// compute clog
        return (clog(_X));
    }

    inline _Dcomplex __CRTDECL log10(_In_ _Dcomplex _X) throw()
    {	// compute clog10
        return (clog10(_X));
    }

    inline _Dcomplex __CRTDECL pow(_In_ _Dcomplex _X, _In_ _Dcomplex _Y) throw()
    {	// compute cpow
        return (cpow(_X, _Y));
    }

    inline _Dcomplex __CRTDECL sin(_In_ _Dcomplex _X) throw()
    {	// compute csin
        return (csin(_X));
    }

    inline _Dcomplex __CRTDECL sinh(_In_ _Dcomplex _X) throw()
    {	// compute csinh
        return (csinh(_X));
    }

    inline _Dcomplex __CRTDECL sqrt(_In_ _Dcomplex _X) throw()
    {	// compute csqrt
        return (csqrt(_X));
    }

    inline _Dcomplex __CRTDECL tan(_In_ _Dcomplex _X) throw()
    {	// compute ctan
        return (ctan(_X));
    }

    inline _Dcomplex __CRTDECL tanh(_In_ _Dcomplex _X) throw()
    {	// compute ctanh
        return (ctanh(_X));
    }

    inline double __CRTDECL abs(_In_ _Dcomplex _X) throw()
    {	// compute cabs
        return (cabs(_X));
    }

    inline double __CRTDECL arg(_In_ _Dcomplex _X) throw()
    {	// compute carg
        return (carg(_X));
    }

    inline double __CRTDECL imag(_In_ _Dcomplex _X) throw()
    {	// compute cimag
        return (cimag(_X));
    }

    inline double __CRTDECL real(_In_ _Dcomplex _X) throw()
    {	// compute creal
        return (creal(_X));
    }

    // float complex OVERLOADS
    inline _Fcomplex __CRTDECL acos(_In_ _Fcomplex _X) throw()
    {	// compute cacos
        return (cacosf(_X));
    }

    inline _Fcomplex __CRTDECL acosh(_In_ _Fcomplex _X) throw()
    {	// compute cacosh
        return (cacoshf(_X));
    }

    inline _Fcomplex __CRTDECL asin(_In_ _Fcomplex _X) throw()
    {	// compute casin
        return (casinf(_X));
    }

    inline _Fcomplex __CRTDECL asinh(_In_ _Fcomplex _X) throw()
    {	// compute casinh
        return (casinhf(_X));
    }

    inline _Fcomplex __CRTDECL atan(_In_ _Fcomplex _X) throw()
    {	// compute catan
        return (catanf(_X));
    }

    inline _Fcomplex __CRTDECL atanh(_In_ _Fcomplex _X) throw()
    {	// compute catanh
        return (catanhf(_X));
    }

    inline _Fcomplex __CRTDECL conj(_In_ _Fcomplex _X) throw()
    {	// compute conj
        return (conjf(_X));
    }

    inline _Fcomplex __CRTDECL cos(_In_ _Fcomplex _X) throw()
    {	// compute ccos
        return (ccosf(_X));
    }

    inline _Fcomplex __CRTDECL cosh(_In_ _Fcomplex _X) throw()
    {	// compute ccosh
        return (ccoshf(_X));
    }

    inline _Fcomplex __CRTDECL cproj(_In_ _Fcomplex _X) throw()
    {	// compute cproj
        return (cprojf(_X));
    }

    inline _Fcomplex __CRTDECL proj(_In_ _Fcomplex _X) throw()
    {	// compute cproj
        return (cprojf(_X));
    }

    inline _Fcomplex __CRTDECL exp(_In_ _Fcomplex _X) throw()
    {	// compute cexp
        return (cexpf(_X));
    }

    inline _Fcomplex __CRTDECL log(_In_ _Fcomplex _X) throw()
    {	// compute clog
        return (clogf(_X));
    }

    inline _Fcomplex __CRTDECL log10(_In_ _Fcomplex _X) throw()
    {	// compute clog10
        return (clog10f(_X));
    }

    inline float __CRTDECL norm(_In_ _Fcomplex _X) throw()
    {	// compute norm
        return (normf(_X));
    }

    inline _Fcomplex __CRTDECL pow(_In_ _Fcomplex _X, _In_ _Fcomplex _Y) throw()
    {	// compute cpow
        return (cpowf(_X, _Y));
    }

    inline _Fcomplex __CRTDECL sin(_In_ _Fcomplex _X) throw()
    {	// compute csin
        return (csinf(_X));
    }

    inline _Fcomplex __CRTDECL sinh(_In_ _Fcomplex _X) throw()
    {	// compute csinh
        return (csinhf(_X));
    }

    inline _Fcomplex __CRTDECL sqrt(_In_ _Fcomplex _X) throw()
    {	// compute csqrt
        return (csqrtf(_X));
    }

    inline _Fcomplex __CRTDECL tan(_In_ _Fcomplex _X) throw()
    {	// compute ctan
        return (ctanf(_X));
    }

    inline _Fcomplex __CRTDECL tanh(_In_ _Fcomplex _X) throw()
    {	// compute ctanh
        return (ctanhf(_X));
    }

    inline float __CRTDECL abs(_In_ _Fcomplex _X) throw()
    {	// compute cabs
        return (cabsf(_X));
    }

    inline float __CRTDECL arg(_In_ _Fcomplex _X) throw()
    {	// compute carg
        return (cargf(_X));
    }

    inline float __CRTDECL carg(_In_ _Fcomplex _X) throw()
    {	// compute carg
        return (cargf(_X));
    }

    inline float __CRTDECL cimag(_In_ _Fcomplex _X) throw()
    {	// compute cimag
        return (cimagf(_X));
    }

    inline float __CRTDECL creal(_In_ _Fcomplex _X) throw()
    {	// compute creal
        return (crealf(_X));
    }

    inline float __CRTDECL imag(_In_ _Fcomplex _X) throw()
    {	// compute cimag
        return (cimagf(_X));
    }

    inline float __CRTDECL real(_In_ _Fcomplex _X) throw()
    {	// compute creal
        return (crealf(_X));
    }

    // long double complex OVERLOADS
    inline _Lcomplex __CRTDECL acos(_In_ _Lcomplex _X) throw()
    {	// compute cacos
        return (cacosl(_X));
    }

    inline _Lcomplex __CRTDECL acosh(_In_ _Lcomplex _X) throw()
    {	// compute cacosh
        return (cacoshl(_X));
    }

    inline _Lcomplex __CRTDECL asin(_In_ _Lcomplex _X) throw()
    {	// compute casin
        return (casinl(_X));
    }

    inline _Lcomplex __CRTDECL asinh(_In_ _Lcomplex _X) throw()
    {	// compute casinh
        return (casinhl(_X));
    }

    inline _Lcomplex __CRTDECL atan(_In_ _Lcomplex _X) throw()
    {	// compute catan
        return (catanl(_X));
    }

    inline _Lcomplex __CRTDECL atanh(_In_ _Lcomplex _X) throw()
    {	// compute catanh
        return (catanhl(_X));
    }

    inline _Lcomplex __CRTDECL conj(_In_ _Lcomplex _X) throw()
    {	// compute conj
        return (conjl(_X));
    }

    inline _Lcomplex __CRTDECL cos(_In_ _Lcomplex _X) throw()
    {	// compute ccos
        return (ccosl(_X));
    }

    inline _Lcomplex __CRTDECL cosh(_In_ _Lcomplex _X) throw()
    {	// compute ccosh
        return (ccoshl(_X));
    }

    inline _Lcomplex __CRTDECL cproj(_In_ _Lcomplex _X) throw()
    {	// compute cproj
        return (cprojl(_X));
    }

    inline _Lcomplex __CRTDECL proj(_In_ _Lcomplex _X) throw()
    {	// compute cproj
        return (cprojl(_X));
    }

    inline _Lcomplex __CRTDECL exp(_In_ _Lcomplex _X) throw()
    {	// compute cexp
        return (cexpl(_X));
    }

    inline _Lcomplex __CRTDECL log(_In_ _Lcomplex _X) throw()
    {	// compute clog
        return (clogl(_X));
    }

    inline _Lcomplex __CRTDECL log10(_In_ _Lcomplex _X) throw()
    {	// compute clog10
        return (clog10l(_X));
    }

    inline long double __CRTDECL norm(_In_ _Lcomplex _X) throw()
    {	// compute norm
        return (norml(_X));
    }

    inline _Lcomplex __CRTDECL pow(_In_ _Lcomplex _X, _In_ _Lcomplex _Y) throw()
    {	// compute cpow
        return (cpowl(_X, _Y));
    }

    inline _Lcomplex __CRTDECL sin(_In_ _Lcomplex _X) throw()
    {	// compute csin
        return (csinl(_X));
    }

    inline _Lcomplex __CRTDECL sinh(_In_ _Lcomplex _X) throw()
    {	// compute csinh
        return (csinhl(_X));
    }

    inline _Lcomplex __CRTDECL sqrt(_In_ _Lcomplex _X) throw()
    {	// compute csqrt
        return (csqrtl(_X));
    }

    inline _Lcomplex __CRTDECL tan(_In_ _Lcomplex _X) throw()
    {	// compute ctan
        return (ctanl(_X));
    }

    inline _Lcomplex __CRTDECL tanh(_In_ _Lcomplex _X) throw()
    {	// compute ctanh
        return (ctanhl(_X));
    }

    inline long double __CRTDECL abs(_In_ _Lcomplex _X) throw()
    {	// compute cabs
        return (cabsl(_X));
    }

    inline long double __CRTDECL arg(_In_ _Lcomplex _X) throw()
    {	// compute carg
        return (cargl(_X));
    }

    inline long double __CRTDECL carg(_In_ _Lcomplex _X) throw()
    {	// compute carg
        return (cargl(_X));
    }

    inline long double __CRTDECL cimag(_In_ _Lcomplex _X) throw()
    {	// compute cimag
        return (cimagl(_X));
    }

    inline long double __CRTDECL creal(_In_ _Lcomplex _X) throw()
    {	// compute creal
        return (creall(_X));
    }

    inline long double __CRTDECL imag(_In_ _Lcomplex _X) throw()
    {	// compute cimag
        return (cimagl(_X));
    }

    inline long double __CRTDECL real(_In_ _Lcomplex _X) throw()
    {	// compute creal
        return (creall(_X));
    }
}	// extern "C++"
#endif /* __cplusplus */

#pragma pack(pop)

#endif /* _COMPLEX */
