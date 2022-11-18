/***
*awint.h - internal definitions for A&W Win32 wrapper routines.
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Contains internal definitions/declarations for A&W wrapper functions.
*       Not included in internal.h since windows.h is required for these.
*
*       [Internal]
*
****/

#pragma once

#ifdef _WIN32

#ifndef _INC_AWINC
#define _INC_AWINC

#ifndef _CRTBLD
/*
 * This is an internal C runtime header file. It is used when building
 * the C runtimes only. It is not to be used as a public header file.
 */
#error ERROR: Use of C runtime library internal header file.
#endif  /* _CRTBLD */

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include <sal.h>
#include <windows.h>
#include <sdkddkver.h>

// Minimum supported versions of Windows:
// NTDDI_WINXPSP2 for x86 and x64
// NTDDI_WIN8     for ARM, CoreSys, App and KernelX

#if (defined (_M_IX86) || defined (_M_X64)) && !defined (_CORESYS) && !defined (_CRT_APP) && !defined (_KERNELX)
#define _CRT_NTDDI_MIN NTDDI_WINXPSP2
#else  /* (defined (_M_IX86) || defined (_M_X64)) && !defined (_CORESYS) && !defined (_CRT_APP) && !defined (_KERNELX) */
#define _CRT_NTDDI_MIN NTDDI_WIN8
#endif  /* (defined (_M_IX86) || defined (_M_X64)) && !defined (_CORESYS) && !defined (_CRT_APP) && !defined (_KERNELX) */

/* internal A&W routines */
struct  threadlocaleinfostruct;
typedef struct threadlocaleinfostruct * pthreadlocinfo;

// Fast fail error codes
#define FAST_FAIL_VTGUARD_CHECK_FAILURE       1
#define FAST_FAIL_STACK_COOKIE_CHECK_FAILURE  2
#define FAST_FAIL_CORRUPT_LIST_ENTRY          3
#define FAST_FAIL_INCORRECT_STACK             4
#define FAST_FAIL_INVALID_ARG                 5
#define FAST_FAIL_GS_COOKIE_INIT              6
#define FAST_FAIL_FATAL_APP_EXIT              7
#define FAST_FAIL_RANGE_CHECK_FAILURE         8

// Remove when winnt.h has the definition
#ifndef PF_FASTFAIL_AVAILABLE
#define PF_FASTFAIL_AVAILABLE                23
#endif  /* PF_FASTFAIL_AVAILABLE */

int __cdecl __crtCompareStringW
(
    _In_ LPCWSTR _LocaleName,
    _In_ DWORD      _DwCmpFlags,
    _In_reads_(_CchCount1) LPCWSTR  _LpString1,
    _In_ int      _CchCount1,
    _In_reads_(_CchCount2) LPCWSTR  _LpString2,
    _In_ int      _CchCount2
);

int __cdecl __crtCompareStringA
(
  _In_opt_ _locale_t _Plocinfo,
  _In_ LPCWSTR _LocaleName,
  _In_ DWORD    _DwCmpFlags,
  _In_reads_(_CchCount1) LPCSTR   _LpString1,
  _In_ int      _CchCount1,
  _In_reads_(_CchCount2) LPCSTR   _LpString2,
  _In_ int      _CchCount2,
  _In_ int      _Code_page);

int __cdecl __crtGetLocaleInfoA
(
  _In_opt_ _locale_t _Plocinfo,
  _In_ LPCWSTR _LocaleName,
  _In_ LCTYPE  _LCType,
  _Out_writes_opt_(_CchData) LPSTR   _LpLCData,
  _In_ int     _CchData);

int __cdecl __crtLCMapStringW
(
  _In_ LPCWSTR _LocaleName,
  _In_ DWORD _DWMapFlag,
  _In_reads_(_CchSrc) LPCWSTR _LpSrcStr,
  _In_ int _CchSrc,
  _Out_writes_opt_(_CchDest) LPWSTR _LpDestStr,
  _In_ int _CchDest);

int __cdecl __crtLCMapStringA
(
  _In_opt_ _locale_t _Plocinfo,
  _In_ LPCWSTR _LocaleName,
  _In_ DWORD _DwMapFlag,
  _In_reads_(_CchSrc) LPCSTR _LpSrcStr,
  _In_ int _CchSrc,
  _Out_writes_opt_(_CchDest) LPSTR _LpDestStr,
  _In_ int _CchDest,
  _In_ int _Code_page,
  _In_ BOOL _BError);

BOOL __cdecl __crtGetStringTypeA
(
  _In_opt_ _locale_t _Plocinfo,
  _In_ DWORD _DWInfoType,
  _In_ LPCSTR _LpSrcStr,
  _In_ int _CchSrc,
  _Out_ LPWORD _LpCharType,
  _In_ int _Code_page,
  _In_ BOOL _BError);

LPVOID __cdecl __crtGetEnvironmentStringsA(VOID);
LPVOID __cdecl __crtGetEnvironmentStringsW(VOID);

int __cdecl __crtMessageBoxA
(
    _In_ LPCSTR _LpText,
    _In_ LPCSTR _LpCaption,
    _In_ UINT _UType);

int __cdecl __crtMessageBoxW
(
    _In_ LPCWSTR _LpText,
    _In_ LPCWSTR _LpCaption,
    _In_ UINT _UType);

/* Helper function for Packaged apps */
_CRTIMP BOOL __cdecl __crtIsPackagedApp(void);

_CRTIMP WORD __cdecl __crtGetShowWindowMode(void);

_CRTIMP void __cdecl __crtSetUnhandledExceptionFilter
(
  _In_ LPTOP_LEVEL_EXCEPTION_FILTER exceptionFilter);

#if defined (_M_IX86) || defined (_M_X64)

_CRTIMP LONG __cdecl __crtUnhandledException
(
  _In_ EXCEPTION_POINTERS *exceptionInfo);

_CRTIMP void __cdecl __crtTerminateProcess
(
  _In_ UINT uExitCode);
#endif  /* defined (_M_IX86) || defined (_M_X64) */

#if defined (_M_X64)
_CRTIMP void __cdecl __crtCaptureCurrentContext
(
  _Out_ CONTEXT *pContextRecord);

_CRTIMP void __cdecl __crtCapturePreviousContext
(
  _Out_ CONTEXT *pContextRecord);
#endif  /* defined (_M_X64) */

#if _CRT_NTDDI_MIN >= NTDDI_VISTA 

/* Helper functions for thread-level storage Win32 APIs */
#define __crtFlsAlloc(lpCallback) \
             FlsAlloc(lpCallback)

#define __crtFlsFree(dwFlsIndex) \
             FlsFree(dwFlsIndex)

#define __crtFlsGetValue(dwFlsIndex) \
             FlsGetValue(dwFlsIndex)

#define __crtFlsSetValue(dwFlsIndex,lpFlsData) \
             FlsSetValue(dwFlsIndex,lpFlsData)

#define __crtInitializeCriticalSectionEx(lpCriticalSection, dwSpinCount, Flags)   \
             InitializeCriticalSectionEx(lpCriticalSection, dwSpinCount, Flags)

#define __crtCreateEventExW(lpEventAttributes, lpName, dwFlags, dwDesiredAccess) \
             CreateEventExW(lpEventAttributes, lpName, dwFlags, dwDesiredAccess)

#define __crtCreateSemaphoreExW(lpSemaphoreAttributes, lInitialCount, lMaximumCount, lpName, dwFlags, dwDesiredAccess) \
             CreateSemaphoreExW(lpSemaphoreAttributes, lInitialCount, lMaximumCount, lpName, dwFlags, dwDesiredAccess)

#define __crtSetThreadStackGuarantee(StackSizeInBytes)  \
        SetThreadStackGuarantee(StackSizeInBytes)

#define __crtCreateThreadpoolTimer(pfnti, pv, pcbe) \
             CreateThreadpoolTimer(pfnti, pv, pcbe)

#define __crtSetThreadpoolTimer(pti, pftDueTime, msPeriod, msWindowLength)  \
             SetThreadpoolTimer(pti, pftDueTime, msPeriod, msWindowLength)

#define __crtWaitForThreadpoolTimerCallbacks(pti, fCancelPendingCallbacks)  \
             WaitForThreadpoolTimerCallbacks(pti, fCancelPendingCallbacks)

#define __crtCloseThreadpoolTimer(pti)  \
             CloseThreadpoolTimer(pti)

#define __crtCreateThreadpoolWait(pfnwa, pv, pcbe)  \
        CreateThreadpoolWait(pfnwa, pv, pcbe)

#define __crtSetThreadpoolWait(pwa, h, pftTimeout)  \
             SetThreadpoolWait(pwa, h, pftTimeout)

#define __crtCloseThreadpoolWait(pwa)  \
             CloseThreadpoolWait(pwa)

// wrapper for FlushProcessWriteBuffers, available only on Vista+
#define __crtFlushProcessWriteBuffers()  \
             FlushProcessWriteBuffers()

// wrapper for FreeLibraryWhenCallbackReturns, available only on Vista+
#define __crtFreeLibraryWhenCallbackReturns(pci, mod)  \
             FreeLibraryWhenCallbackReturns(pci, mod)

// wrapper for GetCurrentProcessorNumber, available only on Vista+
#define __crtGetCurrentProcessorNumber()  \
             GetCurrentProcessorNumber()

// wrapper for GetLogicalProcessorInformation, available only on Vista+
#define __crtGetLogicalProcessorInformation(Buffer, ReturnLength)  \
             GetLogicalProcessorInformation(Buffer, ReturnLength)

// wrapper for CreateSymbolicLink, available only on Vista+
#define __crtCreateSymbolicLinkW(lpSymlinkFileName, lpTargetFileName, dwFlags)  \
             CreateSymbolicLinkW(lpSymlinkFileName, lpTargetFileName, dwFlags)

#define __crtCompareStringEx(lpLocaleName, dwCmpFlags, lpString1, cchCount1, lpString2, cchCount2) \
             CompareStringEx(lpLocaleName, dwCmpFlags, lpString1, cchCount1, lpString2, cchCount2, NULL, NULL, 0)

#define __crtEnumSystemLocalesEx(lpLocaleEnumProcEx, dwFlags, lParam) \
             EnumSystemLocalesEx(lpLocaleEnumProcEx, dwFlags, lParam, NULL)

#define __crtGetDateFormatEx(lpLocaleName, dwFlags, lpDate, lpFormat, lpDateStr, cchDate) \
             GetDateFormatEx(lpLocaleName, dwFlags, lpDate, lpFormat, lpDateStr, cchDate, NULL)

#define __crtGetLocaleInfoEx(lpLocaleName, LCType, lpLCData, cchData) \
             GetLocaleInfoEx(lpLocaleName, LCType, lpLCData, cchData)

#define __crtGetTimeFormatEx(lpLocaleName, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime) \
             GetTimeFormatEx(lpLocaleName, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime)

#define __crtGetUserDefaultLocaleName(lpLocaleName, cchLocaleName) \
             GetUserDefaultLocaleName(lpLocaleName, cchLocaleName)

#define __crtIsValidLocaleName(lpLocaleName) \
             IsValidLocaleName(lpLocaleName)

#define __crtLCMapStringEx(lpLocaleName, dwMapFlags, lpSrcStr, cchSrc, lpDestStr, cchDest) \
        LCMapStringEx(lpLocaleName, dwMapFlags, lpSrcStr, cchSrc, lpDestStr, cchDest, NULL, NULL, 0)

#define __crtGetFileInformationByHandleEx(hFile, FileInformationClass, lpFileInformation, dwBufferSize) \
             GetFileInformationByHandleEx(hFile, FileInformationClass, lpFileInformation, dwBufferSize)

#define __crtSetFileInformationByHandle(hFile, FileInformationClass, lpFileInformation, dwBufferSize) \
             SetFileInformationByHandle(hFile, FileInformationClass, lpFileInformation, dwBufferSize)

#define __crtGetTickCount64() \
             GetTickCount64()

#else  /* _CRT_NTDDI_MIN >= NTDDI_VISTA  */

/* Downlevel NLS locale name <==> LCID conversion */
LCID __cdecl __crtDownlevelLocaleNameToLCID(
  LPCWSTR localeName);

int __cdecl __crtDownlevelLCIDToLocaleName(
  LCID lcid,
  LPWSTR outLocaleName,
  int cchLocaleName);

_CRTIMP DWORD __cdecl __crtFlsAlloc(
  __in  PFLS_CALLBACK_FUNCTION lpCallback);

_CRTIMP BOOL __cdecl __crtFlsFree(
  __in  DWORD dwFlsIndex);

_CRTIMP PVOID __cdecl __crtFlsGetValue(
  __in  DWORD dwFlsIndex);

_CRTIMP BOOL __cdecl __crtFlsSetValue(
  __in      DWORD dwFlsIndex,
  __in_opt  PVOID lpFlsData);

// wrapper for InitializeCriticalSectionEx, available only on Vista+
_CRTIMP BOOL __cdecl __crtInitializeCriticalSectionEx(
  __out LPCRITICAL_SECTION lpCriticalSection,
  __in  DWORD dwSpinCount,
  __in  DWORD Flags);

// wrapper for CreateEventExW, available only on Vista+
_CRTIMP HANDLE __cdecl __crtCreateEventExW(
  __in_opt    LPSECURITY_ATTRIBUTES lpEventAttributes,
  __in_opt    LPCWSTR lpName,
  __reserved  DWORD dwFlags,
  __in        DWORD dwDesiredAccess);

// wrapper for CreateSemaphoreExW, available only on Vista+
_CRTIMP HANDLE __cdecl __crtCreateSemaphoreExW(
  __in_opt    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
  __in        LONG lInitialCount,
  __in        LONG lMaximumCount,
  __in_opt    LPCWSTR lpName,
  __reserved  DWORD dwFlags,
  __in        DWORD dwDesiredAccess);

// wrapper for SetThreadStackGuarantee, available only on Vista+
_CRTIMP BOOL __cdecl __crtSetThreadStackGuarantee(
  PULONG StackSizeInBytes);

// wrappers for thread pool APIs, available only on Vista+
PTP_TIMER __cdecl __crtCreateThreadpoolTimer(
  __in         PTP_TIMER_CALLBACK pfnti,
  __inout_opt  PVOID pv,
  __in_opt     PTP_CALLBACK_ENVIRON pcbe);

VOID __cdecl __crtSetThreadpoolTimer(
  __inout   PTP_TIMER pti,
  __in_opt  PFILETIME pftDueTime,
  __in      DWORD msPeriod,
  __in_opt  DWORD msWindowLength);

VOID __cdecl __crtWaitForThreadpoolTimerCallbacks(
  __inout  PTP_TIMER pti,
  __in     BOOL fCancelPendingCallbacks);

VOID __cdecl __crtCloseThreadpoolTimer(
  __inout  PTP_TIMER pti);

PTP_WAIT __cdecl __crtCreateThreadpoolWait(
  __in         PTP_WAIT_CALLBACK pfnwa,
  __inout_opt  PVOID pv,
  __in_opt     PTP_CALLBACK_ENVIRON pcbe);

VOID __cdecl __crtSetThreadpoolWait(
  __inout   PTP_WAIT pwa,
  __in_opt  HANDLE h,
  __in_opt  PFILETIME pftTimeout);

VOID __cdecl __crtCloseThreadpoolWait(
  __inout  PTP_WAIT pwa);

// wrapper for FlushProcessWriteBuffers, available only on Vista+
VOID __cdecl __crtFlushProcessWriteBuffers(void);

// wrapper for FreeLibraryWhenCallbackReturns, available only on Vista+
VOID __cdecl __crtFreeLibraryWhenCallbackReturns(
  __inout  PTP_CALLBACK_INSTANCE pci,
  __in     HMODULE mod);

// wrapper for GetCurrentProcessorNumber, available only on Vista+
DWORD __cdecl __crtGetCurrentProcessorNumber(void);

// wrapper for GetLogicalProcessorInformation, available only on Vista+
BOOL __cdecl __crtGetLogicalProcessorInformation(
  __out    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer,
  __inout  PDWORD ReturnLength);

// wrapper for CreateSymbolicLink, available only on Vista+
_CRTIMP BOOLEAN __cdecl __crtCreateSymbolicLinkW(
  __in  LPCWSTR lpSymlinkFileName,
  __in  LPCWSTR lpTargetFileName,
  __in  DWORD dwFlags);

/* Helper functions for NLS-specific Win32 APIs */
_CRTIMP int __cdecl __crtCompareStringEx(
  __in_opt  LPCWSTR lpLocaleName,
  __in      DWORD dwCmpFlags,
  __in      LPCWSTR lpString1,
  __in      int cchCount1,
  __in      LPCWSTR lpString2,
  __in      int cchCount2);

_CRTIMP BOOL __cdecl __crtEnumSystemLocalesEx(
  __in  LOCALE_ENUMPROCEX lpLocaleEnumProcEx,
  __in  DWORD dwFlags,
  __in  LPARAM lParam);

_CRTIMP int __cdecl __crtGetDateFormatEx(
  __in_opt   LPCWSTR lpLocaleName,
  __in       DWORD dwFlags,
  __in_opt   const SYSTEMTIME *lpDate,
  __in_opt   LPCWSTR lpFormat,
  __out_opt  LPWSTR lpDateStr,
  __in       int cchDate);

_CRTIMP int  __cdecl __crtGetLocaleInfoEx(
  __in_opt   LPCWSTR lpLocaleName,
  __in       LCTYPE LCType,
  __out_opt  LPWSTR lpLCData,
  __in       int cchData);

_CRTIMP int  __cdecl __crtGetTimeFormatEx(
  __in_opt   LPCWSTR lpLocaleName,
  __in       DWORD dwFlags,
  __in_opt   const SYSTEMTIME *lpTime,
  __in_opt   LPCWSTR lpFormat,
  __out_opt  LPWSTR lpTimeStr,
  __in       int cchTime);

_CRTIMP int  __cdecl __crtGetUserDefaultLocaleName(
  __out  LPWSTR lpLocaleName,
  __in   int cchLocaleName);

_CRTIMP BOOL __cdecl __crtIsValidLocaleName(
     __in  LPCWSTR lpLocaleName);

_CRTIMP int __cdecl __crtLCMapStringEx(
  __in_opt   LPCWSTR lpLocaleName,
  __in       DWORD dwMapFlags,
  __in       LPCWSTR lpSrcStr,
  __in       int cchSrc,
  __out_opt  LPWSTR lpDestStr,
  __in       int cchDest);

// wrapper for GetFileInformationByHandleEx, available only on Vista+
_CRTIMP BOOL __cdecl __crtGetFileInformationByHandleEx(
  _In_   HANDLE hFile,
  _In_   FILE_INFO_BY_HANDLE_CLASS FileInformationClass,
  _Out_  LPVOID lpFileInformation,
  _In_   DWORD dwBufferSize);

// wrapper for SetFileInformationByHandle, available only on Vista+
_CRTIMP BOOL __cdecl __crtSetFileInformationByHandle(
  _In_  HANDLE hFile,
  _In_  FILE_INFO_BY_HANDLE_CLASS FileInformationClass,
  _In_  LPVOID lpFileInformation,
  _In_  DWORD dwBufferSize);

// wrapper function for GetTickCount64, available only on Vista+
_CRTIMP ULONGLONG __cdecl __crtGetTickCount64();

#endif  /* _CRT_NTDDI_MIN >= NTDDI_VISTA  */

/* Helper function for Sleep */
_CRTIMP void __cdecl __crtSleep(_In_ DWORD dwMilliseconds);

// Helper to load all necessary Win32 API function pointers
void __cdecl __crtLoadWinApiPointers(void);

enum wrapEncodedKERNEL32Functions
{
    eFlsAlloc = 0,
    eFlsFree,
    eFlsGetValue,
    eFlsSetValue,
    eInitializeCriticalSectionEx,
    eCreateEventExW,
    eCreateSemaphoreExW,
    eSetThreadStackGuarantee,
    eCreateThreadpoolTimer,
    eSetThreadpoolTimer,
    eWaitForThreadpoolTimerCallbacks,
    eCloseThreadpoolTimer,
    eCreateThreadpoolWait,
    eSetThreadpoolWait,
    eCloseThreadpoolWait,
    eFlushProcessWriteBuffers,
    eFreeLibraryWhenCallbackReturns,
    eGetCurrentProcessorNumber,
    eGetLogicalProcessorInformation,
    eCreateSymbolicLinkW,
    eSetDefaultDllDirectories,
    eCompareStringEx,
    eEnumSystemLocalesEx,
    eGetDateFormatEx,
    eGetLocaleInfoEx,
    eGetTimeFormatEx,
    eGetUserDefaultLocaleName,
    eIsValidLocaleName,
    eLCMapStringEx,
    eGetCurrentPackageId,
    eGetTickCount64,
    eGetFileInformationByHandleExW,
    eSetFileInformationByHandleW,
    eMaxKernel32Function
};

extern PVOID encodedKERNEL32Functions[eMaxKernel32Function];

/* Needed for pointer encoding */
extern UINT_PTR __security_cookie;

typedef DWORD (WINAPI *PFNFLSALLOC)(PFLS_CALLBACK_FUNCTION);
typedef BOOL (WINAPI *PFNFLSFREE)(DWORD);
typedef PVOID (WINAPI *PFNFLSGETVALUE)(DWORD);
typedef BOOL (WINAPI *PFNFLSSETVALUE)(DWORD, PVOID);
typedef BOOL (WINAPI *PFNINITIALIZECRITICALSECTIONEX)(LPCRITICAL_SECTION, DWORD, DWORD);
typedef HANDLE (WINAPI *PFNCREATEEVENTEXW)(LPSECURITY_ATTRIBUTES, LPCWSTR, DWORD, DWORD);
typedef HANDLE (WINAPI *PFNCREATESEMAPHOREEXW)(LPSECURITY_ATTRIBUTES, LONG, LONG, LPCWSTR, DWORD, DWORD);
typedef BOOL (WINAPI *PFNSETTHREADSTACKGUARANTEE)(PULONG);
typedef PTP_TIMER (WINAPI *PFNCREATETHREADPOOLTIMER)(PTP_TIMER_CALLBACK, PVOID, PTP_CALLBACK_ENVIRON);
typedef VOID (WINAPI *PFNSETTHREADPOOLTIMER)(PTP_TIMER, PFILETIME, DWORD, DWORD);
typedef VOID (WINAPI *PFNWAITFORTHREADPOOLTIMERCALLBACKS)(PTP_TIMER, BOOL);
typedef VOID (WINAPI *PFNCLOSETHREADPOOLTIMER)(PTP_TIMER);
typedef PTP_WAIT (WINAPI *PFNCREATETHREADPOOLWAIT)(PTP_WAIT_CALLBACK, PVOID, PTP_CALLBACK_ENVIRON);
typedef VOID (WINAPI *PFNSETTHREADPOOLWAIT)(PTP_WAIT, HANDLE, PFILETIME);
typedef VOID (WINAPI *PFNCLOSETHREADPOOLWAIT)(PTP_WAIT);
typedef VOID (WINAPI *PFNFLUSHPROCESSWRITEBUFFERS)(void);
typedef VOID (WINAPI *PFNFREELIBRARYWHENCALLBACKRETURNS)(PTP_CALLBACK_INSTANCE, HMODULE);
typedef DWORD (WINAPI *PFNGETCURRENTPROCESSORNUMBER)(void);
typedef BOOL (WINAPI *PFNGETLOGICALPROCESSORINFORMATION)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);
typedef BOOLEAN (WINAPI *PFNCREATESYMBOLICLINK)(LPCWSTR, LPCWSTR, DWORD);
typedef BOOL (WINAPI *PFNENUMSYSTEMLOCALESEX)(LOCALE_ENUMPROCEX, DWORD, LPARAM, LPVOID);
typedef int (WINAPI *PFNCOMPARESTRINGEX)(LPCWSTR, DWORD, LPCWSTR, int, LPCWSTR, int, LPNLSVERSIONINFO, LPVOID, LPARAM);
typedef int (WINAPI *PFNGETDATEFORMATEX)(LPCWSTR, DWORD, const SYSTEMTIME *, LPCWSTR, LPWSTR, int, LPCWSTR);
typedef int (WINAPI *PFNGETLOCALEINFOEX)(LPCWSTR, LCTYPE, LPWSTR, int);
typedef int (WINAPI *PFNGETTIMEFORMATEX)(LPCWSTR, DWORD, const SYSTEMTIME *, LPCWSTR, LPWSTR, int);
typedef int (WINAPI *PFNGETUSERDEFAULTLOCALENAME)(LPWSTR, int);
typedef BOOL (WINAPI *PFNISVALIDLOCALENAME)(LPCWSTR);
typedef int (WINAPI *PFNLCMAPSTRINGEX)(LPCWSTR, DWORD, LPCWSTR, int, LPWSTR, int, LPNLSVERSIONINFO, LPVOID, LPARAM);
typedef ULONGLONG (WINAPI *PFNGETTICKCOUNT64)(void);
typedef BOOL (WINAPI *PFNGETFILEINFORMATIONBYHANDLEEX)(HANDLE, FILE_INFO_BY_HANDLE_CLASS, LPVOID, DWORD);
typedef BOOL (WINAPI *PFNSETFILEINFORMATIONBYHANDLE)(HANDLE, FILE_INFO_BY_HANDLE_CLASS, LPVOID, DWORD);

// Use this macro for encoding and caching a function pointer from a DLL
#define STOREENCODEDFUNCTIONPOINTER(instance, libraryname, functionname) \
    encoded##libraryname##Functions[e##functionname] = (PVOID)(((UINT_PTR)GetProcAddress(instance, #functionname)) ^ __security_cookie);

// Use this macro for decoding a cached function pointer from a DLL
#define IFDYNAMICGETCACHEDFUNCTION(libraryname, functiontypedef, functionname, functionpointer) \
    functiontypedef functionpointer = (functiontypedef)(((UINT_PTR)encoded##libraryname##Functions[e##functionname]) ^ __security_cookie); \
    if (functionpointer != NULL)


#ifdef _CRT_APP
/*
 * Helper functions to work with WinRT threadpool threads
 */
HANDLE __cdecl __crtCreateWinRTThread(
    _In_       LPTHREAD_START_ROUTINE lpStartAddress,
    _In_opt_   LPVOID lpParameter,
    __out_opt  LPDWORD lpThreadId
);

BOOL   __cdecl __crtCloseWinRTThreadHandle(_In_opt_ HANDLE hThread);
HANDLE __cdecl __crtGetCurrentWinRTThread(void);
DWORD  __cdecl __crtGetCurrentWinRTThreadId(void);
BOOL   __cdecl __crtGetExitCodeWinRTThread(_In_ HANDLE hThread, _Out_ DWORD *pdwExitCode);
HANDLE __cdecl __crtGetWinRTThreadWaitableHandle(_In_ HANDLE hThread);
DWORD  __cdecl __crtGetWinRTThreadId(_In_ HANDLE hThread);
BOOL   __cdecl __crtSetExitCodeWinRTThread(_In_ HANDLE hThread, _In_ DWORD wExitCode);
DWORD  __cdecl __crtWaitForWinRTThreadExit(_In_ HANDLE hThread, _In_ DWORD timeout);

/*
 * Helper functions to work with WinRT threadpool timers
 */

/// <summary>
/// Create a timer
/// </summary>
/// <param name="lpStartAddress">
///     [in] Pointer to the user function.
/// </param>
/// <param name="lpParameter">
///     [in] Pointer to the user parameter.
/// </param>
/// <param name="duration">
///     [in] The delay or period for the timer.
/// </param>
/// <param name="periodic">
///     [in] Indicates that the timer to be created is periodic.
/// </param>
/// <returns>
///     A Handle to the timer
/// </returns>
HANDLE __cdecl __crtCreateWinRTTimer(
    _In_ WAITORTIMERCALLBACK  lpStartAddress,
    _In_opt_ PVOID lpParameter,
    _In_ unsigned int duration,
    _In_ BOOL periodic
);

/// <summary>
/// Deletes the given timer
/// </summary>
/// <param name="hTimer">
///     [in] Handle to the timer.
/// </param>
/// <param name="waitForHandlers">
///     [in] Indicates that the routine should wait for all outstanding callbacks to complete
/// </param>
void __cdecl __crtDeleteWinRTTimer(_In_opt_ HANDLE hTimer, _In_ BOOL waitForHandlers);

/// <summary>
/// Cancels the given timer
/// </summary>
/// <param name="hTimer">
///     [in] Handle to the timer.
/// </param>
/// <param name="waitForHandlers">
///     [in] Indicates that the routine should wait for all outstanding callbacks to complete
/// </param>
void __cdecl __crtCancelWinRTTimer(_In_opt_ HANDLE hTimer, _In_ BOOL waitForHandlers);


void __cdecl __crtExitProcessWinRT(void);
DWORD __cdecl __crtGetFullPathNameWinRTW(const wchar_t* fileName, DWORD bufferLength, wchar_t* buffer);
DWORD __cdecl __crtGetFullPathNameWinRTA(const char* fileName, DWORD bufferLength, char* buffer);
DWORD __cdecl __crtGetTempPathWinRTW(unsigned int maxBuff, wchar_t* tempPathBuffer);
DWORD __cdecl __crtGetTempPathWinRTA(unsigned int maxBuff, char* tempPathBuffer);

#endif  /* _CRT_APP */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* _INC_AWINC */

#endif  /* _WIN32 */
