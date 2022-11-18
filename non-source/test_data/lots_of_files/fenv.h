/***
*fenv.h - definitions and declarations for floating point environment library
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file contains constant definitions and external subroutine
*       declarations for the floating point environment subroutine library.
*
*       [Public]
*
****/

#pragma once

#ifndef _FENV
#define _FENV

#include <float.h>

/*
 * Currently, all MS C compilers for Win32 platforms default to 8 byte
 * alignment.
 */
#pragma pack(push,_CRT_PACKING)

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#if !defined(_M_CEE) || defined(_CRTBLD)

typedef unsigned long fexcept_t;
typedef struct fenv_t
{
    unsigned long _Fe_ctl, _Fe_stat;
} fenv_t;

#define FE_INEXACT      _SW_INEXACT       /* _EM_INEXACT     0x00000001 inexact (precision) */
#define FE_UNDERFLOW    _SW_UNDERFLOW     /* _EM_UNDERFLOW   0x00000002 underflow */
#define FE_OVERFLOW     _SW_OVERFLOW      /* _EM_OVERFLOW    0x00000004 overflow */
#define FE_DIVBYZERO    _SW_ZERODIVIDE    /* _EM_ZERODIVIDE  0x00000008 zero divide */
#define FE_INVALID      _SW_INVALID       /* _EM_INVALID     0x00000010 invalid */

#define FE_ALL_EXCEPT   (FE_DIVBYZERO | FE_INEXACT | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW)

_CRTIMP int __cdecl fegetenv(_Inout_ fenv_t *);
_CRTIMP int __cdecl fesetenv(_In_ const fenv_t *);
_CRTIMP int __cdecl feclearexcept(_In_ int);
_CRTIMP int __cdecl feholdexcept(_Inout_ fenv_t *);
_CRTIMP int __cdecl fetestexcept(_In_ int);
_CRTIMP int __cdecl fegetexceptflag(_Inout_ fexcept_t *, _In_ int);
_CRTIMP int __cdecl fesetexceptflag(_In_ const fexcept_t *, _In_ int);

__declspec(selectany) extern const fenv_t _Fenv0 = {0, 0};

#define FE_DFL_ENV (&_Fenv0)

#endif /* !defined(_M_CEE) || defined(_CRTBLD) */

#define FE_TONEAREST    0x0000
#define FE_UPWARD       0x0100
#define FE_DOWNWARD     0x0200
#define FE_TOWARDZERO   0x0300

#define FE_ROUND_MASK   0x0300

_CRTIMP int __cdecl fegetround(void);
_CRTIMP int __cdecl fesetround(_In_ int);

#if !defined(_M_CEE) && !defined(_CRTBLD)

/*
* feraiseexcept is inline in this header so that it will be compiled with
* the /arch setting specified in the consuming application, rather than the
* /arch:IA32 setting which is the default for the CRT library itself.
*
* feupdateenv is inline because it calls feraiseexcept.
*/
__inline int __CRTDECL feraiseexcept(_In_ int _Except)
{
    #define __DBL_BIG  1e+300  /* may raise inexact too */

    static struct
    {
        int _Except_Val;
        double _Num, _Denom;
    } const _Table[] = 
    {  /* raise exception by evaluating num / denom */
        {FE_INVALID, 0.0, 0.0},
        {FE_DIVBYZERO, 1.0, 0.0},
        {FE_OVERFLOW, __DBL_BIG, 1.0 / __DBL_BIG},
        {FE_UNDERFLOW, 1.0 / __DBL_BIG, __DBL_BIG},
        {FE_INEXACT, 2.0, 3.0}
    };

    fexcept_t _Flags = 0;
    double _Ans = 0.0;
    int _N;

    if ((_Except &= FE_ALL_EXCEPT) == 0)
    {
        return 0;
    }

    /* get the current status flags */
    fegetexceptflag(&_Flags, FE_ALL_EXCEPT);

    _Except &= ~_Flags;
    _Flags |= _Except;

    /* set the new status flags */
    fesetexceptflag(&_Flags, FE_ALL_EXCEPT);

    /* raise the exceptions not masked */
    for (_N = 0; _N < sizeof(_Table) / sizeof(_Table[0]); ++_N)
    {
        if ((_Except & _Table[_N]._Except_Val) != 0)
        {
            _Ans = _Table[_N]._Num / _Table[_N]._Denom;
        }
    }

    return 0;
}

__inline int __CRTDECL feupdateenv(_In_ const fenv_t *_Penv)
{
    int _Except = fetestexcept(FE_ALL_EXCEPT);

    if (fesetenv(_Penv) != 0 || feraiseexcept(_Except) != 0)
    {
        return 1;
    }

    return 0;
}

#endif /* !defined(_M_CEE) && !defined(_CRTBLD) */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#pragma pack(pop)

#endif /* _FENV */
