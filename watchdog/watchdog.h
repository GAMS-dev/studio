#include <QApplication>
#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include "reportdialog.h"

namespace gams {
namespace studio {

enum ProcState {
    psInit,
    psFailStart,
    psRunning,
    psNormalExit,
    psMayCrash,
    psCrash
};

class Watchdog : public QObject {
    Q_OBJECT

public:
    Watchdog(const QString &programPath, const QStringList &arguments, QObject *parent = nullptr);
    ~Watchdog() override;

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void checkProcess();
    void startStudio();
    void showCrashPopup(ProcState state);

private:
    QString crashData();

private:
    QString mProgramPath;
    QStringList mArguments;
    QTimer mTimer;
    ReportDialog *mDialog = nullptr;
    QProcess *mProcess = nullptr;
    bool mMaybeDown = false;
    ProcState mState = psInit;
};

}
}

