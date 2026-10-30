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
#include <QApplication>
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <QList>
#include <QMutex>
#include <QDir>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <exception>
#include <stdexcept>
#include <thread>

#ifdef Q_OS_WIN
#  include <windows.h>
#  include <dbghelp.h>
#  include <intrin.h>
#endif

// ===========================================================================
// Crash-Handler (Windows; Linux/macOS: Core-Dump or ReportCrash vom OS)
// ===========================================================================
#ifdef Q_OS_WIN
namespace CrashDump {

wchar_t             g_dumpDir[MAX_PATH] = L".";
HANDLE              g_requestEvent = nullptr;
HANDLE              g_doneEvent    = nullptr;
EXCEPTION_POINTERS* g_exceptionPointers = nullptr;
DWORD               g_crashThreadId = 0;
volatile LONG       g_inProgress = 0;
QtMessageHandler    g_previousMessageHandler = nullptr;

// Own thread with own sane stack: also works with Stack Overflow.
DWORD WINAPI writerThread(LPVOID)
{
    WaitForSingleObject(g_requestEvent, INFINITE);

    SYSTEMTIME t;
    GetLocalTime(&t);
    wchar_t path[MAX_PATH + 64];
    swprintf_s(path, L"%ls\\crash_%04u%02u%02u_%02u%02u%02u.dmp", g_dumpDir,
               t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);

    HANDLE file = CreateFileW(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mei{ g_crashThreadId, g_exceptionPointers, FALSE };
        const auto type = MINIDUMP_TYPE(MiniDumpWithThreadInfo
                                        | MiniDumpWithIndirectlyReferencedMemory);
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, type,
                          g_exceptionPointers ? &mei : nullptr, nullptr, nullptr);
        CloseHandle(file);
    }
    SetEvent(g_doneEvent);
    return 0;
}

// Is called from the crashing thread: doesn't need much stack, no Heap, no Qt.
void writeDump(EXCEPTION_POINTERS* ep)
{
    if (InterlockedExchange(&g_inProgress, 1) != 0)
        Sleep(INFINITE);

    g_exceptionPointers = ep;
    g_crashThreadId = GetCurrentThreadId();
    SetEvent(g_requestEvent);
    WaitForSingleObject(g_doneEvent, 60 * 1000);
}

// Own Exception-Codes, for !analyze
constexpr DWORD kCodeTerminate        = 0xE0000001;   // std::terminate
constexpr DWORD kCodeAbort            = 0xE0000002;   // abort() / SIGABRT
constexpr DWORD kCodePureCall         = 0xE0000003;   // pure virtual call
constexpr DWORD kCodeInvalidParameter = 0xE0000004;   // CRT invalid parameter
constexpr DWORD kCodeQtFatal          = 0xE0000005;   // qFatal()

// For cases without real Exception (terminate, abort, purecall, ...)
// get register kontext and write it as exception info to the Dump.
// For .ecxr and !analyze in WinDbg.
Q_NEVER_INLINE void dumpAndExit(DWORD exceptionCode)
{
    CONTEXT context{};
    RtlCaptureContext(&context);

    EXCEPTION_RECORD record{};
    record.ExceptionCode  = exceptionCode;
    record.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
#if defined(_M_X64)
    record.ExceptionAddress = reinterpret_cast<PVOID>(context.Rip);
#elif defined(_M_ARM64)
    record.ExceptionAddress = reinterpret_cast<PVOID>(context.Pc);
#elif defined(_M_IX86)
    record.ExceptionAddress = reinterpret_cast<PVOID>(context.Eip);
#endif

    EXCEPTION_POINTERS pointers{ &record, &context };
    writeDump(&pointers);
    TerminateProcess(GetCurrentProcess(), exceptionCode);
}

LONG WINAPI unhandledExceptionFilter(EXCEPTION_POINTERS* ep)
{
    writeDump(ep);
    return EXCEPTION_EXECUTE_HANDLER;    // EXCEPTION_CONTINUE_SEARCH: additional WER-Dump
}

void onTerminate()      { dumpAndExit(kCodeTerminate); }   // per Thread! Other Threads land via abort() in SIGABRT
void onAbortSignal(int) { dumpAndExit(kCodeAbort); }
void onPureCall()       { dumpAndExit(kCodePureCall); }
void onInvalidParameter(const wchar_t*, const wchar_t*, const wchar_t*, unsigned, uintptr_t)
{
    dumpAndExit(kCodeInvalidParameter);
}

// qFatal() ends the prozess on Windows without SIGABRT/Exception,
// deshalb hier abfangen.
void onQtMessage(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    if (g_previousMessageHandler)
        g_previousMessageHandler(type, context, message);
    if (type == QtFatalMsg)
        dumpAndExit(kCodeQtFatal);
}

QString install()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
                        + QStringLiteral("/CrashDumps");
    QDir().mkpath(dir);
    const QString native = QDir::toNativeSeparators(dir);
    if (native.size() < MAX_PATH) {
        native.toWCharArray(g_dumpDir);
        g_dumpDir[native.size()] = L'\0';
    }

    g_requestEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    g_doneEvent    = CreateEventW(nullptr, TRUE,  FALSE, nullptr);
    if (HANDLE thread = CreateThread(nullptr, 0, writerThread, nullptr, 0, nullptr))
        CloseHandle(thread);

    ULONG stackGuarantee = 64 * 1024;
    SetThreadStackGuarantee(&stackGuarantee);

    SetUnhandledExceptionFilter(unhandledExceptionFilter);
    std::set_terminate(onTerminate);
    std::signal(SIGABRT, onAbortSignal);
    _set_purecall_handler(onPureCall);
    _set_invalid_parameter_handler(onInvalidParameter);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    g_previousMessageHandler = qInstallMessageHandler(onQtMessage);

    return native;
}

} // namespace CrashDump
#endif

// ===========================================================================
// Crash-Funktionen
// Q_NEVER_INLINE: each funktion stays an own frame in the stacktrace.
// volatile:       prevents remove by the optimizer.
// ===========================================================================

Q_NEVER_INLINE void nullPointer()
{
    int* volatile ptr = nullptr;
    *ptr = 42;
}

Q_NEVER_INLINE void divideByZero()
{
    // On ARM64 (Apple Silicon, Windows on ARM) int division by 0 doesn't result in an exception
    volatile int divisor = 0;
    volatile int result = 1 / divisor;
    (void)result;
}

Q_NEVER_INLINE void invalidAddress()
{
    char* volatile ptr = reinterpret_cast<char*>(0xCCCCCCCC);
    *ptr = 'X';
}

Q_NEVER_INLINE int recurse(int depth)
{
    volatile char buffer[1024];
    buffer[0] = static_cast<char>(depth);
    return recurse(depth + 1) + buffer[0];   // no end rekursion -> doesn't turn to loop
}

Q_NEVER_INLINE void stackOverflow()
{
    recurse(0);
}

Q_NEVER_INLINE void uncaughtException()
{
    throw std::runtime_error("Uncaught exception");
}

Q_NEVER_INLINE void throwInNoexcept() noexcept
{
    throw std::runtime_error("Exception in noexcept function");   // -> std::terminate
}

Q_NEVER_INLINE void callAbort()
{
    std::abort();
}

Q_NEVER_INLINE void callQFatal()
{
    qFatal("qFatal test");
}

struct PureBase
{
    PureBase() { callPure(); }
    virtual ~PureBase() = default;
    Q_NEVER_INLINE void callPure() { doIt(); }
    virtual void doIt() = 0;
};

struct PureDerived : PureBase
{
    void doIt() override {}
};

Q_NEVER_INLINE void pureVirtualCall()
{
    PureDerived object;
    (void)object;
}

Q_NEVER_INLINE void doubleDelete()
{
    // Heap-Corruption: Windows mostly ends the prozess with Fail-Fast (0xC0000374),
    // glibc with "double free detected" + abort(). Detection not granted
    int* volatile ptr = new int(42);
    delete ptr;
    delete ptr;
}

Q_NEVER_INLINE void qlistOutOfRange()
{
    // Far Index hits unmapped memory
    QList<int> list{1};
    volatile qsizetype index = 100'000'000;
    qDebug() << list[index];
}

Q_NEVER_INLINE void crashInWorkerThread()
{
    std::thread worker([] { nullPointer(); });
    worker.join();
}

Q_NEVER_INLINE void infiniteLoop()
{
    volatile bool running = true;
    while (running) {}
}

Q_NEVER_INLINE void selfDeadlock()
{
    static QMutex mutex;
    mutex.lock();
    mutex.lock();   // non-rekursive Mutex -> blocks forever
}

#ifdef Q_OS_WIN
Q_NEVER_INLINE void invalidParameter()
{
    char buffer[4];
    const char* volatile source = "definitely too long";
    strcpy_s(buffer, sizeof(buffer), source);
}

Q_NEVER_INLINE void fastFail()
{
    // like a Buffer Overrun (/GS): passes ALL In-Process-Handler
    // Dump only for WER LocalDumps
    __fastfail(FAST_FAIL_FATAL_APP_EXIT);
}
#endif

// ===========================================================================
// Testtable
// ===========================================================================
struct CrashTest
{
    const char* id;
    const char* label;
    void (*func)();
    const char* note;
};

const CrashTest crashTests[] = {
    // id              label                                    func                 note
    { "null",         "Null pointer dereference",              nullPointer,         "OK: dump & source null ptr" },
    { "div0",         "Integer divide by zero",                divideByZero,        "OK: dump & source div /0" },
    { "segfault",     "Write to invalid address",              invalidAddress,      "OK: dump & source address" },
    { "stack",        "Stack overflow",                        stackOverflow,       "OK: dump & source recurse" },
    { "exception",    "Uncaught C++ exception",                uncaughtException,   "OK: dump & source uncaught" },
    { "noexcept",     "Throw in noexcept (std::terminate)",    throwInNoexcept,     "OK: dump & source dumpAndExit?" },
    { "abort",        "std::abort()",                          callAbort,           "OK: dump & source dumpAndExit?" },
    { "qfatal",       "qFatal()",                              callQFatal,          "OK: dump & source dumpAndExit?" },
    { "purecall",     "Pure virtual call",                     pureVirtualCall,     "OK: dump & source dumpAndExit?" },
    { "doubledelete", "Heap corruption (double delete)",       doubleDelete,        "NO dump" },
    { "qlist",        "QList index far out of range",          qlistOutOfRange,     "OK: dump & source index range" },
    { "thread",       "Crash in worker thread",                crashInWorkerThread, "OK: dump & source null ptr" },
#ifdef Q_OS_WIN
    { "invalidparam", "CRT invalid parameter (strcpy_s)",      invalidParameter,    "OK: dump & source dumpAndExit?" },
    { "fastfail",     "__fastfail (only WER LocalDumps)",      fastFail,            "NO dump (expected)" },
#endif
    { "hang",         "Hang: endless loop (watchdog test)",    infiniteLoop,        "" },
    { "deadlock",     "Hang: deadlock (watchdog test)",        selfDeadlock,        "" },
    };

// ===========================================================================
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Crash-Reports folder
    QString dumpDir;
#if defined(Q_OS_WIN)
    dumpDir = CrashDump::install();
#elif defined(Q_OS_MACOS)
    dumpDir = QDir::homePath() + QStringLiteral("/Library/Logs/DiagnosticReports");
#elif defined(Q_OS_LINUX)
    dumpDir = QStringLiteral("/var/lib/systemd/coredump");   // nur mit systemd-coredump
#endif

    // Command line mode
    if (argc > 1) {
        const QString id = QString::fromLocal8Bit(argv[1]);
        for (const CrashTest& test : crashTests) {
            if (id == QLatin1String(test.id)) {
                test.func();
                return 0;
            }
        }
        qWarning() << "Unknown crash type:" << id << "- available:";
        for (const CrashTest& test : crashTests)
            qWarning().noquote() << "  " << test.id << "-" << test.label;
        return 1;
    }

    // GUI-Mode (no try/catch, to keep the crash point)
    QWidget window;
    window.setWindowTitle(QStringLiteral("Qt Crash Demo"));
    auto* layout = new QGridLayout(&window);
    auto* pathLabel = new QLabel;
    if (!dumpDir.isEmpty() && QDir(dumpDir).exists()) {
        const QUrl url = QUrl::fromLocalFile(QDir::fromNativeSeparators(dumpDir));
        pathLabel->setTextFormat(Qt::RichText);
        pathLabel->setText(QStringLiteral("Crash dumps: <a href=\"%1\">%2</a>")
                               .arg(url.toString(), dumpDir.toHtmlEscaped()));
        pathLabel->setToolTip(QStringLiteral("Open folder"));
        QObject::connect(pathLabel, &QLabel::linkActivated, [](const QString& link) {
            QDesktopServices::openUrl(QUrl(link));
        });
    } else {
        pathLabel->setText(QStringLiteral("Crash dumps: core dump / crash report of the OS"));
    }
    layout->addWidget(pathLabel, 0, 0, 1, 2);
    layout->setColumnStretch(1, 1);
    window.setMinimumWidth(500);

    int row = 1;
    for (const CrashTest& test : crashTests) {
        auto* button = new QPushButton(QString::fromLatin1(test.label));
        QObject::connect(button, &QPushButton::clicked, test.func);

        auto* note = new QLabel(QString::fromUtf8(test.note));
        note->setWordWrap(true);
        note->setTextInteractionFlags(Qt::TextSelectableByMouse);

        layout->addWidget(button, row, 0);
        layout->addWidget(note, row, 1);
        ++row;
    }

    window.show();
    return app.exec();
}
