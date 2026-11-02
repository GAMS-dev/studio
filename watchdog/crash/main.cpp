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
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QMutex>
#include <QPushButton>
#include <QUrl>

#include <cstdlib>
#include <stdexcept>
#include <thread>

#ifdef Q_OS_WIN
#  include <windows.h>
#  include <intrin.h>
#endif

#include "crashhandler.h"

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

struct CrashTest
{
    const char* id;
    const char* label;
    void (*func)();
    const char* note; // UTF-8
};

const CrashTest crashTests[] = {
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
    { "dump",         "Write dump, keep running",           [] { CrashHandler::writeDump(); }, "" },
    { "hang",         "Hang: endless loop",                 infiniteLoop,        "" },
    { "deadlock",     "Hang: deadlock",                     selfDeadlock,        "" },
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    CrashHandler::install();
    const QString dumpDir = CrashHandler::dumpDirectory();

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

    // No try/catch: the crash location must stay on the stack.
    QWidget window;
    window.setWindowTitle(QStringLiteral("Qt Crash Demo"));
    auto* layout = new QGridLayout(&window);
    layout->setColumnStretch(1, 1);

    auto* pathLabel = new QLabel;
    if (!dumpDir.isEmpty() && QDir(dumpDir).exists()) {
        const QUrl url = QUrl::fromLocalFile(QDir::fromNativeSeparators(dumpDir));
        pathLabel->setText(QStringLiteral("Crash dumps: <a href=\"%1\">%2</a>")
                               .arg(url.toString(), dumpDir.toHtmlEscaped()));
        QObject::connect(pathLabel, &QLabel::linkActivated,
                         [](const QString& link) { QDesktopServices::openUrl(QUrl(link)); });
    } else {
        pathLabel->setText(QStringLiteral("Crash dumps: core dump / crash report of the OS"));
    }
    layout->addWidget(pathLabel, 0, 0, 1, 2);

    int row = 1;
    for (const CrashTest& test : crashTests) {
        auto* button = new QPushButton(QString::fromLatin1(test.label));
        QObject::connect(button, &QPushButton::clicked, test.func);

        auto* note = new QLabel(QString::fromUtf8(test.note));
        note->setWordWrap(true);
        note->setTextInteractionFlags(Qt::TextSelectableByMouse);

        layout->addWidget(button, row, 0);
        layout->addWidget(note, row++, 1);
    }

    window.show();
    return app.exec();
}
