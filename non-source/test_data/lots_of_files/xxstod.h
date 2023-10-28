/* xxstod.h -- _[W]Sto[d f ld] common functionality */

 #pragma warning(disable: 4210)

		/* macros */
#define ACSIZE	3	/* size of extended-precision accumulators */

#define D16TO7	FLIT(268435456.0)	/* 16^7 */
#define D10TO9	FLIT(1e9)			/* 10^9 */
#define NLONG	((FBITS + 27) / 28)	/* 7 * NLONG == max hexadecimal digits */

/*
FTYPE _Stodx(const CTYPE *s, CTYPE **endptr, long pten, int *perr)
 */
	{	/* convert string to FTYPE, with checking */
	FTYPE x;
	long lo[NLONG + 1];
	const CTYPE *s0 = s;
	int code = CNAME(Stopfx)(&s, endptr);
	const int neg = code & FL_NEG;

	extern _CRTIMP2_PURE FTYPE __CLRCALL_PURE_OR_CDECL FNAME(Dtento)(FTYPE *, long, int *);

	if (perr != 0)
		*perr = 0;
	if ((code &= ~FL_NEG) == FL_DEC)
		{	/* parse decimal format */
		const int nlo = CNAME(Stoflt)(s0, s, endptr, lo, NLONG);
		FTYPE xpx[ACSIZE], xpf[ACSIZE];
		int i;

		FNAME(Xp_setw)(xpf, ACSIZE, D10TO9);
		if (nlo == 0)
			FNAME(Xp_setw)(xpx, ACSIZE, 0.0);
		else
			for (i = 1, FNAME(Xp_setn)(xpx, ACSIZE, lo[1]); i < nlo; )
				{	/* x = x * D10TO9 + (FTYPE)lo[++i] */
				FTYPE xpa[ACSIZE];
				FTYPE xpt[ACSIZE * 2];

				FNAME(Xp_mulx)(xpx, ACSIZE, xpf, ACSIZE, xpt);
				FNAME(Xp_setn)(xpa, ACSIZE, lo[++i]);
				FNAME(Xp_addx)(xpx, ACSIZE, xpa, ACSIZE);
				}
		x = FNAME(Dtento)(xpx, pten + lo[0], perr);
		}
	else if (code == FL_HEX)
		{	/* parse hexadecimal format */
		const int nlo = CNAME(Stoxflt)(s0, s, endptr, lo, NLONG);
		FTYPE xpx[ACSIZE], xpf[ACSIZE];
		int i;

		FNAME(Xp_setw)(xpf, ACSIZE, D16TO7);
		if (nlo == 0)
			FNAME(Xp_setw)(xpx, ACSIZE, 0.0);
		else
			for (i = 1, FNAME(Xp_setn)(xpx, ACSIZE, lo[1]); i < nlo; )
				{	/* x = x * D10TO9 + (FTYPE)lo[++i] */
				FTYPE xpa[ACSIZE];
				FTYPE xpt[ACSIZE * 2];

				FNAME(Xp_mulx)(xpx, ACSIZE, xpf, ACSIZE, xpt);
				FNAME(Xp_setn)(xpa, ACSIZE, lo[++i]);
				FNAME(Xp_addx)(xpx, ACSIZE, xpa, ACSIZE);
				}
		x = FNAME(Dtento)(xpx, pten, perr);
		FNAME(Dscale)(&x, lo[0]);
		}
	else if (code == FL_INF)
		x = FCONST(Inf);
	else if (code == FL_NAN)
		x = FCONST(Nan);
	else
		x = FLIT(0.0);	/* code == FL_ERR */

	if (neg)
		FNEGATE(x);
	return (x);
	}

/*
 * Copyright (c) 1992-2012 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V6.00:0009 */
