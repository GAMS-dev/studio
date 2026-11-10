/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "crashhandler.h"
#include "commonpaths.h"

#include <QDebug>
#include <QDir>

#ifdef Q_OS_WIN
#  include <windows.h>
#  include <dbghelp.h>
#  include <csignal>
#  include <exception>
#endif

namespace {

QString gDumpDirectory;
bool gInstalled = false;

#ifdef Q_OS_WIN

constexpr DWORD kCodeTerminate        = 0xE0000001;
constexpr DWORD kCodeAbort            = 0xE0000002;
constexpr DWORD kCodePureCall         = 0xE0000003;
constexpr DWORD kCodeInvalidParameter = 0xE0000004;
constexpr DWORD kCodeQtFatal          = 0xE0000005;

wchar_t gDumpDir[MAX_PATH] = L".";
wchar_t gPrefix[32] = L"crash";
HANDLE gRequestEvent = nullptr;
HANDLE gDoneEvent = nullptr;
EXCEPTION_POINTERS* gExceptionPointers = nullptr;
DWORD gThreadId = 0;
BOOL gLastResult = FALSE;
volatile LONG gBusy = 0;
volatile LONG gFatal = 0;
QtMessageHandler gPreviousMessageHandler = nullptr;

// Runs on its own stack, so dumps work even after a stack overflow.
DWORD WINAPI writerThread(LPVOID)
{
    for (;;) {
        WaitForSingleObject(gRequestEvent, INFINITE);

        SYSTEMTIME t;
        GetLocalTime(&t);
        wchar_t path[MAX_PATH + 64];
        swprintf_s(path, L"%ls\\%ls_%04u%02u%02u_%02u%02u%02u_%03u.dmp", gDumpDir, gPrefix,
                   t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);

        gLastResult = FALSE;
        HANDLE file = CreateFileW(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file != INVALID_HANDLE_VALUE) {
            MINIDUMP_EXCEPTION_INFORMATION mei{ gThreadId, gExceptionPointers, FALSE };
            const auto type = MINIDUMP_TYPE(MiniDumpWithThreadInfo | MiniDumpWithIndirectlyReferencedMemory);
            gLastResult = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, type,
                                             gExceptionPointers ? &mei : nullptr, nullptr, nullptr);
            CloseHandle(file);
        }
        SetEvent(gDoneEvent);
    }
}

bool requestDump(EXCEPTION_POINTERS* ep, const wchar_t* prefix)
{
    wcsncpy_s(gPrefix, prefix, _TRUNCATE);
    gExceptionPointers = ep;
    gThreadId = GetCurrentThreadId();
    SetEvent(gRequestEvent);
    return WaitForSingleObject(gDoneEvent, 60 * 1000) == WAIT_OBJECT_0 && gLastResult;
}

// Called on the crashing thread: no heap, no Qt.
void writeFatalDump(EXCEPTION_POINTERS* ep)
{
    if (InterlockedExchange(&gFatal, 1) != 0)
        Sleep(INFINITE);
    while (InterlockedCompareExchange(&gBusy, 1, 0) != 0)
        Sleep(10);
    requestDump(ep, L"crash");
}

// Builds exception info from the current context so .ecxr / !analyze work.
Q_NEVER_INLINE void dumpAndExit(DWORD code)
{
    CONTEXT context{};
    RtlCaptureContext(&context);

    EXCEPTION_RECORD record{};
    record.ExceptionCode = code;
    record.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
#if defined(_M_X64)
    record.ExceptionAddress = reinterpret_cast<PVOID>(context.Rip);
#elif defined(_M_ARM64)
    record.ExceptionAddress = reinterpret_cast<PVOID>(context.Pc);
#elif defined(_M_IX86)
    record.ExceptionAddress = reinterpret_cast<PVOID>(context.Eip);
#endif

    EXCEPTION_POINTERS pointers{ &record, &context };
    writeFatalDump(&pointers);
    TerminateProcess(GetCurrentProcess(), code);
}

LONG WINAPI unhandledExceptionFilter(EXCEPTION_POINTERS* ep)
{
    writeFatalDump(ep);
    return EXCEPTION_EXECUTE_HANDLER;
}

void onTerminate() { dumpAndExit(kCodeTerminate); } // per thread; other threads end up in SIGABRT
void onAbortSignal(int) { dumpAndExit(kCodeAbort); }
void onPureCall() { dumpAndExit(kCodePureCall); }
void onInvalidParameter(const wchar_t*, const wchar_t*, const wchar_t*, unsigned, uintptr_t)
{
    dumpAndExit(kCodeInvalidParameter);
}

// qFatal() terminates without SIGABRT on Windows.
void onQtMessage(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    if (gPreviousMessageHandler)
        gPreviousMessageHandler(type, context, message);
    if (type == QtFatalMsg)
        dumpAndExit(kCodeQtFatal);
}

#endif // Q_OS_WIN

} // namespace

namespace gams {
namespace studio {

bool CrashHandler::install(const QString& dumpDirectory)
{
    if (gInstalled)
        return true;

#ifdef Q_OS_WIN
    const QString dir = dumpDirectory.isEmpty()
        ? gams::studio::CommonPaths::studioDocumentsDir() + QStringLiteral("/Dumps")
        : dumpDirectory;
    const QString native = QDir::toNativeSeparators(QDir(dir).absolutePath());
    if (!QDir().mkpath(dir) || native.size() >= MAX_PATH) {
        qWarning() << "CrashHandler: invalid dump directory" << native;
        return false;
    }
    native.toWCharArray(gDumpDir);
    gDumpDir[native.size()] = L'\0';
    gDumpDirectory = native;

    gRequestEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    gDoneEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    HANDLE thread = CreateThread(nullptr, 0, writerThread, nullptr, 0, nullptr);
    if (!gRequestEvent || !gDoneEvent || !thread) {
        qWarning() << "CrashHandler: cannot create writer thread";
        return false;
    }
    CloseHandle(thread);

    ULONG stackGuarantee = 64 * 1024;
    SetThreadStackGuarantee(&stackGuarantee);

    SetUnhandledExceptionFilter(unhandledExceptionFilter);
    std::set_terminate(onTerminate);
    std::signal(SIGABRT, onAbortSignal);
    _set_purecall_handler(onPureCall);
    _set_invalid_parameter_handler(onInvalidParameter);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    gPreviousMessageHandler = qInstallMessageHandler(onQtMessage);
#else
    Q_UNUSED(dumpDirectory);
#  if defined(Q_OS_MACOS)
    gDumpDirectory = QDir::homePath() + QStringLiteral("/Library/Logs/DiagnosticReports");
#  elif defined(Q_OS_LINUX)
    gDumpDirectory = QStringLiteral("/var/lib/systemd/coredump");
#  endif
#endif

    gInstalled = true;
    return true;
}

bool CrashHandler::isInstalled()
{
    return gInstalled;
}

QString CrashHandler::dumpDirectory()
{
    return gDumpDirectory;
}

bool CrashHandler::writeDump(const QString& namePrefix)
{
#ifdef Q_OS_WIN
    if (!gInstalled || gFatal || InterlockedCompareExchange(&gBusy, 1, 0) != 0)
        return false;

    wchar_t prefix[32] = {};
    namePrefix.left(31).toWCharArray(prefix);
    const bool ok = requestDump(nullptr, prefix[0] ? prefix : L"dump");
    InterlockedExchange(&gBusy, 0);
    return ok;
#else
    Q_UNUSED(namePrefix);
    return false;
#endif
}

}
}
