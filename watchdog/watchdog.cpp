#include "watchdog.h"
#include "reportdialog.h"
#include <QApplication>
#include <QFileInfo>
#include <chrono>

#ifdef _WIN64
#include "checksystem_win.h"
#elif _APPLE_
#include "checksystem_mac.h"
#else
#include "checksystem_unix.h"
#endif


// ---------------- main ----------------

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (argc < 2) {
        QMessageBox::critical(nullptr, "Usage Error", "Usage: watchdog <program> [args...]");
        return 1;
    }

    QString programPath = argv[1];
    QStringList programArgs;
    programArgs.reserve(argc);
    for (int i = 2; i < argc; ++i)
        programArgs << argv[i];

    gams::studio::Watchdog watchdog(programPath, programArgs);

    return a.exec();
}


// ------------- watchdog ---------------

using namespace std::chrono;

namespace gams {
namespace studio {

Watchdog::Watchdog(const QString &programPath, const QStringList &arguments, QObject *parent)
    : QObject(parent), mProgramPath(programPath), mArguments(arguments), mProcess(new QProcess(this))
{

    connect(mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Watchdog::onProcessFinished);
    startStudio();

    mTimer.setSingleShot(true);
    connect(&mTimer, &QTimer::timeout, this, &Watchdog::checkProcess);
    mTimer.start(3s);
}

Watchdog::~Watchdog()
{
    delete mDialog;
}

void Watchdog::startStudio()
{
    qDebug() << "Starting Studio:" << mProgramPath << mArguments;
    QString path = QFileInfo(mProgramPath).absolutePath();
    mProcess->setWorkingDirectory(path);
    mProcess->start(mProgramPath, mArguments);
    // mProcess->startDetached(mProgramPath, mArguments);

    if (!mProcess->waitForStarted(mTimer.interval())) {
        qWarning() << "Failed to start Studio!";
        mState = psFailStart;
    } else if (mState == psInit) {
        mState = psRunning;
    }
}

void Watchdog::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qWarning() << "Studio exited with code:" << exitCode;

    if (exitStatus == QProcess::CrashExit) {
        qWarning() << "Detected crash!";
        showCrashPopup(psCrash);
    } else {
        mState = psNormalExit;
        qInfo() << QString("Studio exited normally with exitCode %1.").arg(exitCode);
        QCoreApplication::quit();  // Terminates the app cleanly
    }
}

void Watchdog::checkProcess()
{
    if (mProcess->state() == QProcess::Running) {
        qWarning() << "Process maybe crashed.";
        showCrashPopup(psMayCrash);
    } else {
        mTimer.start(3s);
    }
}

void Watchdog::showCrashPopup(ProcState state)
{
    DialogKind kind = state >= mState && state == psMayCrash ? dkUnresponsive : dkCrash;
    if (!mDialog) {
        mDialog = new ReportDialog(kind, crashData());
        mState = state;
        mDialog->exec();
        if (mDialog)
            QCoreApplication::quit();

    } else if (mState != state && state == psCrash) {
        mState = state;
        mDialog->setKind(dkCrash);

    } else {
        mDialog->show();
    }
}

QString Watchdog::crashData()
{
    CheckSystem cs(mProcess);
    QTemporaryDir tempDir;
    return cs.CheckSystem::gatherInfo(tempDir);
}

} // namespace studio
} // namespace gams
