/***
*atonexit.c - _onexit/atexit for using the MSVCRT* model of C run-time
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       atexit and _onexit are handled differently for EXE's and DLL's linked
*       with MSVCRT.LIB to call MSVCRT*.DLL.  Specifically, the _onexit/atexit
*       list for DLL's must be maintained on a per-module basis and must be
*       processed at DLL process detach .  For EXE's the atexit/_onexit list
*       should be maintained by MSVCRT*.DLL and processed at process exit.
*
*******************************************************************************/

/*
 * SPECIAL BUILD MACRO! Note that atonexit.c is linked in with the client's
 * code. It does not go into crtdll.dll! Therefore, it must be built under
 * the _DLL switch (like user code) and CRTDLL must be undefined.
 */

#undef  CRTDLL
#ifndef _DLL
#define _DLL
#endif  /* _DLL */

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <stdlib.h>
#include <mtdll.h>

#include <rterr.h>
#include <sect_attribs.h>
#include <dbgint.h>

int __cdecl __atonexitinit(void);
_CRTALLOC(".CRT$XIC") static _PIFV pinit = __atonexitinit;

/*
 * Pointers to beginning and end of the table of function pointers manipulated
 * by _onexit()/atexit().  If this module is an EXE, _onexitbegin will be -1.
 * Otherwise _onexitbegin will point to a block of malloc-ed memory used to
 * maintain the DLL module's private onexit list of terminator routines.
 * NOTE - the pointers are stored encoded.
 */

_PVFV *__onexitbegin;
_PVFV *__onexitend;

// This method initializes the onexit variables used by atonexit.
// Returns 0 if successful, or _RT_ONEXIT if there is no memory available
// for the function pointers table.
// Sometimes, __onexitinit is already initializing the onexit variables.
// __atonexitinit is needed for some corner cases
// where no other initialization is done (like for native process under managed).
// This is an alternate fix for performing the same initialization in mstartup.cpp,
// which causes _main symbol to be pulled in even when it shouldn't
// (by referencing extern __onexitbegin/__onexitend),
// causing unresolved symbol at link time.
int __cdecl __atonexitinit (
        void
        )
{
        _PVFV * onexitbegin;

        // Beginning and end of table should be either both initialized
        // or both uninitialized.
        _ASSERTE((__onexitbegin != NULL && __onexitend != NULL) ||
            (__onexitbegin == NULL && __onexitend == NULL));

        // Sometimes, __onexitbegin/__onexitend
        // might come already initialized via __onexitinit.
        if (__onexitbegin != NULL) {
            return 0;
        }

        onexitbegin = (_PVFV *) _calloc_crt(32, sizeof(_PVFV));
        __onexitend = __onexitbegin = (_PVFV *) EncodePointer(onexitbegin);

        if ( onexitbegin == NULL )
            /*
             * cannot allocate minimal required size. return
             * fatal runtime error.
             */
            return _RT_ONEXIT;

        *onexitbegin = (_PVFV) NULL;

        return 0;
}

/***
*_onexit, atexit - calls to _onexit & atexit in MSVCRT*.DLL
*
*Purpose:
*       A DLL linked with MSVCRT.LIB must not call the standard _onexit or
*       atexit exported from MSVCRT*.DLL, but an EXE linked with MSVCRT.LIB
*       will call the standard versions of those two routines.  The standard
*       names are not exported from MSVCRT*.DLL, but the _imp__* names are,
*       so this module can just invoke the standard versions if linked into
*       an EXE module (indicated by __onexitbegin == -1).  If this module is
*       linked into a DLL (indicated by __onexitbegin != -1), it will call a
*       helper routine in MSVCRT*.DLL called __dllonexit to manage the DLL's
*       private onexit list.
*
*Entry:
*       Same as the regular versions of _onexit, atexit.
*
*Exit:
*       Same as the regular versions of _onexit, atexit.
*
*Exceptions:
*
*******************************************************************************/

extern _onexit_t __dllonexit(_onexit_t, _PVFV**, _PVFV**);

#ifdef _M_IX86
extern _onexit_t (__cdecl *_imp___onexit)(_onexit_t func);
#else  /* _M_IX86 */
extern _onexit_t (__cdecl *__imp__onexit)(_onexit_t func);
#endif  /* _M_IX86 */

_onexit_t __cdecl _onexit (
        _onexit_t func
        )
{
        _PVFV * onexitbegin;
        _PVFV * onexitend;
        _onexit_t retval = NULL;

        onexitbegin = (_PVFV *) DecodePointer(__onexitbegin);

        if (onexitbegin == (_PVFV *)-1)
        {
            /* EXE */
#ifdef  _M_IX86
            return (*_imp___onexit)(func);
#else  /* _M_IX86 */
            return (*__imp__onexit)(func);
#endif  /* _M_IX86 */
        }

        /*
         * Note that we decode/encode the onexit array pointers on the
         * client side, not the CRT DLL side, to ease backwards compatibility.
         * That means we have to take a lock on this side, making the lock
         * found in __dllonexit redundant.
         */

        _lock(_EXIT_LOCK1);

        __try
        {
            onexitbegin = (_PVFV *) DecodePointer(__onexitbegin);
            onexitend   = (_PVFV *) DecodePointer(__onexitend);

            retval = __dllonexit((_onexit_t) EncodePointer(func), &onexitbegin, &onexitend);

            __onexitbegin = (_PVFV *) EncodePointer(onexitbegin);
            __onexitend   = (_PVFV *) EncodePointer(onexitend);
        }
        __finally
        {
            _unlock(_EXIT_LOCK1);
        }

        return retval;
}

int __cdecl atexit (
        _PVFV func
        )
{
        return (_onexit((_onexit_t)func) == NULL) ? -1 : 0;
}
