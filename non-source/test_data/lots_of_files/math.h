/***
*math.h - definitions and declarations for math library
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file contains constant definitions and external subroutine
*       declarations for the math subroutine library.
*       [ANSI/System V]
*
*       [Public]
*
****/

#ifndef _INC_MATH
#define _INC_MATH

#include <crtdefs.h>

/*
 * Currently, all MS C compilers for Win32 platforms default to 8 byte
 * alignment.
 */
#pragma pack(push,_CRT_PACKING)

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#ifndef __assembler

/* Definition of _exception struct - this struct is passed to the matherr
 * routine when a floating point exception is detected
 */

#ifndef _EXCEPTION_DEFINED
struct _exception {
        int type;       /* exception type - see below */
        char *name;     /* name of function where error occured */
        double arg1;    /* first argument to function */
        double arg2;    /* second argument (if any) to function */
        double retval;  /* value to be returned by function */
        } ;

#define _EXCEPTION_DEFINED
#endif  /* _EXCEPTION_DEFINED */


/* Definition of a _complex struct to be used by those who use cabs and
 * want type checking on their argument
 */

#ifndef _COMPLEX_DEFINED
struct _complex {
        double x,y; /* real and imaginary parts */
        } ;

#if !__STDC__ && !defined (__cplusplus)
/* Non-ANSI name for compatibility */
#define complex _complex
#endif  /* !__STDC__ && !defined (__cplusplus) */

#define _COMPLEX_DEFINED
#endif  /* _COMPLEX_DEFINED */
#endif  /* __assembler */

typedef float float_t;
typedef double double_t;

/* Constant definitions for the exception type passed in the _exception struct
 */

#define _DOMAIN     1   /* argument domain error */
#define _SING       2   /* argument singularity */
#define _OVERFLOW   3   /* overflow range error */
#define _UNDERFLOW  4   /* underflow range error */
#define _TLOSS      5   /* total loss of precision */
#define _PLOSS      6   /* partial loss of precision */

#define EDOM        33
#define ERANGE      34


/* Definitions of _HUGE and HUGE_VAL - respectively the XENIX and ANSI names
 * for a value returned in case of error by a number of the floating point
 * math routines
 */
#ifndef __assembler
#if !defined (_M_CEE_PURE)
_CRTIMP extern double _HUGE;
#else  /* !defined (_M_CEE_PURE) */
const double _HUGE = System::Double::PositiveInfinity;
#endif  /* !defined (_M_CEE_PURE) */
#endif  /* __assembler */

#define HUGE_VAL _HUGE

#ifndef _HUGE_ENUF
#define _HUGE_ENUF  1e+300	/* _HUGE_ENUF*_HUGE_ENUF must overflow */
#endif /* _HUGE_ENUF */

#define INFINITY   ((float)(_HUGE_ENUF * _HUGE_ENUF))  /* causes warning C4756: overflow in constant arithmetic (by design) */
#define HUGE_VALD  ((double)INFINITY)
#define HUGE_VALF  ((float)INFINITY)
#define HUGE_VALL  ((long double)INFINITY)
#define NAN        ((float)(INFINITY * 0.0F))

#define _DENORM    (-2)
#define _FINITE    (-1)
#define _INFCODE   1
#define _NANCODE   2

#define FP_INFINITE  _INFCODE
#define FP_NAN       _NANCODE
#define FP_NORMAL    _FINITE
#define FP_SUBNORMAL _DENORM
#define FP_ZERO      0

#define _C2          1  /* 0 if not 2's complement */
#define _FP_ILOGB0   (-0x7fffffff - _C2)
#define _FP_ILOGBNAN 0x7fffffff

#define MATH_ERRNO        1
#define MATH_ERREXCEPT    2
#define math_errhandling  (MATH_ERRNO | MATH_ERREXCEPT)

/* MACROS FOR _fperrraise ARGUMENT */
#define _FE_DIVBYZERO 0x04
#define _FE_INEXACT   0x20
#define _FE_INVALID   0x01
#define _FE_OVERFLOW  0x08
#define _FE_UNDERFLOW 0x10

#define _D0_C  3 /* little-endian, small long doubles */
#define _D1_C  2
#define _D2_C  1
#define _D3_C  0

#define _DBIAS 0x3fe
#define _DOFF  4

#define _F0_C  1 /* little-endian */
#define _F1_C  0

#define _FBIAS 0x7e
#define _FOFF  7
#define _FRND  1

#define _L0_C  3 /* little-endian, 64-bit long doubles */
#define _L1_C  2
#define _L2_C  1
#define _L3_C  0

#define _LBIAS 0x3fe
#define _LOFF  4

/* IEEE 754 double properties */
#define _DFRAC  ((unsigned short)((1 << _DOFF) - 1))
#define _DMASK  ((unsigned short)(0x7fff & ~_DFRAC))
#define _DMAX   ((unsigned short)((1 << (15 - _DOFF)) - 1))
#define _DSIGN  ((unsigned short)0x8000)

/* IEEE 754 float properties */
#define _FFRAC  ((unsigned short)((1 << _FOFF) - 1))
#define _FMASK  ((unsigned short)(0x7fff & ~_FFRAC))
#define _FMAX   ((unsigned short)((1 << (15 - _FOFF)) - 1))
#define _FSIGN  ((unsigned short)0x8000)

/* IEEE 754 long double properties */
#define _LFRAC  ((unsigned short)(-1))
#define _LMASK  ((unsigned short)0x7fff)
#define _LMAX   ((unsigned short)0x7fff)
#define _LSIGN  ((unsigned short)0x8000)

#define HUGE_EXP  (int)(_DMAX * 900L / 1000)
#define VHUGE_EXP (int)(_VMAX * 900L / 1000)
#define FHUGE_EXP (int)(_FMAX * 900L / 1000)
#define LHUGE_EXP (int)(_LMAX * 900L / 1000)

#define DSIGN_C(_Val)  (((_double_val *)(char *)&(_Val))->_Sh[_D0_C] & _DSIGN)
#define FSIGN_C(_Val)  (((_float_val *)(char *)&(_Val))->_Sh[_F0_C] & _FSIGN)
#define LSIGN_C(_Val)  (((_ldouble_val *)(char *)&(_Val))->_Sh[_L0_C] & _LSIGN)

void __cdecl _fperrraise(_In_ int _Except);

short _CRTIMP __cdecl _dclass(_In_ double _X);
short _CRTIMP __cdecl _ldclass(_In_ long double _X);
short _CRTIMP __cdecl _fdclass(_In_ float _X);

int _CRTIMP __cdecl _dsign(_In_ double _X);
int _CRTIMP __cdecl _ldsign(_In_ long double _X);
int _CRTIMP __cdecl _fdsign(_In_ float _X);

int _CRTIMP __cdecl _dpcomp(_In_ double _X, _In_ double _Y);
int _CRTIMP __cdecl _ldpcomp(_In_ long double _X, _In_ long double _Y);
int _CRTIMP __cdecl _fdpcomp(_In_ float _X, _In_ float _Y);

short _CRTIMP __cdecl _dtest(_In_ double *_Px);
short _CRTIMP __cdecl _ldtest(_In_ long double *_Px);
short _CRTIMP __cdecl _fdtest(_In_ float *_Px);

short _CRTIMP __cdecl _d_int(_In_ double *_Px, _In_ short _Xexp);
short _CRTIMP __cdecl _ld_int(_In_ long double *_Px, _In_ short _Xexp);
short _CRTIMP __cdecl _fd_int(_In_ float *_Px, _In_ short _Xexp);

short _CRTIMP __cdecl _dscale(_In_ double *_Px, _In_ long _Lexp);
short _CRTIMP __cdecl _ldscale(_In_ long double *_Px, _In_ long _Lexp);
short _CRTIMP __cdecl _fdscale(_In_ float *_Px, _In_ long _Lexp);

short _CRTIMP  __cdecl _dunscale(_In_ short *_Pex, _In_ double *_Px);
short _CRTIMP __cdecl _ldunscale(_In_ short *_Pex, _In_ long double *_Px);
short _CRTIMP __cdecl _fdunscale(_In_ short *_Pex, _In_ float *_Px);

short _CRTIMP __cdecl _dexp(_In_ double *_Px, _In_ double _Y, _In_ long _Eoff);
short _CRTIMP __cdecl _ldexp(_In_ long double *_Px, _In_ long double _Y, _In_ long _Eoff);
short _CRTIMP __cdecl _fdexp(_In_ float *_Px, _In_ float _Y, _In_ long _Eoff);

short _CRTIMP __cdecl _dnorm(_In_ unsigned short *_Ps);
short _CRTIMP __cdecl _fdnorm(_In_ unsigned short *_Ps);

double __cdecl _dpoly(_In_ double _X, _In_ const double *_Tab, _In_ int _N);
long double __cdecl _ldpoly(_In_ long double _X, _In_ const long double *_Tab, _In_ int _N);
float __cdecl _fdpoly(_In_ float _X, _In_ const float *_Tab, _In_ int _N);

double _CRTIMP __cdecl _dlog(_In_ double _X, _In_ int _Baseflag);
long double _CRTIMP __cdecl _ldlog(_In_ long double _X, _In_ int _Baseflag);
float _CRTIMP __cdecl _fdlog(_In_ float _X, _In_ int _Baseflag);

double _CRTIMP __cdecl _dsin(_In_ double _X, _In_ unsigned int _Qoff);
long double _CRTIMP __cdecl _ldsin(_In_ long double _X, _In_ unsigned int _Qoff);
float _CRTIMP __cdecl _fdsin(_In_ float _X, _In_ unsigned int _Qoff);

/* double declarations */
typedef union
{	/* pun floating type as integer array */
    unsigned short _Sh[8];
    double _Val;
} _double_val;

/* float declarations */
typedef union
{	/* pun floating type as integer array */
    unsigned short _Sh[8];
    float _Val;
} _float_val;

/* long double declarations */
typedef union
{	/* pun floating type as integer array */
    unsigned short _Sh[8];
    long double _Val;
} _ldouble_val;

typedef union
{	/* pun float types as integer array */
    unsigned short _Word[8];
    float _Float;
    double _Double;
    long double _Long_double;
} _float_const;

extern const _float_const _Denorm_C,  _Inf_C,  _Nan_C,  _Snan_C, _Hugeval_C;
extern const _float_const _FDenorm_C, _FInf_C, _FNan_C, _FSnan_C;
extern const _float_const _LDenorm_C, _LInf_C, _LNan_C, _LSnan_C;

extern const _float_const _Eps_C,  _Rteps_C;
extern const _float_const _FEps_C, _FRteps_C;
extern const _float_const _LEps_C, _LRteps_C;

extern const double _Zero_C, _Xbig_C;
extern const float _FZero_C, _FXbig_C;
extern const long double _LZero_C, _LXbig_C;

#define _FP_LT  1
#define _FP_EQ  2
#define _FP_GT  4

#ifndef __cplusplus

#define _CLASS_ARG(_Val)                                  (sizeof ((_Val) + (float)0) == sizeof (float) ? 'f' : sizeof ((_Val) + (double)0) == sizeof (double) ? 'd' : 'l')
#define _CLASSIFY(_Val, _FFunc, _DFunc, _LDFunc)          (_CLASS_ARG(_Val) == 'f' ? _FFunc((float)(_Val)) : _CLASS_ARG(_Val) == 'd' ? _DFunc((double)(_Val)) : _LDFunc((long double)(_Val)))
#define _CLASSIFY2(_Val1, _Val2, _FFunc, _DFunc, _LDFunc) (_CLASS_ARG((_Val1) + (_Val2)) == 'f' ? _FFunc((float)(_Val1), (float)(_Val2)) : _CLASS_ARG((_Val1) + (_Val2)) == 'd' ? _DFunc((double)(_Val1), (double)(_Val2)) : _LDFunc((long double)(_Val1), (long double)(_Val2)))

#define fpclassify(_Val)      (_CLASSIFY(_Val, _fdclass, _dclass, _ldclass))
#define _FPCOMPARE(_Val1, _Val2) (_CLASSIFY2(_Val1, _Val2, _fdpcomp, _dpcomp, _ldpcomp))

#define isfinite(_Val)      (fpclassify(_Val) <= 0)
#define isinf(_Val)         (fpclassify(_Val) == FP_INFINITE)
#define isnan(_Val)         (fpclassify(_Val) == FP_NAN)
#define isnormal(_Val)      (fpclassify(_Val) == FP_NORMAL)
#define signbit(_Val)       (_CLASSIFY(_Val, _fdsign, _dsign, _ldsign))

#define isgreater(x, y)      ((_FPCOMPARE(x, y) & _FP_GT) != 0)
#define isgreaterequal(x, y) ((_FPCOMPARE(x, y) & (_FP_EQ | _FP_GT)) != 0)
#define isless(x, y)         ((_FPCOMPARE(x, y) & _FP_LT) != 0)
#define islessequal(x, y)    ((_FPCOMPARE(x, y) & (_FP_LT | _FP_EQ)) != 0)
#define islessgreater(x, y)  ((_FPCOMPARE(x, y) & (_FP_LT | _FP_GT)) != 0)
#define isunordered(x, y)    (_FPCOMPARE(x, y) == 0)

#else /* __cplusplus */

extern "C++" {

inline __nothrow int fpclassify(float _X)
{
    return (_fdtest(&_X));
}

inline __nothrow int fpclassify(double _X)
{
    return (_dtest(&_X));
}

inline __nothrow int fpclassify(long double _X)
{
    return (_ldtest(&_X));
}

inline __nothrow bool signbit(float _X)
{
    return (_fdsign(_X) != 0);
}

inline __nothrow bool signbit(double _X)
{
    return (_dsign(_X) != 0);
}

inline __nothrow bool signbit(long double _X)
{
    return (_ldsign(_X) != 0);
}

inline __nothrow int _fpcomp(float _X, float _Y)
{
    return (_fdpcomp(_X, _Y));
}

inline __nothrow int _fpcomp(double _X, double _Y)
{
    return (_dpcomp(_X, _Y));
}

inline __nothrow int _fpcomp(long double _X, long double _Y)
{
    return (_ldpcomp(_X, _Y));
}

template<class _Trc, class _Tre> struct _Combined_type
{	// determine combined type
    typedef float _Type;	// (real, float) is float
};

template<> struct _Combined_type<float, double>
{	// determine combined type
    typedef double _Type;
};

template<> struct _Combined_type<float, long double>
{	// determine combined type
    typedef long double _Type;
};

template<class _Ty, class _T2> struct _Real_widened
{	// determine widened real type
    typedef long double _Type;	// default is long double
};

template<> struct _Real_widened<float, float>
{	// determine widened real type
    typedef float _Type;
};

template<> struct _Real_widened<float, double>
{	// determine widened real type
    typedef double _Type;
};

template<> struct _Real_widened<double, float>
{	// determine widened real type
    typedef double _Type;
};

template<> struct _Real_widened<double, double>
{	// determine widened real type
    typedef double _Type;
};

template<class _Ty> struct _Real_type
{	// determine equivalent real type
    typedef double _Type;	// default is double
};

template<> struct _Real_type<float>
{	// determine equivalent real type
    typedef float _Type;
};

template<> struct _Real_type<long double>
{	// determine equivalent real type
    typedef long double _Type;
};

template<class _T1, class _T2> inline __nothrow int _fpcomp(_T1 _X, _T2 _Y)
{	// compare _Left and _Right
    typedef typename _Combined_type<float,
        typename _Real_widened<
        typename _Real_type<_T1>::_Type,
        typename _Real_type<_T2>::_Type>::_Type>::_Type _Tw;
    return (_fpcomp((_Tw)_X, (_Tw)_Y));
}

template<class _Ty> inline __nothrow bool isfinite(_Ty _X)
{
    return (fpclassify(_X) <= 0);
}

template<class _Ty> inline __nothrow bool isinf(_Ty _X)
{
    return (fpclassify(_X) == FP_INFINITE);
}

template<class _Ty> inline __nothrow bool isnan(_Ty _X)
{
    return (fpclassify(_X) == FP_NAN);
}

template<class _Ty> inline __nothrow bool isnormal(_Ty _X)
{
    return (fpclassify(_X) == FP_NORMAL);
}

template<class _Ty1, class _Ty2> inline __nothrow bool isgreater(_Ty1 _X, _Ty2 _Y)
{
    return ((_fpcomp(_X, _Y) & _FP_GT) != 0);
}

template<class _Ty1, class _Ty2> inline __nothrow bool isgreaterequal(_Ty1 _X, _Ty2 _Y)
{
    return ((_fpcomp(_X, _Y) & (_FP_EQ | _FP_GT)) != 0);
}

template<class _Ty1, class _Ty2> inline __nothrow bool isless(_Ty1 _X, _Ty2 _Y)
{
    return ((_fpcomp(_X, _Y) & _FP_LT) != 0);
}

template<class _Ty1, class _Ty2> inline __nothrow bool islessequal(_Ty1 _X, _Ty2 _Y)
{
    return ((_fpcomp(_X, _Y) & (_FP_LT | _FP_EQ)) != 0);
}

template<class _Ty1, class _Ty2> inline __nothrow bool islessgreater(_Ty1 _X, _Ty2 _Y)
{
    return ((_fpcomp(_X, _Y) & (_FP_LT | _FP_GT)) != 0);
}

template<class _Ty1, class _Ty2> inline __nothrow bool isunordered(_Ty1 _X, _Ty2 _Y)
{
    return (_fpcomp(_X, _Y) == 0);
}

}  // extern "C++"

#endif /* __cplusplus */

/* Function prototypes */

#if !defined (__assembler)
int       __cdecl abs(_In_ int _X);
long      __cdecl labs(_In_ long _X);
long long __cdecl llabs(_In_ long long _X);

double  __cdecl acos(_In_ double _X);
_CRTIMP double __cdecl acosh(_In_ double _X);
double  __cdecl asin(_In_ double _X);
_CRTIMP double __cdecl asinh(_In_ double _X);
double  __cdecl atan(_In_ double _X);
_CRTIMP double __cdecl atanh(_In_ double _X);
double  __cdecl atan2(_In_ double _Y, _In_ double _X);

_CRTIMP double __cdecl cbrt(_In_ double _X);
_CRTIMP double __cdecl copysign(_In_ double _X, _In_ double _Y);
double  __cdecl cos(_In_ double _X);
double  __cdecl cosh(_In_ double _X);
_CRTIMP double __cdecl erf(_In_ double _X);
_CRTIMP double __cdecl erfc(_In_ double _X);
double  __cdecl exp(_In_ double _X);
_CRTIMP double __cdecl exp2(_In_ double _X);
_CRTIMP double __cdecl expm1(_In_ double _X);
_CRT_JIT_INTRINSIC double  __cdecl fabs(_In_ double _X);
_CRTIMP double __cdecl fdim(_In_ double _X, _In_ double _Y);
_CRTIMP double __cdecl fma(_In_ double _X, _In_ double _Y, _In_ double _Z);
_CRTIMP double __cdecl fmax(_In_ double _X, _In_ double _Y);
_CRTIMP double __cdecl fmin(_In_ double _X, _In_ double _Y);
double  __cdecl fmod(_In_ double _X, _In_ double _Y);
_CRTIMP int __cdecl ilogb(_In_ double _X);
_CRTIMP double __cdecl lgamma(_In_ double _X);
_CRTIMP long long __cdecl llrint(_In_ double _X);
_CRTIMP long long __cdecl llround(_In_ double _X);
double  __cdecl log(_In_ double _X);
double  __cdecl log10(_In_ double _X);
_CRTIMP double __cdecl log1p(_In_ double _X);
_CRTIMP double __cdecl log2(_In_ double _X);
_CRTIMP double __cdecl logb(_In_ double _X);
_CRTIMP long __cdecl lrint(_In_ double _X);
_CRTIMP long __cdecl lround(_In_ double _X);
_CRTIMP double __cdecl nan(_In_ const char *);
_CRTIMP double __cdecl nearbyint(_In_ double _X);
_CRTIMP double __cdecl nextafter(_In_ double _X, _In_ double _Y);
_CRTIMP double __cdecl nexttoward(_In_ double _X, _In_ long double _Y);
double  __cdecl pow(_In_ double _X, _In_ double _Y);
_CRTIMP double __cdecl remainder(_In_ double _X, _In_ double _Y);
_CRTIMP double __cdecl remquo(_In_ double _X, _In_ double _Y, _Out_ int *_Z);
_CRTIMP double __cdecl rint(_In_ double _X);
_CRTIMP double __cdecl round(_In_ double _X);
_CRTIMP double __cdecl scalbln(_In_ double _X, _In_ long _Y);
_CRTIMP double __cdecl scalbn(_In_ double _X, _In_ int _Y);
double  __cdecl sin(_In_ double _X);
double  __cdecl sinh(_In_ double _X);
_CRT_JIT_INTRINSIC  double  __cdecl sqrt(_In_ double _X);
double  __cdecl tan(_In_ double _X);
double  __cdecl tanh(_In_ double _X);
_CRTIMP double __cdecl tgamma(_In_ double _X);
_CRTIMP double __cdecl trunc(_In_ double _X);

_Check_return_ _CRTIMP double  __cdecl atof(_In_z_ const char *_String);
_Check_return_ _CRTIMP double  __cdecl _atof_l(_In_z_ const char *_String, _In_opt_ _locale_t _Locale);

_CRTIMP double  __cdecl _cabs(_In_ struct _complex _Complex_value);
_CRTIMP double  __cdecl ceil(_In_ double _X);

_Check_return_ _CRTIMP double __cdecl _chgsign (_In_ double _X);
_Check_return_ _CRTIMP double __cdecl _copysign (_In_ double _Number, _In_ double _Sign);

_CRTIMP double  __cdecl floor(_In_ double _X);
_CRTIMP double  __cdecl frexp(_In_ double _X, _Out_ int * _Y);
_CRTIMP double  __cdecl _hypot(_In_ double _X, _In_ double _Y);
_CRTIMP double  __cdecl _j0(_In_ double _X );
_CRTIMP double  __cdecl _j1(_In_ double _X );
_CRTIMP double  __cdecl _jn(int _X, _In_ double _Y);
_CRTIMP double  __cdecl ldexp(_In_ double _X, _In_ int _Y);

#if defined (MRTDLL) || defined (_M_CEE_PURE)
int     __CRTDECL _matherr(_Inout_ struct _exception * _Except);
#else  /* defined (MRTDLL) || defined (_M_CEE_PURE) */
int     __cdecl _matherr(_Inout_ struct _exception * _Except);
#endif  /* defined (MRTDLL) || defined (_M_CEE_PURE) */

_CRTIMP double  __cdecl modf(_In_ double _X, _Out_ double * _Y);
_CRTIMP double  __cdecl _y0(_In_ double _X);
_CRTIMP double  __cdecl _y1(_In_ double _X);
_CRTIMP double  __cdecl _yn(_In_ int _X, _In_ double _Y);

__inline double __CRTDECL hypot(_In_ double _X, _In_ double _Y)
{
    return _hypot(_X, _Y);
}


_CRTIMP float __cdecl acoshf(_In_ float _X);
_CRTIMP float __cdecl asinhf(_In_ float _X);
_CRTIMP float __cdecl atanhf(_In_ float _X);
_CRTIMP float __cdecl cbrtf(_In_ float _X);
_CRTIMP float  __cdecl _chgsignf(_In_ float _X);
_CRTIMP float __cdecl copysignf(_In_ float _X, _In_ float _Y);
_CRTIMP float  __cdecl _copysignf(_In_ float _Number, _In_ float _Sign);
_CRTIMP float __cdecl erff(_In_ float _X);
_CRTIMP float __cdecl erfcf(_In_ float _X);
_CRTIMP float __cdecl expm1f(_In_ float _X);
_CRTIMP float __cdecl exp2f(_In_ float _X);
_CRTIMP float __cdecl fdimf(_In_ float _X, _In_ float _Y);
_CRTIMP float __cdecl fmaf(_In_ float _X, _In_ float _Y, _In_ float _Z);
_CRTIMP float __cdecl fmaxf(_In_ float _X, _In_ float _Y);
_CRTIMP float __cdecl fminf(_In_ float _X, _In_ float _Y);
_CRTIMP float  __cdecl _hypotf(_In_ float _X, _In_ float _Y);
_CRTIMP int __cdecl ilogbf(_In_ float _X);
_CRTIMP float __cdecl lgammaf(_In_ float _X);
_CRTIMP long long __cdecl llrintf(_In_ float _X);
_CRTIMP long long __cdecl llroundf(_In_ float _X);
_CRTIMP float __cdecl log1pf(_In_ float _X);
_CRTIMP float __cdecl log2f(_In_ float _X);
_CRTIMP float __cdecl logbf(_In_ float _X);
_CRTIMP long __cdecl lrintf(_In_ float _X);
_CRTIMP long __cdecl lroundf(_In_ float _X);
_CRTIMP float __cdecl nanf(_In_ const char *);
_CRTIMP float __cdecl nearbyintf(_In_ float _X);
_CRTIMP float __cdecl nextafterf(_In_ float _X, _In_ float _Y);
_CRTIMP float __cdecl nexttowardf(_In_ float _X, _In_ long double _Y);
_CRTIMP float __cdecl remainderf(_In_ float _X, _In_ float _Y);
_CRTIMP float __cdecl remquof(_In_ float _X, _In_ float _Y, _Out_ int *_Z);
_CRTIMP float __cdecl rintf(_In_ float _X);
_CRTIMP float __cdecl roundf(_In_ float _X);
_CRTIMP float __cdecl scalblnf(_In_ float _X, _In_ long _Y);
_CRTIMP float __cdecl scalbnf(_In_ float _X, _In_ int _Y);
_CRTIMP float __cdecl tgammaf(_In_ float _X);
_CRTIMP float __cdecl truncf(_In_ float _X);

#if defined (_M_IX86)

_CRTIMP int  __cdecl _set_SSE2_enable(_In_ int _Flag);

#endif  /* defined (_M_IX86) */

#if defined (_M_X64)

_CRTIMP float  __cdecl _logbf(_In_ float _X);
_CRTIMP float  __cdecl _nextafterf(_In_ float _X, _In_ float _Y);
_CRTIMP int    __cdecl _finitef(_In_ float _X);
_CRTIMP int    __cdecl _isnanf(_In_ float _X);
_CRTIMP int    __cdecl _fpclassf(_In_ float _X);

_CRTIMP int  __cdecl _set_FMA3_enable(_In_ int _Flag);

#endif  /* defined (_M_X64) */

#if defined (_M_ARM)

_CRTIMP int    __cdecl _finitef(_In_ float _X);
_CRTIMP float  __cdecl _logbf(_In_ float _X);

#endif  /* defined (_M_ARM) */

#if defined (_M_X64) || defined (_M_ARM)

_CRTIMP float  __cdecl acosf(_In_ float _X);
_CRTIMP float  __cdecl asinf(_In_ float _X);
_CRTIMP float  __cdecl atan2f(_In_ float _Y, _In_ float _X);
_CRTIMP float  __cdecl atanf(_In_ float _X);
_CRTIMP float  __cdecl ceilf(_In_ float _X);
_CRTIMP float  __cdecl cosf(_In_ float _X);
_CRTIMP float  __cdecl coshf(_In_ float _X);
_CRTIMP float  __cdecl expf(_In_ float _X);

#else  /* defined(_M_X64) || defined(_M_ARM) */

__inline float  __CRTDECL acosf(_In_ float _X)
{
    return (float)acos(_X);
}

__inline float  __CRTDECL asinf(_In_ float _X)
{
    return (float)asin(_X);
}

__inline float  __CRTDECL atan2f(_In_ float _Y, _In_ float _X)
{
    return (float)atan2(_Y, _X);
}

__inline float  __CRTDECL atanf(_In_ float _X)
{
    return (float)atan(_X);
}

__inline float  __CRTDECL ceilf(_In_ float _X)
{
    return (float)ceil(_X);
}

__inline float  __CRTDECL cosf(_In_ float _X)
{
    return (float)cos(_X);
}

__inline float  __CRTDECL coshf(_In_ float _X)
{
    return (float)cosh(_X);
}

__inline float  __CRTDECL expf(_In_ float _X)
{
    return (float)exp(_X);
}

#endif  /* defined (_M_X64) || defined (_M_ARM) */

#if defined (_M_ARM)

_CRT_JIT_INTRINSIC _CRTIMP float  __cdecl fabsf(_In_ float  _X);

#else  /* defined (_M_ARM) */

__inline float __CRTDECL fabsf(_In_ float _X)
{
    return (float)fabs(_X);
}

#endif  /* defined (_M_ARM) */

#if defined (_M_X64) || defined (_M_ARM)

_CRTIMP float  __cdecl floorf(_In_ float _X);
_CRTIMP float  __cdecl fmodf(_In_ float _X, _In_ float _Y);

#else /* defined (_M_X64) || defined (_M_ARM) */

__inline float __CRTDECL floorf(_In_ float _X)
{
    return (float)floor(_X);
}

__inline float __CRTDECL fmodf(_In_ float _X, _In_ float _Y)
{
    return (float)fmod(_X, _Y);
}

#endif  /* !defined(_M_X64) && !defined(_M_ARM) */

__inline float __CRTDECL frexpf(_In_ float _X, _Out_ int *_Y)
{
    return (float)frexp(_X, _Y);
}

__inline float __CRTDECL hypotf(_In_ float _X, _In_ float _Y)
{
    return _hypotf(_X, _Y);
}

__inline float __CRTDECL ldexpf(_In_ float _X, _In_ int _Y)
{
    return (float)ldexp(_X, _Y);
}

#if defined (_M_X64) || defined (_M_ARM)

_CRTIMP float  __cdecl log10f(_In_ float _X);
_CRTIMP float  __cdecl logf(_In_ float _X);
_CRTIMP float  __cdecl modff(_In_ float _X, _Out_ float *_Y);
_CRTIMP float  __cdecl powf(_In_ float _X, _In_ float _Y);
_CRTIMP float  __cdecl sinf(_In_ float _X);
_CRTIMP float  __cdecl sinhf(_In_ float _X);
_CRTIMP float  __cdecl sqrtf(_In_ float _X);
_CRTIMP float  __cdecl tanf(_In_ float _X);
_CRTIMP float  __cdecl tanhf(_In_ float _X);

#else /* defined (_M_X64) || defined (_M_ARM) */

__inline float __CRTDECL log10f(_In_ float _X)
{
    return (float)log10(_X);
}

__inline float __CRTDECL logf(_In_ float _X)
{
    return (float)log(_X);
}

__inline float __CRTDECL modff(_In_ float _X, _Out_ float *_Y)
{
    double _F, _I;
    _F = modf(_X, &_I);
    *_Y = (float)_I;
    return (float)_F;
}

__inline float __CRTDECL powf(_In_ float _X, _In_ float _Y)
{
    return (float)pow(_X, _Y);
}

__inline float __CRTDECL sinf(_In_ float _X)
{
    return (float)sin(_X);
}

__inline float __CRTDECL sinhf(_In_ float _X)
{
    return (float)sinh(_X);
}

__inline float __CRTDECL sqrtf(_In_ float _X)
{
    return (float)sqrt(_X);
}

__inline float __CRTDECL tanf(_In_ float _X)
{
    return (float)tan(_X);
}

__inline float __CRTDECL tanhf(_In_ float _X)
{
    return (float)tanh(_X);
}

#endif  /* defined (_M_X64) || defined (_M_ARM) */

_CRTIMP long double __cdecl acoshl(_In_ long double _X);

__inline long double __CRTDECL acosl(_In_ long double _X)
{
    return acos((double)_X);
}

_CRTIMP long double __cdecl asinhl(_In_ long double _X);

__inline long double __CRTDECL asinl(_In_ long double _X)
{
    return asin((double)_X);
}

__inline long double __CRTDECL atan2l(_In_ long double _Y, _In_ long double _X)
{
    return atan2((double)_Y, (double)_X);
}

_CRTIMP long double __cdecl atanhl(_In_ long double _X);

__inline long double __CRTDECL atanl(_In_ long double _X)
{
    return atan((double)_X);
}

_CRTIMP long double __cdecl cbrtl(_In_ long double _X);

__inline long double __CRTDECL ceill(_In_ long double _X)
{
    return ceil((double)_X);
}

__inline long double __CRTDECL _chgsignl(_In_ long double _X)
{
    return _chgsign((double)_X);
}

_CRTIMP long double __cdecl copysignl(_In_ long double _X, _In_ long double _Y);

__inline long double __CRTDECL _copysignl(_In_ long double _X, _In_ long double _Y)
{
    return _copysign((double)_X, (double)_Y);
}

__inline long double __CRTDECL coshl(_In_ long double _X)
{
    return cosh((double)_X);
}

__inline long double __CRTDECL cosl(_In_ long double _X)
{
    return cos((double)_X);
}

_CRTIMP long double __cdecl erfl(_In_ long double _X);
_CRTIMP long double __cdecl erfcl(_In_ long double _X);

__inline long double __CRTDECL expl(_In_ long double _X)
{
    return exp((double)_X);
}

_CRTIMP long double __cdecl exp2l(_In_ long double _X);
_CRTIMP long double __cdecl expm1l(_In_ long double _X);

__inline long double __CRTDECL fabsl(_In_ long double _X)
{
    return fabs((double)_X);
}

_CRTIMP long double __cdecl fdiml(_In_ long double _X, _In_ long double _Y);

__inline long double __CRTDECL floorl(_In_ long double _X)
{
    return floor((double)_X);
}

_CRTIMP long double __cdecl fmal(_In_ long double _X, _In_ long double _Y, _In_ long double _Z);
_CRTIMP long double __cdecl fmaxl(_In_ long double _X, _In_ long double _Y);
_CRTIMP long double __cdecl fminl(_In_ long double _X, _In_ long double _Y);

__inline long double __CRTDECL fmodl(_In_ long double _X, _In_ long double _Y)
{
    return fmod((double)_X, (double)_Y);
}

__inline long double __CRTDECL frexpl(_In_ long double _X, _Out_ int *_Y)
{
    return frexp((double)_X, _Y);
}

_CRTIMP int __cdecl ilogbl(_In_ long double _X);

__inline long double __CRTDECL _hypotl(_In_ long double _X, _In_ long double _Y)
{
    return _hypot((double)_X, (double)_Y);
}

__inline long double __CRTDECL hypotl(_In_ long double _X, _In_ long double _Y)
{
    return _hypot((double)_X, (double)_Y);
}

__inline long double __CRTDECL ldexpl(_In_ long double _X, _In_ int _Y)
{
    return ldexp((double)_X, _Y);
}

_CRTIMP long double __cdecl lgammal(_In_ long double _X);
_CRTIMP long long __cdecl llrintl(_In_ long double _X);
_CRTIMP long long __cdecl llroundl(_In_ long double _X);

__inline long double __CRTDECL logl(_In_ long double _X)
{
    return log((double)_X);
}

__inline long double __CRTDECL log10l(_In_ long double _X)
{
    return log10((double)_X);
}

_CRTIMP long double __cdecl log1pl(_In_ long double _X);
_CRTIMP long double __cdecl log2l(_In_ long double _X);
_CRTIMP long double __cdecl logbl(_In_ long double _X);
_CRTIMP long __cdecl lrintl(_In_ long double _X);
_CRTIMP long __cdecl lroundl(_In_ long double _X);

__inline long double __CRTDECL modfl(_In_ long double _X, _Out_ long double *_Y)
{
    double _F, _I;
    _F = modf((double)_X, &_I);
    *_Y = _I;
    return _F;
}
_CRTIMP long double __cdecl nanl(_In_ const char *);
_CRTIMP long double __cdecl nearbyintl(_In_ long double _X);
_CRTIMP long double __cdecl nextafterl(_In_ long double _X, _In_ long double _Y);
_CRTIMP long double __cdecl nexttowardl(_In_ long double _X, _In_ long double _Y);

__inline long double __CRTDECL powl(_In_ long double _X, _In_ long double _Y)
{
    return pow((double)_X, (double)_Y);
}

_CRTIMP long double __cdecl remainderl(_In_ long double _X, _In_ long double _Y);
_CRTIMP long double __cdecl remquol(_In_ long double _X, _In_ long double _Y, _Out_ int *_Z);
_CRTIMP long double __cdecl rintl(_In_ long double _X);
_CRTIMP long double __cdecl roundl(_In_ long double _X);
_CRTIMP long double __cdecl scalblnl(_In_ long double _X, _In_ long _Y);
_CRTIMP long double __cdecl scalbnl(_In_ long double _X, _In_ int _Y);

__inline long double __CRTDECL sinhl(_In_ long double _X)
{
    return sinh((double)_X);
}

__inline long double __CRTDECL sinl(_In_ long double _X)
{
    return sin((double)_X);
}

__inline long double __CRTDECL sqrtl(_In_ long double _X)
{
    return sqrt((double)_X);
}

__inline long double __CRTDECL tanhl(_In_ long double _X)
{
    return tanh((double)_X);
}

__inline long double __CRTDECL tanl(_In_ long double _X)
{
    return tan((double)_X);
}

_CRTIMP long double __cdecl tgammal(_In_ long double _X);
_CRTIMP long double __cdecl truncl(_In_ long double _X);

#ifndef __cplusplus
#define _matherrl _matherr
#endif /* __cplusplus */
#endif  /* !defined (__assembler) */

#if !__STDC__

/* Non-ANSI names for compatibility */

#define DOMAIN      _DOMAIN
#define SING        _SING
#define OVERFLOW    _OVERFLOW
#define UNDERFLOW   _UNDERFLOW
#define TLOSS       _TLOSS
#define PLOSS       _PLOSS

#define matherr     _matherr

#ifndef __assembler

#if !defined (_M_CEE_PURE)
_CRTIMP extern double HUGE;
#else  /* !defined (_M_CEE_PURE) */
    const double HUGE = _HUGE;
#endif  /* !defined (_M_CEE_PURE) */

_CRT_NONSTDC_DEPRECATE(_j0) _CRTIMP double  __cdecl j0(_In_ double _X);
_CRT_NONSTDC_DEPRECATE(_j1) _CRTIMP double  __cdecl j1(_In_ double _X);
_CRT_NONSTDC_DEPRECATE(_jn) _CRTIMP double  __cdecl jn(_In_ int _X, _In_ double _Y);
_CRT_NONSTDC_DEPRECATE(_y0) _CRTIMP double  __cdecl y0(_In_ double _X);
_CRT_NONSTDC_DEPRECATE(_y1) _CRTIMP double  __cdecl y1(_In_ double _X);
_CRT_NONSTDC_DEPRECATE(_yn) _CRTIMP double  __cdecl yn(_In_ int _X, _In_ double _Y);

#endif  /* __assembler */
#endif  /* !__STDC__ */

#ifdef __cplusplus
}

extern "C++" {

template<class _Ty> inline
        _Ty _Pow_int(_Ty _X, int _Y) throw()
        {unsigned int _N;
        if (_Y >= 0)
                _N = (unsigned int)_Y;
        else
                _N = (unsigned int)(-_Y);
        for (_Ty _Z = _Ty(1); ; _X *= _X)
                {if ((_N & 1) != 0)
                        _Z *= _X;
                if ((_N >>= 1) == 0)
                        return (_Y < 0 ? _Ty(1) / _Z : _Z); }}

inline double __CRTDECL abs(_In_ double _X) throw()
        {return (fabs(_X)); }
inline double __CRTDECL pow(_In_ double _X, _In_ int _Y) throw()
        {return (_Pow_int(_X, _Y)); }
inline float __CRTDECL abs(_In_ float _X) throw()
        {return (fabsf(_X)); }
inline float __CRTDECL acos(_In_ float _X) throw()
        {return (acosf(_X)); }
inline float __CRTDECL acosh(_In_ float _X) throw()
        {return (acoshf(_X)); }
inline float __CRTDECL asin(_In_ float _X) throw()
        {return (asinf(_X)); }
inline float __CRTDECL asinh(_In_ float _X) throw()
        {return (asinhf(_X)); }
inline float __CRTDECL atan(_In_ float _X) throw()
        {return (atanf(_X)); }
inline float __CRTDECL atanh(_In_ float _X) throw()
        {return (atanhf(_X)); }
inline float __CRTDECL atan2(_In_ float _Y, _In_ float _X) throw()
        {return (atan2f(_Y, _X)); }
inline float __CRTDECL cbrt(_In_ float _X) throw()
        {return (cbrtf(_X)); }
inline float __CRTDECL ceil(_In_ float _X) throw()
        {return (ceilf(_X)); }
inline float __CRTDECL copysign(_In_ float _X, _In_ float _Y) throw()
        {return (copysignf(_X, _Y)); }
inline float __CRTDECL cos(_In_ float _X) throw()
        {return (cosf(_X)); }
inline float __CRTDECL cosh(_In_ float _X) throw()
        {return (coshf(_X)); }
inline float __CRTDECL erf(_In_ float _X) throw()
        {return (erff(_X)); }
inline float __CRTDECL erfc(_In_ float _X) throw()
        {return (erfcf(_X)); }
inline float __CRTDECL exp(_In_ float _X) throw()
        {return (expf(_X)); }
inline float __CRTDECL exp2(_In_ float _X) throw()
        {return (exp2f(_X)); }
inline float __CRTDECL expm1(_In_ float _X) throw()
        {return (expm1f(_X)); }
inline float __CRTDECL fabs(_In_ float _X) throw()
        {return (fabsf(_X)); }
inline float __CRTDECL fdim(_In_ float _X, _In_ float _Y) throw()
        {return (fdimf(_X, _Y)); }
inline float __CRTDECL floor(_In_ float _X) throw()
        {return (floorf(_X)); }
inline float __CRTDECL fma(_In_ float _X, _In_ float _Y, _In_ float _Z) throw()
        {return (fmaf(_X, _Y, _Z)); }
inline float __CRTDECL fmax(_In_ float _X, _In_ float _Y) throw()
        {return (fmaxf(_X, _Y)); }
inline float __CRTDECL fmin(_In_ float _X, _In_ float _Y) throw()
        {return (fminf(_X, _Y)); }
inline float __CRTDECL fmod(_In_ float _X, _In_ float _Y) throw()
        {return (fmodf(_X, _Y)); }
inline float __CRTDECL frexp(_In_ float _X, _Out_ int * _Y) throw()
        {return (frexpf(_X, _Y)); }
inline float __CRTDECL hypot(_In_ float _X, _In_ float _Y) throw()
        {return (hypotf(_X, _Y)); }
inline int __CRTDECL ilogb(_In_ float _X) throw()
        {return (ilogbf(_X)); }
inline float __CRTDECL ldexp(_In_ float _X, _In_ int _Y) throw()
        {return (ldexpf(_X, _Y)); }
inline float __CRTDECL lgamma(_In_ float _X) throw()
        {return (lgammaf(_X)); }
inline long long __CRTDECL llrint(_In_ float _X) throw()
        {return (llrintf(_X)); }
inline long long __CRTDECL llround(_In_ float _X) throw()
        {return (llroundf(_X)); }
inline float __CRTDECL log(_In_ float _X) throw()
        {return (logf(_X)); }
inline float __CRTDECL log10(_In_ float _X) throw()
        {return (log10f(_X)); }
inline float __CRTDECL log1p(_In_ float _X) throw()
        {return (log1pf(_X)); }
inline float __CRTDECL log2(_In_ float _X) throw()
        {return (log2f(_X)); }
inline float __CRTDECL logb(_In_ float _X) throw()
        {return (logbf(_X)); }
inline long __CRTDECL lrint(_In_ float _X) throw()
        {return (lrintf(_X)); }
inline long __CRTDECL lround(_In_ float _X) throw()
        {return (lroundf(_X)); }
inline float __CRTDECL modf(_In_ float _X, _Out_ float * _Y) throw()
        {return (modff(_X, _Y)); }
inline float __CRTDECL nearbyint(_In_ float _X) throw()
        {return (nearbyintf(_X)); }
inline float __CRTDECL nextafter(_In_ float _X, _In_ float _Y) throw()
        {return (nextafterf(_X, _Y)); }
inline float __CRTDECL nexttoward(_In_ float _X, _In_ long double _Y) throw()
        {return (nexttowardf(_X, _Y)); }
inline float __CRTDECL pow(_In_ float _X, _In_ float _Y) throw()
        {return (powf(_X, _Y)); }
inline float __CRTDECL pow(_In_ float _X, _In_ int _Y) throw()
        {return (_Pow_int(_X, _Y)); }
inline float __CRTDECL remainder(_In_ float _X, _In_ float _Y) throw()
        {return (remainderf(_X, _Y)); }
inline float __CRTDECL remquo(_In_ float _X, _In_ float _Y, _Out_ int *_Z) throw()
        {return (remquof(_X, _Y, _Z)); }
inline float __CRTDECL rint(_In_ float _X) throw()
        {return (rintf(_X)); }
inline float __CRTDECL round(_In_ float _X) throw()
        {return (roundf(_X)); }
inline float __CRTDECL scalbln(_In_ float _X, _In_ long _Y) throw()
        {return (scalblnf(_X, _Y)); }
inline float __CRTDECL scalbn(_In_ float _X, _In_ int _Y) throw()
        {return (scalbnf(_X, _Y)); }
inline float __CRTDECL sin(_In_ float _X) throw()
        {return (sinf(_X)); }
inline float __CRTDECL sinh(_In_ float _X) throw()
        {return (sinhf(_X)); }
inline float __CRTDECL sqrt(_In_ float _X) throw()
        {return (sqrtf(_X)); }
inline float __CRTDECL tan(_In_ float _X) throw()
        {return (tanf(_X)); }
inline float __CRTDECL tanh(_In_ float _X) throw()
        {return (tanhf(_X)); }
inline float __CRTDECL tgamma(_In_ float _X) throw()
        {return (tgammaf(_X)); }
inline float __CRTDECL trunc(_In_ float _X) throw()
        {return (truncf(_X)); }
inline long double __CRTDECL abs(_In_ long double _X) throw()
        {return (fabsl(_X)); }
inline long double __CRTDECL acos(_In_ long double _X) throw()
        {return (acosl(_X)); }
inline long double __CRTDECL acosh(_In_ long double _X) throw()
        {return (acoshl(_X)); }
inline long double __CRTDECL asin(_In_ long double _X) throw()
        {return (asinl(_X)); }
inline long double __CRTDECL asinh(_In_ long double _X) throw()
        {return (asinhl(_X)); }
inline long double __CRTDECL atan(_In_ long double _X) throw()
        {return (atanl(_X)); }
inline long double __CRTDECL atanh(_In_ long double _X) throw()
        {return (atanhl(_X)); }
inline long double __CRTDECL atan2(_In_ long double _Y, _In_ long double _X) throw()
        {return (atan2l(_Y, _X)); }
inline long double __CRTDECL cbrt(_In_ long double _X) throw()
        {return (cbrtl(_X)); }
inline long double __CRTDECL ceil(_In_ long double _X) throw()
        {return (ceill(_X)); }
inline long double __CRTDECL copysign(_In_ long double _X, _In_ long double _Y) throw()
        {return (copysignl(_X, _Y)); }
inline long double __CRTDECL cos(_In_ long double _X) throw()
        {return (cosl(_X)); }
inline long double __CRTDECL cosh(_In_ long double _X) throw()
        {return (coshl(_X)); }
inline long double __CRTDECL erf(_In_ long double _X) throw()
        {return (erfl(_X)); }
inline long double __CRTDECL erfc(_In_ long double _X) throw()
        {return (erfcl(_X)); }
inline long double __CRTDECL exp(_In_ long double _X) throw()
        {return (expl(_X)); }
inline long double __CRTDECL exp2(_In_ long double _X) throw()
        {return (exp2l(_X)); }
inline long double __CRTDECL expm1(_In_ long double _X) throw()
        {return (expm1l(_X)); }
inline long double __CRTDECL fabs(_In_ long double _X) throw()
        {return (fabsl(_X)); }
inline long double __CRTDECL fdim(_In_ long double _X, _In_ long double _Y) throw()
        {return (fdiml(_X, _Y)); }
inline long double __CRTDECL floor(_In_ long double _X) throw()
        {return (floorl(_X)); }
inline long double __CRTDECL fma(_In_ long double _X, _In_ long double _Y, _In_ long double _Z) throw()
        {return (fmal(_X, _Y, _Z)); }
inline long double __CRTDECL fmax(_In_ long double _X, _In_ long double _Y) throw()
        {return (fmaxl(_X, _Y)); }
inline long double __CRTDECL fmin(_In_ long double _X, _In_ long double _Y) throw()
        {return (fminl(_X, _Y)); }
inline long double __CRTDECL fmod(_In_ long double _X, _In_ long double _Y) throw()
        {return (fmodl(_X, _Y)); }
inline long double __CRTDECL frexp(_In_ long double _X, _Out_ int * _Y) throw()
        {return (frexpl(_X, _Y)); }
inline long double __CRTDECL hypot(_In_ long double _X, _In_ long double _Y) throw()
        {return (hypotl(_X, _Y)); }
inline int __CRTDECL ilogb(_In_ long double _X) throw()
        {return (ilogbl(_X)); }
inline long double __CRTDECL ldexp(_In_ long double _X, _In_ int _Y) throw()
        {return (ldexpl(_X, _Y)); }
inline long double __CRTDECL lgamma(_In_ long double _X) throw()
        {return (lgammal(_X)); }
inline long long __CRTDECL llrint(_In_ long double _X) throw()
        {return (llrintl(_X)); }
inline long long __CRTDECL llround(_In_ long double _X) throw()
        {return (llroundl(_X)); }
inline long double __CRTDECL log(_In_ long double _X) throw()
        {return (logl(_X)); }
inline long double __CRTDECL log10(_In_ long double _X) throw()
        {return (log10l(_X)); }
inline long double __CRTDECL log1p(_In_ long double _X) throw()
        {return (log1pl(_X)); }
inline long double __CRTDECL log2(_In_ long double _X) throw()
        {return (log2l(_X)); }
inline long double __CRTDECL logb(_In_ long double _X) throw()
        {return (logbl(_X)); }
inline long  __CRTDECL lrint(_In_ long double _X) throw()
        {return (lrintl(_X)); }
inline long  __CRTDECL lround(_In_ long double _X) throw()
        {return (lroundl(_X)); }
inline long double __CRTDECL modf(_In_ long double _X, _Out_ long double * _Y) throw()
        {return (modfl(_X, _Y)); }
inline long double __CRTDECL nearbyint(_In_ long double _X) throw()
        {return (nearbyintl(_X)); }
inline long double __CRTDECL nextafter(_In_ long double _X, _In_ long double _Y) throw()
        {return (nextafterl(_X, _Y)); }
inline long double __CRTDECL nexttoward(_In_ long double _X, _In_ long double _Y) throw()
        {return (nexttowardl(_X, _Y)); }
inline long double __CRTDECL pow(_In_ long double _X, _In_ long double _Y) throw()
        {return (powl(_X, _Y)); }
inline long double __CRTDECL pow(_In_ long double _X, _In_ int _Y) throw()
        {return (_Pow_int(_X, _Y)); }
inline long double __CRTDECL remainder(_In_ long double _X, _In_ long double _Y) throw()
        {return (remainderl(_X, _Y)); }
inline long double __CRTDECL remquo(_In_ long double _X, _In_ long double _Y, _Out_ int *_Z) throw()
        {return (remquol(_X, _Y, _Z)); }
inline long double __CRTDECL rint(_In_ long double _X) throw()
        {return (rintl(_X)); }
inline long double __CRTDECL round(_In_ long double _X) throw()
        {return (roundl(_X)); }
inline long double __CRTDECL scalbln(_In_ long double _X, _In_ long _Y) throw()
        {return (scalblnl(_X, _Y)); }
inline long double __CRTDECL scalbn(_In_ long double _X, _In_ int _Y) throw()
        {return (scalbnl(_X, _Y)); }
inline long double __CRTDECL sin(_In_ long double _X) throw()
        {return (sinl(_X)); }
inline long double __CRTDECL sinh(_In_ long double _X) throw()
        {return (sinhl(_X)); }
inline long double __CRTDECL sqrt(_In_ long double _X) throw()
        {return (sqrtl(_X)); }
inline long double __CRTDECL tan(_In_ long double _X) throw()
        {return (tanl(_X)); }
inline long double __CRTDECL tanh(_In_ long double _X) throw()
        {return (tanhl(_X)); }
inline long double __CRTDECL tgamma(_In_ long double _X) throw()
        {return (tgammal(_X)); }
inline long double __CRTDECL trunc(_In_ long double _X) throw()
        {return (truncl(_X)); }

}
#endif  /* __cplusplus */

#pragma pack(pop)

#endif  /* _INC_MATH */

#if defined (_USE_MATH_DEFINES) && !defined (_MATH_DEFINES_DEFINED)
#define _MATH_DEFINES_DEFINED

/* Define _USE_MATH_DEFINES before including math.h to expose these macro
 * definitions for common math constants.  These are placed under an #ifdef
 * since these commonly-defined names are not part of the C/C++ standards.
 */

/* Definitions of useful mathematical constants
 * M_E        - e
 * M_LOG2E    - log2(e)
 * M_LOG10E   - log10(e)
 * M_LN2      - ln(2)
 * M_LN10     - ln(10)
 * M_PI       - pi
 * M_PI_2     - pi/2
 * M_PI_4     - pi/4
 * M_1_PI     - 1/pi
 * M_2_PI     - 2/pi
 * M_2_SQRTPI - 2/sqrt(pi)
 * M_SQRT2    - sqrt(2)
 * M_SQRT1_2  - 1/sqrt(2)
 */

#define M_E        2.71828182845904523536
#define M_LOG2E    1.44269504088896340736
#define M_LOG10E   0.434294481903251827651
#define M_LN2      0.693147180559945309417
#define M_LN10     2.30258509299404568402
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.785398163397448309616
#define M_1_PI     0.318309886183790671538
#define M_2_PI     0.636619772367581343076
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2    1.41421356237309504880
#define M_SQRT1_2  0.707106781186547524401

#endif  /* defined (_USE_MATH_DEFINES) && !defined (_MATH_DEFINES_DEFINED) */
