/***
*setjmpex.h - definitions/declarations for extended setjmp/longjmp routines
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file causes _setjmpex to be called which will enable safe
*       setjmp/longjmp that work correctly with try/except/finally.
*
*       [Public]
*
****/

#pragma once

#ifndef _INC_SETJMPEX
#define _INC_SETJMPEX

#if !defined (_WIN32)
#error ERROR: Only Win32 target supported!
#endif  /* !defined (_WIN32) */

/*
 * Definitions specific to particular setjmp implementations.
 */

#if defined (_M_IX86)

/*
 * MS compiler for x86
 */

#define setjmp  _setjmp
#define longjmp _longjmpex

#else  /* defined (_M_IX86) */

#ifdef setjmp
#undef setjmp
#endif  /* setjmp */
#define setjmp _setjmpex

#endif  /* defined (_M_IX86) */

#include <setjmp.h>

#endif  /* _INC_SETJMPEX */
