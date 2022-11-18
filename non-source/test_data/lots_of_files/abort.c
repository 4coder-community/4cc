/***
*abort.c - abort a program by raising SIGABRT
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       defines abort() - print a message and raise SIGABRT.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdlib.h>
#include <internal.h>
#include <awint.h>
#include <rterr.h>
#include <signal.h>
#include <oscalls.h>
#include <mtdll.h>
#include <dbgint.h>

#ifdef _DEBUG
#define _INIT_ABORT_BEHAVIOR _WRITE_ABORT_MSG
#else  /* _DEBUG */
#define _INIT_ABORT_BEHAVIOR _CALL_REPORTFAULT
#endif  /* _DEBUG */

unsigned int __abort_behavior = _INIT_ABORT_BEHAVIOR;

/***
*void abort() - abort the current program by raising SIGABRT
*
*Purpose:
*   print out an abort message and raise the SIGABRT signal.  If the user
*   hasn't defined an abort handler routine, terminate the program
*   with exit status of 3 without cleaning up.
*
*   Multi-thread version does not raise SIGABRT -- this isn't supported
*   under multi-thread.
*
*Entry:
*   None.
*
*Exit:
*   Does not return.
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

void __cdecl abort (
        void
        )
{
    _PHNDLR sigabrt_act = SIG_DFL;

#ifdef _DEBUG
    if (__abort_behavior & _WRITE_ABORT_MSG)
    {
        /* write the abort message */
        _NMSG_WRITE(_RT_ABORT);
    }
#endif  /* _DEBUG */


    /* Check if the user installed a handler for SIGABRT.
     * We need to read the user handler atomically in the case
     * another thread is aborting while we change the signal
     * handler.
     */
    sigabrt_act = __get_sigabrt();
    if (sigabrt_act != SIG_DFL)
    {
        raise(SIGABRT);
    }

    /* If there is no user handler for SIGABRT or if the user
     * handler returns, then exit from the program anyway
     */

    if (__abort_behavior & _CALL_REPORTFAULT)
    {
#if defined (_M_ARM) || defined (_CRT_APP)
        __fastfail(FAST_FAIL_FATAL_APP_EXIT);
#else  /* defined (_M_ARM) || defined (_CRT_APP) */
        if (IsProcessorFeaturePresent(PF_FASTFAIL_AVAILABLE))
            __fastfail(FAST_FAIL_FATAL_APP_EXIT);

        _call_reportfault(_CRT_DEBUGGER_ABORT, STATUS_FATAL_APP_EXIT, EXCEPTION_NONCONTINUABLE);
#endif  /* defined (_M_ARM) || defined (_CRT_APP) */
    }


    /* If we don't want to call ReportFault, then we call _exit(3), which is the
     * same as invoking the default handler for SIGABRT
     */


    _exit(3);
}

/***
*unsigned int _set_abort_behavior(unsigned int, unsigned int) - set the behavior on abort
*
*Purpose:
*
*Entry:
*   unsigned int flags - the flags we want to set
*   unsigned int mask - mask the flag values
*
*Exit:
*   Return the old behavior flags
*
*Exceptions:
*   None
*
*******************************************************************************/

unsigned int __cdecl _set_abort_behavior(unsigned int flags, unsigned int mask)
{
    unsigned int oldflags = __abort_behavior;
    __abort_behavior = oldflags & (~mask) | flags & mask;
    return oldflags;
}
