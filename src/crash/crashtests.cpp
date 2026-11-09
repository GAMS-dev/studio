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
#include "crashtests.h"
#include "crashhandler.h"

#include <QAction>
#include <QDebug>
#include <QList>
#include <QMenu>
#include <QMutex>

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <thread>

#ifdef Q_OS_WIN
#  include <windows.h>
#  include <intrin.h>
#endif

namespace {

// Q_NEVER_INLINE keeps each test as its own stack frame,
// volatile stops the optimizer from removing the crash in release builds.

Q_NEVER_INLINE void nullPointer()
{
    int* volatile ptr = nullptr;
    *ptr = 42;
}

Q_NEVER_INLINE void divideByZero() // no exception on ARM64
{
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
    return recurse(depth + 1) + buffer[0]; // not a tail call
}

Q_NEVER_INLINE void stackOverflow() { recurse(0); }
Q_NEVER_INLINE void uncaughtException() { throw std::runtime_error("Uncaught exception"); }
Q_NEVER_INLINE void throwInNoexcept() noexcept { throw std::runtime_error("Throw in noexcept"); }
Q_NEVER_INLINE void callAbort() { std::abort(); }
Q_NEVER_INLINE void callQFatal() { qFatal("qFatal test"); }

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

Q_NEVER_INLINE void pureVirtualCall() { PureDerived object; }

Q_NEVER_INLINE void doubleDelete() // detection not guaranteed
{
    int* volatile ptr = new int(42);
    delete ptr;
    delete ptr;
}

Q_NEVER_INLINE void qlistOutOfRange() // small indexes don't crash in release
{
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
    mutex.lock();
}

#ifdef Q_OS_WIN
Q_NEVER_INLINE void invalidParameter()
{
    char buffer[4];
    const char* volatile source = "definitely too long";
    strcpy_s(buffer, sizeof(buffer), source);
}

Q_NEVER_INLINE void fastFail() { __fastfail(FAST_FAIL_FATAL_APP_EXIT); } // bypasses all in-process handlers
#endif

struct Test
{
    const char* id;
    const char* label;
    void (*func)();
    const char* note; // UTF-8
};

const Test kTests[] = {
    { "null",         "Null pointer dereference",                           nullPointer,
     "WEI:\t dump created\nDAC:\t todo\nLEG:\t todo" },
    { "div0",         "Integer divide by zero",                             divideByZero,
     "WEI:\t dump created\nDAC:\t todo\nLEG:\t todo" },
    { "segfault",     "Write to invalid address",                           invalidAddress,
     "WEI:\t dump created\nDAC:\t todo\nLEG:\t todo" },
    { "stack",        "Stack overflow",                                     stackOverflow,
     "WEI:\t dump created\nDAC:\t todo\nLEG:\t todo" },
    { "exception",    "Uncaught C++ exception",                             uncaughtException,
     "WEI:\t dump created, reduce dialogs\nDAC:\t todo\nLEG:\t todo" },
    { "noexcept",     "Throw in noexcept (std::terminate)",                 throwInNoexcept,
     "WEI:\t dump created\nDAC:\t todo\nLEG:\t todo" },
    { "abort",        "std::abort()",                                       callAbort,
     "WEI:\t dump created\nDAC:\t todo\nLEG:\t todo" },
    { "qfatal",       "qFatal()",                                           callQFatal,
     "WEI:\t dump created\nDAC:\t todo\nLEG:\t todo" },
    { "purecall",     "Pure virtual call",                                  pureVirtualCall,
     "WEI:\t dump created\nDAC:\t todo\nLEG:\t todo" },
    { "doubledelete", "Heap corruption (double delete, WER LocalDumps)",    doubleDelete,
     "WEI:\t dump NOT created, WER necessary\nDAC:\t todo\nLEG:\t todo" },
    { "qlist",        "QList index far out of range",                       qlistOutOfRange,
     "WEI:\t dump created\nDAC:\t todo\nLEG:\t todo" },
    { "thread",       "Crash in worker thread",                             crashInWorkerThread,
     "WEI:\t dump created\nDAC:\t todo\nLEG:\t todo" },
#ifdef Q_OS_WIN
    { "invalidparam", "CRT invalid parameter (strcpy_s)",                   invalidParameter,
     "WEI:\t dump created" },
    { "fastfail",     "__fastfail (only WER LocalDumps)",                   fastFail,
     "WEI:\t dump NOT created, WER necessary" },
#endif
    { "dump",         "Write dump, keep running",           [] { CrashHandler::writeDump(); },
     "WEI: dump created\nDAC:\t todo\nLEG:\t todo" },
    // { "hang",         "Hang: endless loop",                 infiniteLoop,        "Only with watchdog" },
    // { "deadlock",     "Hang: deadlock",                     selfDeadlock,        "Only with watchdog" },
};

} // namespace

void CrashTests::addToMenu(QMenu* menu)
{
    if (!menu)
        return;

    menu->setToolTipsVisible(true);
    for (const Test& test : kTests) {
        QAction* action = menu->addAction(QString::fromLatin1(test.label));
        const QString note = QString::fromUtf8(test.note);
        if (!note.isEmpty()) {
            action->setToolTip(note);
            action->setStatusTip(note);
        }
        QObject::connect(action, &QAction::triggered, test.func);
    }
}

bool CrashTests::run(const QString& id)
{
    for (const Test& test : kTests) {
        if (id == QLatin1String(test.id)) {
            test.func();
            return true;
        }
    }
    return false;
}

QStringList CrashTests::ids()
{
    QStringList result;
    for (const Test& test : kTests)
        result << QString::fromLatin1(test.id);
    return result;
}
