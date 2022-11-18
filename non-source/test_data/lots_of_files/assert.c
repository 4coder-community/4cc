/***
*assert.c - Display a message and abort
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*******************************************************************************/

#include <cruntime.h>
#include <windows.h>
#include <file2.h>
#include <internal.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <signal.h>
#include <awint.h>
#include <limits.h>
#include <dbgint.h>

#ifdef NDEBUG
#undef NDEBUG
#endif  /* NDEBUG */
#define _ASSERT_OK
#include <assert.h>

extern IMAGE_DOS_HEADER __ImageBase;


/*
 * assertion format string for use with output to stderr
 */
static TCHAR _assertstring[] = _T("Assertion failed: %s, file %s, line %d\n");

/*      Format of MessageBox for assertions:
*
*       ================= Microsft Visual C++ Debug Library ================
*
*       Assertion Failed!
*
*       Program: c:\test\mytest\foo.exe
*       File: c:\test\mytest\bar.c
*       Line: 69
*
*       Expression: <expression>
*
*       For information on how your program can cause an assertion
*       failure, see the Visual C++ documentation on asserts
*
*       (Press Retry to debug the application - JIT must be enabled)
*
*       ===================================================================
*/

/*
 * assertion string components for message box
 */
#define BOXINTRO    _T("Assertion failed!")
#define PROGINTRO   _T("Program: ")
#define FILEINTRO   _T("File: ")
#define LINEINTRO   _T("Line: ")
#define EXPRINTRO   _T("Expression: ")
#define INFOINTRO   _T("For information on how your program can cause an assertion\n") \
                    _T("failure, see the Visual C++ documentation on asserts")
#define HELPINTRO   _T("(Press Retry to debug the application - JIT must be enabled)")

static TCHAR * dotdotdot = _T("...");
static TCHAR * newline = _T("\n");
static TCHAR * dblnewline = _T("\n\n");

#define DOTDOTDOTSZ 3
#define NEWLINESZ   1
#define DBLNEWLINESZ   2

#define MAXLINELEN  64 /* max length for line in message box */
#define ASSERTBUFSZ (MAXLINELEN * 9) /* 9 lines in message box */

#ifdef _UNICODE
void __cdecl _assert (const char *, const char *,unsigned);
#endif  /* _UNICODE */

/***
*_assert() - Display a message and abort
*
*Purpose:
*       The assert macro calls this routine if the assert expression is
*       true.  By placing the assert code in a subroutine instead of within
*       the body of the macro, programs that call assert multiple times will
*       save space.
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

#ifdef _UNICODE
void __cdecl _wassert (
        const wchar_t *expr,
        const wchar_t *filename,
        unsigned lineno
        )
#else  /* _UNICODE */
void __cdecl _assert (

        const char *expr,
        const char *filename,
        unsigned lineno
        )
#endif  /* _UNICODE */
{
#if !defined (_CRT_APP) || defined (_DEBUG)
        /*
         * Build the assertion message, then write it out. The exact form
         * depends on whether it is to be written out via stderr or the
         * MessageBox API.
         */
        if ( (_set_error_mode(_REPORT_ERRMODE)== _OUT_TO_STDERR) ||
             ((_set_error_mode(_REPORT_ERRMODE) == _OUT_TO_DEFAULT) &&
              (__app_type == _CONSOLE_APP)) )
        {
#ifdef _UNICODE
            {
                TCHAR assertbuf[ASSERTBUFSZ];
                HANDLE hErr ;
                DWORD written;

                hErr = GetStdHandle(STD_ERROR_HANDLE);

                if(hErr!=INVALID_HANDLE_VALUE && hErr!=NULL)
                {
                    if(swprintf(assertbuf, ASSERTBUFSZ,_assertstring,expr,filename,lineno) >= 0)
                    {
                        if(GetFileType(hErr) == FILE_TYPE_CHAR)
                        {
                            if(WriteConsoleW(hErr, assertbuf, (unsigned long)wcslen(assertbuf), &written, NULL))
                            {
                                abort();
                            }
                        }
                    }
                }
            }

#endif  /* _UNICODE */

            /*
             * Build message and write it out to stderr. It will be of the
             * form:
             *        Assertion failed: <expr>, file <filename>, line <lineno>
             */
            if ( !anybuf(stderr) )
            /*
             * stderr is unused, hence unbuffered, as yet. set it to
             * single character buffering (to avoid a malloc() of a
             * stream buffer).
             */
             (void) setvbuf(stderr, NULL, _IONBF, 0);


            _ftprintf(stderr, _assertstring, expr, filename, lineno);
            fflush(stderr);

        }
        /* Display the message using a message box */
        else
#endif  /* !defined (_CRT_APP) || defined (_DEBUG) */
        {
            int nCode;
            TCHAR * pch;
            TCHAR assertbuf[ASSERTBUFSZ];
            TCHAR progname[MAX_PATH + 1];
            HMODULE hModule = NULL;

            /*
             * Line 1: box intro line
             */
            _ERRCHECK(_tcscpy_s( assertbuf, ASSERTBUFSZ, BOXINTRO ));
            _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, dblnewline ));

            /*
             * Line 2: program line
             */
            _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, PROGINTRO ));

#if !defined (_DEBUG) && defined (_CRT_APP)
            _ERRCHECK(_tcscpy_s( progname, MAX_PATH + 1, _T("Application")));
#else  /* !defined (_DEBUG) && defined (_CRT_APP) */
            if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR) _ReturnAddress(), &hModule))
            {
                hModule = NULL;
            }
#ifdef CRTDLL
            else if (hModule == (HMODULE)&__ImageBase)
            {
                // This is CRT DLL.  Use the EXE instead
                hModule = NULL;
            }
#endif  /* CRTDLL */
#endif  /* !defined(_DEBUG) && defined(_CRT_APP) */

#if !defined(_DEBUG) && defined(_CRT_APP)
            _ERRCHECK(_tcscpy_s( progname, MAX_PATH + 1, _T("Application")));
#else
            progname[MAX_PATH] = _T('\0');
            if ( !GetModuleFileName(hModule, progname, MAX_PATH))
                _ERRCHECK(_tcscpy_s( progname, MAX_PATH + 1, _T("<program name unknown>")));
#endif  /* !defined (_DEBUG) && defined (_CRT_APP) */
            pch = (TCHAR *)progname;

            /* sizeof(PROGINTRO) includes the NULL terminator */
            if ( (sizeof(PROGINTRO)/sizeof(TCHAR)) + _tcslen(progname) + NEWLINESZ > MAXLINELEN )
            {
                pch += ((sizeof(PROGINTRO)/sizeof(TCHAR)) + _tcslen(progname) + NEWLINESZ) - MAXLINELEN;
                /* Only replace first (sizeof(TCHAR) * DOTDOTDOTSZ) bytes to ellipsis */
                _ERRCHECK(memcpy_s(pch, sizeof(TCHAR) * ((MAX_PATH + 1) - (pch - progname)), dotdotdot, sizeof(TCHAR) * DOTDOTDOTSZ ));

            }

            _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, pch ));
            _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, newline ));

            /*
             * Line 3: file line
             */
            _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, FILEINTRO ));

            /* sizeof(FILEINTRO) includes the NULL terminator */
            if ( (sizeof(FILEINTRO)/sizeof(TCHAR)) + _tcslen(filename) + NEWLINESZ > MAXLINELEN )
            {
                size_t p, len, ffn;

                pch = (TCHAR *) filename;
                ffn = MAXLINELEN - (sizeof(FILEINTRO)/sizeof(TCHAR)) - NEWLINESZ;

                for ( len = _tcslen(filename), p = 1;
                      pch[len - p] != _T('\\') && pch[len - p] != _T('/') && p < len;
                      p++ );

                /* keeping pathname almost 2/3rd of full filename and rest
                 * is filename
                 */
                if ( (ffn - ffn/3) < (len - p) && ffn/3 > p )
                {
                    /* too long. using first part of path and the
                       filename string */
                    _ERRCHECK(_tcsncat_s( assertbuf, ASSERTBUFSZ, pch, ffn - DOTDOTDOTSZ - p ));
                    _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, dotdotdot ));
                    _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, pch + len - p ));
                }
                else if ( ffn - ffn/3 > len - p )
                {
                    /* pathname is smaller. keeping full pathname and putting
                     * dotdotdot in the middle of filename
                     */
                    p = p/2;
                    _ERRCHECK(_tcsncat_s( assertbuf, ASSERTBUFSZ, pch, ffn - DOTDOTDOTSZ - p ));
                    _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, dotdotdot ));
                    _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, pch + len - p ));
                }
                else
                {
                    /* both are long. using first part of path. using first and
                     * last part of filename.
                     */
                    _ERRCHECK(_tcsncat_s( assertbuf, ASSERTBUFSZ, pch, ffn - ffn/3 - DOTDOTDOTSZ ));
                    _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, dotdotdot ));
                    _ERRCHECK(_tcsncat_s( assertbuf, ASSERTBUFSZ, pch + len - p, ffn/6 - 1 ));
                    _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, dotdotdot ));
                    _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, pch + len - (ffn/3 - ffn/6 - 2) ));
                }

            }
            else
                /* plenty of room on the line, just append the filename */
                _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, filename ));

            _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, newline ));

            /*
             * Line 4: line line
             */
            _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, LINEINTRO ));
            _ERRCHECK(_itot_s( lineno, assertbuf + _tcslen(assertbuf), ASSERTBUFSZ - _tcslen(assertbuf), 10 ));
            _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, dblnewline ));

            /*
             * Line 5: message line
             */
            _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, EXPRINTRO ));

            /* sizeof(HELPINTRO) includes the NULL terminator */

            if (    _tcslen(assertbuf) +
                    _tcslen(expr) +
                    2*DBLNEWLINESZ +
                    (sizeof(INFOINTRO)/sizeof(TCHAR)) - 1 +
                    (sizeof(HELPINTRO)/sizeof(TCHAR)) > ASSERTBUFSZ )
            {
                _ERRCHECK(_tcsncat_s( assertbuf, ASSERTBUFSZ, expr,
                    ASSERTBUFSZ -
                    (_tcslen(assertbuf) +
                    DOTDOTDOTSZ +
                    2*DBLNEWLINESZ +
                    (sizeof(INFOINTRO)/sizeof(TCHAR))-1 +
                    (sizeof(HELPINTRO)/sizeof(TCHAR)))));
                _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, dotdotdot ));
            }
            else
                _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, expr ));

            _ERRCHECK(_tcscat_s( assertbuf, ASSERTBUFSZ, dblnewline ));

            /*
             * Line 6, 7: info line
             */

            _ERRCHECK(_tcscat_s(assertbuf, ASSERTBUFSZ, INFOINTRO));
            _ERRCHECK(_tcscat_s(assertbuf, ASSERTBUFSZ, dblnewline ));

            /*
             * Line 8: help line
             */
            _ERRCHECK(_tcscat_s(assertbuf, ASSERTBUFSZ, HELPINTRO));

#if !defined (_DEBUG) && defined (_CRT_APP)
            /*
             * Break if debugger is attached only.
             */
            if (IsDebuggerPresent())
            {
                nCode = IDRETRY;
#ifdef _UNICODE
                OutputDebugStringW(assertbuf);
#endif  /* _UNICODE */
            }
            else
            {
                nCode = IDABORT;
            }
#else  /* !defined (_DEBUG) && defined (_CRT_APP) */
            /*
             * Write out via MessageBox
             */
            nCode = __crtMessageBox(assertbuf,
                _T("Microsoft Visual C++ Runtime Library"),
                MB_TASKMODAL|MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SETFOREGROUND);
#endif  /* !defined (_DEBUG) && defined (_CRT_APP) */

            /* Abort: abort the program */
            if (nCode == IDABORT)
            {
                /* raise abort signal */
                raise(SIGABRT);

                /* We usually won't get here, but it's possible that
                   SIGABRT was ignored.  So exit the program anyway. */

                _exit(3);
            }

            /* Retry: call the debugger */
            if (nCode == IDRETRY)
            {
                __debugbreak();
                /* return to user code */
                return;
            }

            /* Ignore: continue execution */
            if (nCode == IDIGNORE)
                return;
        }

        abort();
}
