/***
*mbctype.h - MBCS character conversion macros
*
*       Copyright (c) Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       Defines macros for MBCS character classification/conversion.
*
*       [Public]
*
****/

#pragma once

#ifndef _INC_MBCTYPE
#define _INC_MBCTYPE

#include <crtdefs.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


/*
 * MBCS - Multi-Byte Character Set
 */

/*
 * This declaration allows the user access the _mbctype[] look-up array.
 */
#ifdef _CRTBLD
#if defined (_DLL) && defined (_M_IX86)
/* Retained for compatibility with VC++ 5.0 and earlier versions */
_Check_return_ _CRTIMP unsigned char * __cdecl __p__mbctype(void);
_Check_return_ _CRTIMP unsigned char * __cdecl __p__mbcasemap(void);
#endif  /* defined (_DLL) && defined (_M_IX86) */
#endif  /* _CRTBLD */

#if !defined (_M_CEE_PURE)
/* No data exports in pure code */
_CRTIMP extern unsigned char _mbctype[];
_CRTIMP extern unsigned char _mbcasemap[];
#else  /* !defined (_M_CEE_PURE) */
_Check_return_ _CRTIMP unsigned char * __cdecl __p__mbctype(void);
_Check_return_ _CRTIMP unsigned char * __cdecl __p__mbcasemap(void);
#define _mbctype   (__p__mbctype())
#define _mbcasemap (__p__mbcasemap())
#endif  /* !defined (_M_CEE_PURE) */

#ifdef _CRTBLD
extern pthreadmbcinfo __ptmbcinfo;
extern int __globallocalestatus;
extern int __locale_changed;
extern struct threadmbcinfostruct __initialmbcinfo;
pthreadmbcinfo __cdecl __updatetmbcinfo(void);
#endif  /* _CRTBLD */

/* bit masks for MBCS character types */

#define _MS     0x01    /* MBCS single-byte symbol */
#define _MP     0x02    /* MBCS punct */
#define _M1     0x04    /* MBCS 1st (lead) byte */
#define _M2     0x08    /* MBCS 2nd byte*/

#define _SBUP   0x10    /* SBCS upper char */
#define _SBLOW  0x20    /* SBCS lower char */

/* byte types  */

#define _MBC_SINGLE     0       /* valid single byte char */
#define _MBC_LEAD       1       /* lead byte */
#define _MBC_TRAIL      2       /* trailing byte */
#define _MBC_ILLEGAL    (-1)    /* illegal byte */

#define _KANJI_CP   932

/* _setmbcp parameter defines */
#define _MB_CP_SBCS     0
#define _MB_CP_OEM      -2
#define _MB_CP_ANSI     -3
#define _MB_CP_LOCALE   -4


#ifndef _MBCTYPE_DEFINED

/* MB control routines */

_CRTIMP int __cdecl _setmbcp(_In_ int _CodePage);
_CRTIMP int __cdecl _getmbcp(void);


/* MBCS character classification function prototypes */

#ifdef _CRT_USE_WINAPI_FAMILY_DESKTOP_APP

/* byte routines */
_Check_return_ _CRTIMP int __cdecl _ismbbkalnum( _In_ unsigned int _C );
_Check_return_ _CRTIMP int __cdecl _ismbbkalnum_l(_In_ unsigned int _C, _In_opt_ _locale_t _Locale);
_Check_return_ _CRTIMP int __cdecl _ismbbkana( _In_ unsigned int _C );
_Check_return_ _CRTIMP int __cdecl _ismbbkana_l(_In_ unsigned int _C, _In_opt_ _locale_t _Locale);
_Check_return_ _CRTIMP int __cdecl _ismbbkpunct( _In_ unsigned int _C );
_Check_return_ _CRTIMP int __cdecl _ismbbkpunct_l(_In_ unsigned int _C, _In_opt_ _locale_t _Locale);
_Check_return_ _CRTIMP int __cdecl _ismbbkprint( _In_ unsigned int _C );
_Check_return_ _CRTIMP int __cdecl _ismbbkprint_l(_In_ unsigned int _C, _In_opt_ _locale_t _Locale);
_Check_return_ _CRTIMP int __cdecl _ismbbalpha( _In_ unsigned int _C );
_Check_return_ _CRTIMP int __cdecl _ismbbalpha_l(_In_ unsigned int _C, _In_opt_ _locale_t _Locale);
_Check_return_ _CRTIMP int __cdecl _ismbbpunct( _In_ unsigned int _C );
_Check_return_ _CRTIMP int __cdecl _ismbbpunct_l(_In_ unsigned int _C, _In_opt_ _locale_t _Locale);
_Check_return_ _CRTIMP int __cdecl _ismbbblank( _In_ unsigned int _C );
_Check_return_ _CRTIMP int __cdecl _ismbbblank_l(_In_ unsigned int _C, _In_opt_ _locale_t _Locale);
_Check_return_ _CRTIMP int __cdecl _ismbbalnum( _In_ unsigned int _C );
_Check_return_ _CRTIMP int __cdecl _ismbbalnum_l(_In_ unsigned int _C, _In_opt_ _locale_t _Locale);
_Check_return_ _CRTIMP int __cdecl _ismbbprint( _In_ unsigned int _C );
_Check_return_ _CRTIMP int __cdecl _ismbbprint_l(_In_ unsigned int _C, _In_opt_ _locale_t _Locale);
_Check_return_ _CRTIMP int __cdecl _ismbbgraph( _In_ unsigned int _C );
_Check_return_ _CRTIMP int __cdecl _ismbbgraph_l(_In_ unsigned int _C, _In_opt_ _locale_t _Locale);

#ifndef _MBLEADTRAIL_DEFINED
_Check_return_ _CRTIMP int __cdecl _ismbblead( _In_ unsigned int _C);
_Check_return_ _CRTIMP int __cdecl _ismbblead_l(_In_ unsigned int _C, _In_opt_ _locale_t _Locale );
_Check_return_ _CRTIMP int __cdecl _ismbbtrail( _In_ unsigned int _C);
_Check_return_ _CRTIMP int __cdecl _ismbbtrail_l(_In_ unsigned int _C, _In_opt_ _locale_t _Locale );
_Check_return_ _CRTIMP int __cdecl _ismbslead(_In_reads_z_(_Pos - _Str+1) const unsigned char * _Str, _In_z_ const unsigned char * _Pos);
_Check_return_ _CRTIMP int __cdecl _ismbslead_l(_In_reads_z_(_Pos - _Str+1) const unsigned char * _Str, _In_z_ const unsigned char * _Pos, _In_opt_ _locale_t _Locale);
_Check_return_ _CRTIMP int __cdecl _ismbstrail(_In_reads_z_(_Pos - _Str+1) const unsigned char * _Str, _In_z_ const unsigned char * _Pos);
_Check_return_ _CRTIMP int __cdecl _ismbstrail_l(_In_reads_z_(_Pos - _Str+1) const unsigned char * _Str, _In_z_ const unsigned char * _Pos, _In_opt_ _locale_t _Locale);

#define _MBLEADTRAIL_DEFINED
#endif  /* _MBLEADTRAIL_DEFINED */

#endif  /* _CRT_USE_WINAPI_FAMILY_DESKTOP_APP */

#define _MBCTYPE_DEFINED
#endif  /* _MBCTYPE_DEFINED */

#ifdef _CRT_USE_WINAPI_FAMILY_DESKTOP_APP

/*
 * char byte classification macros
 */

#if defined (_CRT_DISABLE_PERFCRIT_LOCKS) && !defined (_DLL)
#define _ismbbkalnum(_c)    ((_mbctype+1)[(unsigned char)(_c)] & _MS)
#define _ismbbkprint(_c)    ((_mbctype+1)[(unsigned char)(_c)] & (_MS|_MP))
#define _ismbbkpunct(_c)    ((_mbctype+1)[(unsigned char)(_c)] & _MP)

#define _ismbbalnum(_c) (((_pctype)[(unsigned char)(_c)] & (_ALPHA|_DIGIT))||_ismbbkalnum(_c))
#define _ismbbalpha(_c) (((_pctype)[(unsigned char)(_c)] & (_ALPHA))||_ismbbkalnum(_c))
#define _ismbbgraph(_c) (((_pctype)[(unsigned char)(_c)] & (_PUNCT|_ALPHA|_DIGIT))||_ismbbkprint(_c))
#define _ismbbprint(_c) (((_pctype)[(unsigned char)(_c)] & (_BLANK|_PUNCT|_ALPHA|_DIGIT))||_ismbbkprint(_c))
#define _ismbbpunct(_c) (((_pctype)[(unsigned char)(_c)] & _PUNCT)||_ismbbkpunct(_c))
#define _ismbbblank(_c) (((_c) == '\t') ? _BLANK : (_pctype)[(unsigned char)(_c)] & _BLANK)

#define _ismbblead(_c)  ((_mbctype+1)[(unsigned char)(_c)] & _M1)
#define _ismbbtrail(_c) ((_mbctype+1)[(unsigned char)(_c)] & _M2)

#define _ismbbkana(_c)  ((_mbctype+1)[(unsigned char)(_c)] & (_MS|_MP))
#endif  /* defined (_CRT_DISABLE_PERFCRIT_LOCKS) && !defined (_DLL) */

#ifdef _CRTBLD
#define _ismbbalnum_l(_c, pt)  ((((pt)->locinfo->pctype)[(unsigned char)(_c)] & \
                                (_ALPHA|_DIGIT)) || \
                                (((pt)->mbcinfo->mbctype+1)[(unsigned char)(_c)] & _MS))
#define _ismbbalpha_l(_c, pt)  ((((pt)->locinfo->pctype)[(unsigned char)(_c)] & \
                            (_ALPHA)) || \
                            (((pt)->mbcinfo->mbctype+1)[(unsigned char)(_c)] & _MS))
#define _ismbbgraph_l(_c, pt)  ((((pt)->locinfo->pctype)[(unsigned char)(_c)] & \
                            (_PUNCT|_ALPHA|_DIGIT)) || \
                            (((pt)->mbcinfo->mbctype+1)[(unsigned char)(_c)] & (_MS|_MP)))
#define _ismbbprint_l(_c, pt)  ((((pt)->locinfo->pctype)[(unsigned char)(_c)] & \
                            (_BLANK|_PUNCT|_ALPHA|_DIGIT)) || \
                            (((pt)->mbcinfo->mbctype + 1)[(unsigned char)(_c)] & (_MS|_MP)))
#define _ismbbpunct_l(_c, pt)  ((((pt)->locinfo->pctype)[(unsigned char)(_c)] & _PUNCT) || \
                                (((pt)->mbcinfo->mbctype+1)[(unsigned char)(_c)] & _MP))
#define _ismbbblank_l(_c, pt)  (((_c) == '\t') ? _BLANK : (((pt)->locinfo->pctype)[(unsigned char)(_c)] & _BLANK) || \
                                (((pt)->mbcinfo->mbctype+1)[(unsigned char)(_c)] & _MP))
#define _ismbblead_l(_c, p)   ((p->mbcinfo->mbctype + 1)[(unsigned char)(_c)] & _M1)
#define _ismbbtrail_l(_c, p)  ((p->mbcinfo->mbctype + 1)[(unsigned char)(_c)] & _M2)
#endif  /* _CRTBLD */

#endif  /* _CRT_USE_WINAPI_FAMILY_DESKTOP_APP */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* _INC_MBCTYPE */
