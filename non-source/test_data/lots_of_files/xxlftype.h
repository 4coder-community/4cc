/* xxlftype.h -- parameters for long double floating-point type */
#include <yvals.h>
#include <float.h>

#define FTYPE	long double
#define FCTYPE	_Lcomplex
#define FBITS	LDBL_MANT_DIG
#define FEPS	LDBL_EPSILON
#define FMAX	LDBL_MAX
#define FMIN	LDBL_MIN
#define FMAXEXP	LDBL_MAX_EXP

#define FFUN(fun)	fun##l
#define FMACRO(x)	L##x
#define FNAME(fun)	_L##fun
#define FCONST(obj)	_L##obj._Long_double
#define FLIT(lit)	lit##L

#define FDIV(x, y)	_FDIV(x, y)
#define FINVERT(x)	_FINVERT(x)

#define FPMSW(x)	(*_LPmsw(&(x)))
#define FSIGNBIT	_LSIGN

#define FISNEG(x)	(FPMSW(x) & FSIGNBIT)
#define FMAKENEG(x)	(FPMSW(x) |= FSIGNBIT)
#define FMAKEPOS(x)	(FPMSW(x) &= ~FSIGNBIT)
#define FNEGATE(x)	(FPMSW(x) ^= FSIGNBIT)

#define FSETLSB(x)	(*_LPlsw(&(x)) |= 1)

 #if _IS_EMBEDDED
#define FCPTYPE	float_complex	/* place holder */

 #else /* _IS_EMBEDDED */
#define FCPTYPE	complex<long double>
 #endif /* _IS_EMBEDDED */

/*
 * Copyright (c) 1992-2012 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V6.00:0009 */
