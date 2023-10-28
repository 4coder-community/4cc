/***
*share.h - defines file sharing modes for sopen
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file defines the file sharing modes for sopen().
*
*       [Public]
*
****/

#pragma once

#ifndef _INC_SHARE
#define _INC_SHARE

#if !defined (_WIN32)
#error ERROR: Only Win32 target supported!
#endif  /* !defined (_WIN32) */

#define _SH_DENYRW      0x10    /* deny read/write mode */
#define _SH_DENYWR      0x20    /* deny write mode */
#define _SH_DENYRD      0x30    /* deny read mode */
#define _SH_DENYNO      0x40    /* deny none mode */
#define _SH_SECURE      0x80    /* secure mode */

#if !__STDC__
/* Non-ANSI names for compatibility */
#define SH_DENYRW _SH_DENYRW
#define SH_DENYWR _SH_DENYWR
#define SH_DENYRD _SH_DENYRD
#define SH_DENYNO _SH_DENYNO
#endif  /* !__STDC__ */

#endif  /* _INC_SHARE */
