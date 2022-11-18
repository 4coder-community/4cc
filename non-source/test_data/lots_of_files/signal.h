/***
*signal.h - defines signal values and routines
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file defines the signal values and declares the signal functions.
*       [ANSI/System V]
*
*       [Public]
*
****/

#pragma once

#ifndef _INC_SIGNAL
#define _INC_SIGNAL

#include <crtdefs.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* Define __cdecl for non-Microsoft compilers */

#ifndef _SIG_ATOMIC_T_DEFINED
typedef int sig_atomic_t;
#define _SIG_ATOMIC_T_DEFINED
#endif  /* _SIG_ATOMIC_T_DEFINED */

#define NSIG 23     /* maximum signal number + 1 */


/* Signal types */

#define SIGINT          2       /* interrupt */
#define SIGILL          4       /* illegal instruction - invalid function image */
#define SIGFPE          8       /* floating point exception */
#define SIGSEGV         11      /* segment violation */
#define SIGTERM         15      /* Software termination signal from kill */
#define SIGBREAK        21      /* Ctrl-Break sequence */
#define SIGABRT         22      /* abnormal termination triggered by abort call */

#define SIGABRT_COMPAT  6       /* SIGABRT compatible with other platforms, same as SIGABRT */

#ifndef _M_CEE_PURE
/* signal action codes */

#define SIG_DFL (void (__cdecl *)(int))0           /* default signal action */
#define SIG_IGN (void (__cdecl *)(int))1           /* ignore signal */
#define SIG_GET (void (__cdecl *)(int))2           /* return current value */
#define SIG_SGE (void (__cdecl *)(int))3           /* signal gets error */
#define SIG_ACK (void (__cdecl *)(int))4           /* acknowledge */

#ifdef _CRTBLD
/* internal use only! not valid as an argument to signal() */

#define SIG_DIE (void (__cdecl *)(int))5           /* terminate process */
#endif  /* _CRTBLD */

/* signal error value (returned by signal call on error) */

#define SIG_ERR (void (__cdecl *)(int))-1          /* signal error value */
#else  /* _M_CEE_PURE */
/* signal action codes */

#define SIG_DFL (void (__clrcall *)(int))0           /* default signal action */
#define SIG_IGN (void (__clrcall *)(int))1           /* ignore signal */
#define SIG_GET (void (__clrcall *)(int))2           /* return current value */
#define SIG_SGE (void (__clrcall *)(int))3           /* signal gets error */
#define SIG_ACK (void (__clrcall *)(int))4           /* acknowledge */

#ifdef _CRTBLD
/* internal use only! not valid as an argument to signal() */

#define SIG_DIE (void (__clrcall *)(int))5           /* terminate process */
#endif  /* _CRTBLD */

/* signal error value (returned by signal call on error) */

#define SIG_ERR (void (__clrcall *)(int))-1          /* signal error value */
#endif  /* _M_CEE_PURE */


/* pointer to exception information pointers structure */

extern void * * __cdecl __pxcptinfoptrs(void);
#define _pxcptinfoptrs  (*__pxcptinfoptrs())

/* Function prototypes */

#ifndef _M_CEE_PURE
_CRTIMP void (__cdecl * __cdecl signal(_In_ int _SigNum, _In_opt_ void (__cdecl * _Func)(int)))(int);
#endif  /* _M_CEE_PURE */

_CRTIMP int __cdecl raise(_In_ int _SigNum);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* _INC_SIGNAL */
