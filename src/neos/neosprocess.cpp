#include "neosprocess.h"
#include "neosmanager.h"
#include "logger.h"
#include "commonpaths.h"
#include "process/gmsunzipprocess.h"
#include <QStandardPaths>
#include <QDir>
#include <QDialog>

#ifdef _WIN32
#include "Windows.h"
#endif

namespace gams {
namespace studio {
namespace neos {


NeosProcess::NeosProcess(QObject *parent) : AbstractGamsProcess("gams", parent)
{
    disconnect(&mProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(completed(int)));
    connect(&mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &NeosProcess::compileCompleted);

    mManager = new NeosManager(this);
    mManager->setUrl("https://neos-server.org:3333");
    connect(mManager, &NeosManager::sslErrors, this, &NeosProcess::sslErrors);
    connect(mManager, &NeosManager::rePing, this, &NeosProcess::rePing);
    connect(mManager, &NeosManager::reError, this, &NeosProcess::reError);
    connect(mManager, &NeosManager::reKillJob, this, &NeosProcess::reKillJob);
    connect(mManager, &NeosManager::reVersion, this, &NeosProcess::reVersion);
    connect(mManager, &NeosManager::reSubmitJob, this, &NeosProcess::reSubmitJob);
    connect(mManager, &NeosManager::reGetJobInfo, this, &NeosProcess::reGetJobInfo);
    connect(mManager, &NeosManager::reGetJobStatus, this, &NeosProcess::reGetJobStatus);
    connect(mManager, &NeosManager::reGetOutputFile, this, &NeosProcess::reGetOutputFile);
    connect(mManager, &NeosManager::reGetCompletionCode, this, &NeosProcess::reGetCompletionCode);
    connect(mManager, &NeosManager::reGetFinalResultsNonBlocking, this, &NeosProcess::reGetFinalResultsNonBlocking);
    connect(mManager, &NeosManager::reGetIntermediateResultsNonBlocking, this, &NeosProcess::reGetIntermediateResultsNonBlocking);

    mPullTimer.setInterval(1000);
    mPullTimer.setSingleShot(true);
    connect(&mPullTimer, &QTimer::timeout, this, &NeosProcess::pullStatus);
    mManager->ping();
}

NeosProcess::~NeosProcess()
{
    delete mManager;
}

void NeosProcess::execute()
{
    QStringList params = compileParameters();
    mProcess.setWorkingDirectory(workingDirectory());

#if defined(__unix__) || defined(__APPLE__)
    mProcess.start(nativeAppPath(), params);
#else
    mProcess.setNativeArguments(params.join(" "));
    mProcess.setProgram(nativeAppPath());
    mProcess.start();
#endif

    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    setNeosState(Neos1Compile);
}

QStringList NeosProcess::compileParameters()
{
    if (mOutPath.isEmpty()) {
        DEB() << "Error: No runable file assigned to the NEOS process";
        return QStringList();
    }
    QFileInfo fi(mOutPath);
    QDir dir(fi.path());
    dir.mkdir(fi.fileName());
    QStringList params = parameters();
    QMutableListIterator<QString> i(params);
    bool needsXSave = true;
    bool needsActC = true;
    while (i.hasNext()) {
        QString par = i.next();
        if (par.startsWith("xsave=", Qt::CaseInsensitive) || par.startsWith("xs=", Qt::CaseInsensitive)) {
            needsXSave = false;
            i.setValue("xsave=" + fi.fileName());
        } else if (par.startsWith("action=", Qt::CaseInsensitive) || par.startsWith("a=", Qt::CaseInsensitive)) {
            needsActC = false;
            i.setValue("action=c");
        }
    }
    if (needsXSave) params << ("xsave=" + fi.fileName());
    if (needsActC) params << ("action=c");
    return params;
}

QStringList NeosProcess::remoteParameters()
{
    QStringList params = parameters();
    if (params.size()) params.removeFirst();
    QMutableListIterator<QString> i(params);
    bool needsFw = true;
    while (i.hasNext()) {
        QString par = i.next();
        if (par.startsWith("forceWork=", Qt::CaseInsensitive) || par.startsWith("fw=", Qt::CaseInsensitive)) {
            needsFw = false;
            i.setValue("fw=1");
        } else if (par.startsWith("action=", Qt::CaseInsensitive) || par.startsWith("a=", Qt::CaseInsensitive)) {
            i.remove();
            continue;
        } else if (par.startsWith("reference=", Qt::CaseInsensitive) || par.startsWith("rf=", Qt::CaseInsensitive)) {
            i.remove();
            continue;
        } else if (par.startsWith("xsave=", Qt::CaseInsensitive) || par.startsWith("xs=", Qt::CaseInsensitive)) {
            i.remove();
            continue;
        } else if (par.startsWith("previousWork=", Qt::CaseInsensitive)) {
            i.remove();
            continue;
        }
    }
    if (needsFw) params << ("fw=1");
    return params;
}

void NeosProcess::compileCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit || exitCode) {
        DEB() << "Error on compilation, exitCode " << QString::number(exitCode);
        completed(-1);
        return;
    }
    if (mNeosState == Neos1Compile) {
        QStringList params = remoteParameters();
        QString g00 = mOutPath + ".g00";
        mManager->submitJob(g00, params.join(" "), mPrio==prioShort);
    } else {
        DEB() << "Wrong step order: step 1 expected, step " << mNeosState << " faced.";
    }
}

void NeosProcess::unpackCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    setNeosState(NeosIdle);
    completed(exitCode);
}

void NeosProcess::sslErrors(const QStringList &errors)
{
    QString data("\n*** SSL errors:\n%1\n");
    emit newStdChannelData(data.arg(errors.join("\n")).toUtf8());
    if (mAskOnSslError) {
        QDialog *dialog = new QDialog();
        connect(dialog, &QDialog::finished, this, &NeosProcess::sslErrorDialogFinished);
        // TODO open dialog
    }
}

void NeosProcess::sslErrorDialogFinished(int result)
{
//    sender()
    mManager->setIgnoreSslErrors();
}

void NeosProcess::parseUnzipStdOut(const QByteArray &data)
{
    if (data.startsWith(" extracting: ")) {
        QByteArray fName = data.trimmed();
        fName = QString(QDir::separator()).toUtf8() + fName.right(fName.length() - fName.indexOf(':') -2);

        emit newStdChannelData("--- extracting: ."+ fName +"[FIL:\""+mOutPath.toUtf8()+fName+"\",0,0]\n");
    } else
        emit newStdChannelData(data);
}

void NeosProcess::unzipStateChanged(QProcess::ProcessState newState)
{
    if (newState == QProcess::NotRunning) {
        setNeosState(NeosIdle);
        mSubProc->deleteLater();
        completed(mSubProc->exitCode());
    }
}

void NeosProcess::interrupt()
{
    bool ok;
    mManager->killJob(ok);
    if (!ok) AbstractGamsProcess::interrupt();
    setNeosState(NeosIdle);
    completed(-1);
}

void NeosProcess::setParameters(const QStringList &parameters)
{
    if (parameters.size()) {
        mOutPath = parameters.size() ? parameters.first() : QString();
        if (mOutPath.startsWith('"'))
            mOutPath = mOutPath.mid(1, mOutPath.length()-2);
        int i = mOutPath.lastIndexOf('.');
        if (i >= 0) mOutPath = mOutPath.left(i);
    } else {
        mOutPath = QString();
    }
    AbstractProcess::setParameters(parameters);
}

QProcess::ProcessState NeosProcess::state() const
{
    return mNeosState==NeosIdle ? QProcess::NotRunning : QProcess::Running;
}

void NeosProcess::rePing(const QString &value)
{
    DEB() << "PING: " << value;
}

void NeosProcess::reVersion(const QString &value)
{
    DEB() << "VERSION: " << value;
}

void NeosProcess::reSubmitJob(const int &jobNumber, const QString &jobPassword)
{
    DEB() << "SUBMITED: " << jobNumber << " - pw: " << jobPassword;

    QString newLstEntry("\n--- switch to NEOS .%1%2%1solve.lst[LS2:\"%3\"]\n");
    QString name = mOutPath.split(QDir::separator(),QString::SkipEmptyParts).last();
    emit newStdChannelData(newLstEntry.arg(QDir::separator()).arg(name).arg(mOutPath).toUtf8());
    // TODO(JM) store jobnumber and password for later resuming

    // monitoring starts automatically after successfull submission
    setNeosState(Neos2Monitor);
}


enum JobStatusEnum {jsInvalid, jsDone, jsRunning, jsWaiting, jsUnknownJob, jsBadPassword};

static const QHash<QString, JobStatusEnum> CJobStatus {
    {"invalid", jsInvalid}, {"done", jsDone}, {"running", jsRunning}, {"waiting", jsWaiting},
    {"Unknown Job", jsUnknownJob}, {"Bad Password", jsBadPassword}
};

void NeosProcess::reGetJobStatus(const QString &status)
{
    switch (int iStatus = CJobStatus.value(status, jsInvalid)) {
    case jsDone: {
        DEB() << "finish state from: " << mNeosState;
        if (mNeosState == Neos2Monitor) {
            mManager->getCompletionCode();
            if (mPullTimer.isActive()) mPullTimer.stop();
            setNeosState(Neos3GetResult);
        }
    }   break;
    case jsRunning:
    case jsWaiting:
        if (!mPullTimer.isActive()) {
            DEB() << "intermediate status: " << status;
            mPullTimer.start();
        }
        break;
    case jsUnknownJob:
    case jsBadPassword:
    case jsInvalid:
        emit newStdChannelData("\n*** Neos error-status: "+status.toUtf8()+'\n');
        completed(-1);
        break;
    }
}


enum CompletionCodeEnum {ccInvalid, ccNormal, ccOutOfMemory, ccTimedOut, ccDiskSpace, ccServerError, ccUnknownJob, ccBadPassword};

static const QHash<QString, CompletionCodeEnum> CCompletionCodes {
    {"Invalid", ccInvalid}, {"Normal", ccNormal}, {"Out of memory", ccOutOfMemory}, {"Timed out", ccTimedOut},
    {"Disk Space", ccDiskSpace}, {"Server error", ccServerError}, {"Unknown Job", ccUnknownJob}, {"Bad Password", ccBadPassword}
};

void NeosProcess::reGetCompletionCode(const QString &code)
{
    switch (CCompletionCodes.value(code, ccInvalid)) {
    case ccNormal: {
        DEB() << "Normal completion - getting output";
        if (mPrio == prioLong)
            mManager->getFinalResultsNonBlocking();

        // TODO(JM) Load result file (for large files this may take a while, check if neos supports progress monitoring)
        mManager->getOutputFile("solver-output.zip");
    }   break;
    default:
        emit newStdChannelData("\n*** Neos error-exit: "+code.toUtf8()+'\n');
        completed(-1);
        setNeosState(NeosIdle);
        break;
    }
}

void NeosProcess::reGetJobInfo(const QStringList &info)
{
    DEB() << "NEOS-INFO: " << info.join(", ");
}

void NeosProcess::reKillJob(const QString &text)
{
    emit newStdChannelData('\n'+text.toUtf8()+'\n');
}

void NeosProcess::reGetIntermediateResultsNonBlocking(const QByteArray &data)
{
    QByteArray res = convertReferences(data);
    if (!res.isEmpty())
        emit newStdChannelData(res);
}

void NeosProcess::reGetFinalResultsNonBlocking(const QByteArray &data)
{
    QByteArray res = convertReferences(data);
    if (!res.isEmpty())
        emit newStdChannelData(res);
}

void NeosProcess::reGetOutputFile(const QByteArray &data)
{
    // TODO(JM) check if neos sends partial files when the result-file is too large
    QFile res(mOutPath+"/solver-output.zip");
    if (res.open(QFile::WriteOnly)) {
        res.write(data);
        res.flush();
        res.close();
    }
    startUnpacking();
}

void NeosProcess::reError(const QString &errorText)
{
    DEB() << "ERROR: " << errorText;
}

void NeosProcess::pullStatus()
{
    if (mPrio == prioShort)
        mManager->getIntermediateResultsNonBlocking();
    mManager->getJobStatus();
}

void NeosProcess::setNeosState(NeosState newState)
{
    if (newState != NeosIdle && int(newState) != int(mNeosState)+1) {
        DEB() << "Warning: NeosState jumped from " << mNeosState << " to " << newState;
    }
    QProcess::ProcessState stateBefore = state();
    mNeosState = newState;
    if (stateBefore != state())
        emit stateChanged(mNeosState==NeosIdle ? QProcess::NotRunning : QProcess::Running);
    emit neosStateChanged(this, mNeosState);
}

QByteArray NeosProcess::convertReferences(const QByteArray &data)
{
    QByteArray res;
    res.reserve(data.size()+mOutPath.length());
    QByteArray remotePath("/var/lib/condor/execute/dir_+/gamsexec/");
    QByteArray lstTag("[LST:");
    int iRP = 0;
    int iLT = 0;
    int iCount = 0;  // count of chars currently not copied

    for (int i = 0; i < data.size(); ++i) {
        if (iRP == remotePath.length()) {
            // add local path
            iRP = 0;
        }
        // Check if still in remotePath pattern
        if (data.at(i) >= '0' && data.at(i) <= '9') {
            if (remotePath.at(iRP) != '+') iRP = 0;
        } else {
            if (remotePath.at(iRP) == '+') ++iRP;
            if (iRP < remotePath.length()) {
                if (remotePath.at(iRP) == data.at(i)) ++iRP;
                else iRP = 0;
            }
        }
        // Check if still in lstTag pattern
        if (lstTag.at(iLT) == data.at(i)) ++iLT;
        else iLT = 0;
        ++iCount;
        if (iRP == remotePath.size()) {
            res.append(mOutPath+QDir::separator());
            iRP = 0;
            iLT = 0;
            iCount = 0;
        } else if (iLT == lstTag.size()) {
            res.append("[LS2:");
            iRP = 0;
            iLT = 0;
            iCount = 0;
        } else if (!iRP && !iLT) {
            res.append(data.mid(i+1-iCount, iCount));
            iCount = 0;
        }
    }
    return res;
}

void NeosProcess::startUnpacking()
{
    mSubProc = new GmsunzipProcess(this);
    mSubProc->setWorkingDirectory(mOutPath);
    QStringList params;
    params << "-o solver-output.zip";
    mSubProc->setParameters(params);
    connect(mSubProc, &GmsunzipProcess::stateChanged, this, &NeosProcess::unzipStateChanged);
    connect(mSubProc, QOverload<int, QProcess::ExitStatus>::of(&GmsunzipProcess::finished), this, &NeosProcess::unpackCompleted);
    connect(mSubProc, &GmsunzipProcess::newStdChannelData, this, &NeosProcess::parseUnzipStdOut);

    mSubProc->execute();

//#if defined(__unix__) || defined(__APPLE__)
//    mSubProc.start(nativeAppPath("gmsunzip"), params);
//#else
//    mSubProc.setNativeArguments(params.join(" "));
//    mSubProc.setProgram(nativeAppPath("gmsunzip"));
//    mSubProc.start();
//#endif
}

} // namespace neos
} // namespace studio
} // namespace gams
