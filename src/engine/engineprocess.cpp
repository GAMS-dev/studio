#include "engineprocess.h"
#include "client/OAIJobsApi.h"
#include "enginemanager.h"
#include "logger.h"
#include "commonpaths.h"
#include "process/gmsunzipprocess.h"
#include "process/gmszipprocess.h"
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>

#ifdef _WIN32
#include "Windows.h"
#endif

namespace gams {
namespace studio {
namespace engine {

/*
url: https://miro.gams.com/engine/api
namespace: studiotests
user: studiotests
password: rercud-qinRa9-wagbew
*/

EngineProcess::EngineProcess(QObject *parent) : AbstractGamsProcess("gams", parent), mProcState(ProcCheck)
{
    disconnect(&mProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(completed(int)));
    connect(&mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &EngineProcess::compileCompleted);

    mManager = new EngineManager(this);
    connect(mManager, &EngineManager::sslErrors, this, &EngineProcess::sslErrors);
    connect(mManager, &EngineManager::reAuth, this, &EngineProcess::authenticated);
    connect(mManager, &EngineManager::rePing, this, &EngineProcess::rePing);
    connect(mManager, &EngineManager::reError, this, &EngineProcess::reError);
    connect(mManager, &EngineManager::reKillJob, this, &EngineProcess::reKillJob, Qt::QueuedConnection);
    connect(mManager, &EngineManager::reVersion, this, &EngineProcess::reVersion);
    connect(mManager, &EngineManager::reCreateJob, this, &EngineProcess::reCreateJob);
    connect(mManager, &EngineManager::reGetJobStatus, this, &EngineProcess::reGetJobStatus);
    connect(mManager, &EngineManager::reGetOutputFile, this, &EngineProcess::reGetOutputFile);
    connect(mManager, &EngineManager::reGetLog, this, &EngineProcess::reGetLog);

    mPullTimer.setInterval(1000);
    mPullTimer.setSingleShot(true);
    connect(&mPullTimer, &QTimer::timeout, this, &EngineProcess::pullStatus);
}

EngineProcess::~EngineProcess()
{
    delete mManager;
}

void EngineProcess::execute()
{
    QStringList params = compileParameters();
    mProcess.setWorkingDirectory(workingDirectory());
    mManager->setWorkingDirectory(workingDirectory());

#if defined(__unix__) || defined(__APPLE__)
    mProcess.start(nativeAppPath(), params);
#else
    mProcess.setNativeArguments(params.join(" "));
    mProcess.setProgram(nativeAppPath());
    mProcess.start();
#endif

    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    setProcState(Proc1Compile);
}

QStringList EngineProcess::compileParameters()
{
    if (mOutPath.isEmpty()) {
        DEB() << "Error: No runable file assigned to the GAMS Engine process";
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

QStringList EngineProcess::remoteParameters()
{
    QStringList params = parameters();
    if (params.size()) params.removeFirst();
    QMutableListIterator<QString> i(params);
    bool needsFw = true;
    bool needsRestart = true;
    while (i.hasNext()) {
        QString par = i.next();
        if (par.startsWith("forceWork=", Qt::CaseInsensitive) || par.startsWith("fw=", Qt::CaseInsensitive)) {
            needsFw = false;
            i.setValue("fw=1");
        } else if (par.startsWith("restart=", Qt::CaseInsensitive)) {
            needsRestart = false;
            continue;
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
    if (needsRestart) params << ("restart="+modelName());
    return params;
}

void EngineProcess::compileCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit || exitCode) {
        DEB() << "Error on compilation, exitCode " << QString::number(exitCode);
        completed(-1);
        return;
    }
    if (mProcState == Proc1Compile) {
        QStringList params = remoteParameters();
        setProcState(Proc2Pack);
        startPacking();
    } else {
        DEB() << "Wrong step order: step 1 expected, step " << mProcState << " faced.";
        completed(-1);
    }
}

void EngineProcess::packCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode || exitStatus == QProcess::CrashExit) {
        emit newStdChannelData("\nErrors while packing. exitCode: " + QString::number(exitCode).toUtf8());
        completed(exitCode);
    } else if (mProcState == Proc2Pack) {
        QString modlName = modelName();
        QString zip = mOutPath + QDir::separator() + modlName + ".zip";

        mManager->submitJob(modlName, mNamespace, zip, remoteParameters());
        setProcState(Proc3Monitor);
    }
    mSubProc->deleteLater();
    mSubProc = nullptr;
}

void EngineProcess::unpackCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    setProcState(ProcIdle);
    completed(exitCode);
}

void EngineProcess::sslErrors(const QStringList &errors)
{
    QString data("\n*** SSL errors:\n%1\n");
    emit newStdChannelData(data.arg(errors.join("\n")).toUtf8());
    if (mProcState == ProcCheck) {
        emit sslValidation(errors.join("\n").toUtf8());
    }
}

void EngineProcess::parseUnZipStdOut(const QByteArray &data)
{
    if (data.startsWith(" extracting: ")) {
        QByteArray fName = data.trimmed();
        fName = QString(QDir::separator()).toUtf8() + fName.right(fName.length() - fName.indexOf(':') -2);
        QByteArray folder = mOutPath.split(QDir::separator(),QString::SkipEmptyParts).last().toUtf8();
        folder.prepend(QDir::separator().toLatin1());
        emit newStdChannelData("--- extracting: ."+ folder + fName +"[FIL:\""+mOutPath.toUtf8()+fName+"\",0,0]");
        if (data.endsWith("\n")) emit newStdChannelData("\n");
    } else
        emit newStdChannelData(data);
}

void EngineProcess::subProcStateChanged(QProcess::ProcessState newState)
{
    if (newState == QProcess::NotRunning) {
        setProcState(ProcIdle);
        mSubProc->deleteLater();
        completed(mSubProc->exitCode());
    }
}

void EngineProcess::interrupt()
{
    bool ok = !mManager->getToken().isEmpty();
    if (ok)
        emit mManager->syncKillJob(false);
    else
        AbstractGamsProcess::interrupt();
}

void EngineProcess::terminate()
{
    bool ok = !mManager->getToken().isEmpty();
    if (ok)
        emit mManager->syncKillJob(true);
    else
        AbstractGamsProcess::terminate();
    // ensure termination
    setProcState(ProcIdle);
    completed(-1);
}

void EngineProcess::setParameters(const QStringList &parameters)
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

QProcess::ProcessState EngineProcess::state() const
{
    return (mProcState <= ProcIdle) ? QProcess::NotRunning : QProcess::Running;
}

void EngineProcess::authenticate(const QString &host, const QString &user, const QString &password)
{
    mManager->setUrl(host);
    mManager->authenticate(user, password);
    setProcState(ProcIdle);
}

//void EngineProcess::authenticate(const QString &host, const QString &token)
//{

//}

void EngineProcess::setNamespace(const QString &nSpace)
{
    mNamespace = nSpace;
}

void EngineProcess::setIgnoreSslErrors()
{
    mManager->setIgnoreSslErrors();
    if (mProcState == ProcCheck) {
        setProcState(ProcIdle);
    }
}

void EngineProcess::getVersions()
{

}

void EngineProcess::completed(int exitCode)
{
    disconnect(&mPullTimer, &QTimer::timeout, this, &EngineProcess::pullStatus);
    mPullTimer.stop();
    setProcState(ProcIdle);
    AbstractGamsProcess::completed(exitCode);
}

void EngineProcess::rePing(const QString &value)
{
    Q_UNUSED(value)
    if (mProcState == ProcCheck) {
        setProcState(ProcIdle);
        emit sslValidation(QString());
    }
}

void EngineProcess::reVersion(const QString &engineVersion, const QString &gamsVersion)
{
    // TODO(JM) compare GAMS version
    if (mProcState == ProcCheck) {
        setProcState(ProcIdle);
//        emit sslValidation(QString());
    }
    mEngineVersion = engineVersion;
    mGamsVersion = gamsVersion;
    GAMS_DISTRIB_VERSION_SHORT;
}

void EngineProcess::reVersionError()
{

}

void EngineProcess::reCreateJob(const QString &message, const QString &token)
{
    Q_UNUSED(message)
    mManager->setToken(token);
    QString newLstEntry("\n--- switch to Engine .%1%2%1%2.lst[LS2:\"%3\"]\nTOKEN: %4\n\n");
    QString lstPath = mOutPath+"/"+modelName()+".lst";
    emit newStdChannelData(newLstEntry.arg(QDir::separator()).arg(modelName()).arg(lstPath).arg(token).toUtf8());
    // TODO(JM) store token for later resuming
    // monitoring starts automatically after successfull submission
    pullStatus();
}


enum JobStatusEnum {jsInvalid, jsDone, jsRunning, jsWaiting, jsUnknownJob, jsBadPassword};

static const QHash<QString, JobStatusEnum> CJobStatus {
    {"invalid", jsInvalid}, {"done", jsDone}, {"running", jsRunning}, {"waiting", jsWaiting},
    {"Unknown Job", jsUnknownJob}, {"Bad Password", jsBadPassword}
};

void EngineProcess::reGetJobStatus(const qint32 &status, const qint32 &gamsExitCode)
{
    // TODO(JM) convert status to EngineManager::Status
    EngineManager::StatusCode code = EngineManager::StatusCode(status);
    if (code == EngineManager::Finished && mProcState == Proc3Monitor) {
        mManager->getLog();
        if (gamsExitCode) {
            QByteArray code = QString::number(gamsExitCode).toLatin1();
            emit newStdChannelData("\nGAMS terminated with exit code " +code+ "\n");
            completed(-1);
            return;
        }
        setProcState(Proc4GetResult);
        mManager->getOutputFile();
    }
}

void EngineProcess::reKillJob(const QString &text)
{
    emit newStdChannelData('\n'+text.toUtf8()+'\n');
}

void EngineProcess::reGetLog(const QByteArray &data)
{
    QByteArray res = convertReferences(data);
    if (!res.isEmpty())
        emit newStdChannelData(res);
}

void EngineProcess::reGetOutputFile(const QByteArray &data)
{
    disconnect(&mPullTimer, &QTimer::timeout, this, &EngineProcess::pullStatus);
    mPullTimer.stop();
    if (data.isEmpty()) {
        emit newStdChannelData("\nEmpty result received\n");
        completed(-1);
        return;
    }
    QFile res(mOutPath+"/solver-output.zip");
    if (res.exists() && !res.remove()) {
        emit newStdChannelData("\nError on removing file "+res.fileName().toUtf8()+"\n");
        completed(-1);
    }
    if (res.open(QFile::WriteOnly)) {
        res.write(data);
        res.flush();
        res.close();
        startUnpacking();
        return;
    }
    emit newStdChannelData("\nError writing file "+res.fileName().toUtf8()+"\n");
    completed(-1);
}

void EngineProcess::reError(const QString &errorText)
{
    disconnect(&mPullTimer, &QTimer::timeout, this, &EngineProcess::pullStatus);
    mPullTimer.stop();
    emit newStdChannelData("\n"+errorText.toUtf8()+"\n");
    completed(-1);
}

void EngineProcess::pullStatus()
{
    mManager->getLog();
    mManager->getJobStatus();
    mPullTimer.start();
}

void EngineProcess::setProcState(ProcState newState)
{
    if (newState != ProcIdle && int(newState) != int(mProcState)+1) {
        DEB() << "Warning: ProcState jumped from " << mProcState << " to " << newState;
    }
    QProcess::ProcessState stateBefore = state();
    mProcState = newState;
    if (stateBefore != state())
        emit stateChanged(mProcState == ProcIdle ? QProcess::NotRunning : QProcess::Running);
    emit procStateChanged(this, mProcState);
}

QByteArray EngineProcess::convertReferences(const QByteArray &data)
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

void EngineProcess::startPacking()
{
    GmszipProcess *subProc = new GmszipProcess(this);
//    connect(subProc, &GmszipProcess::stateChanged, this, &EngineProcess::subProcStateChanged);
    connect(subProc, QOverload<int, QProcess::ExitStatus>::of(&GmszipProcess::finished), this, &EngineProcess::packCompleted);
    connect(subProc, &GmsunzipProcess::newStdChannelData, this, &EngineProcess::parseUnZipStdOut);
    connect(subProc, &GmsunzipProcess::newProcessCall, this, &EngineProcess::newProcessCall);

    QFileInfo path(mOutPath);
    QString baseName = modelName();
    QFile file(mOutPath+'/'+baseName+".gms");
    if (!file.open(QFile::WriteOnly)) {
        emit newStdChannelData("\n*** Can't create file: "+file.fileName().toUtf8()+'\n');
        completed(-1);
        return;
    }
    file.write("*dummy");
    file.close();
    file.setFileName(mOutPath+'/'+baseName+".g00");
    if (file.exists() && !file.remove()) {
        emit newStdChannelData("\n*** Can't remove file from subdirectory: "+file.fileName().toUtf8()+'\n');
        completed(-1);
        return;
    }
    file.setFileName(mOutPath+".g00");
    if (!file.rename(mOutPath+'/'+baseName+".g00")) {
        emit newStdChannelData("\n*** Can't move file to subdirectory: "+file.fileName().toUtf8()+'\n');
        completed(-1);
        return;
    }

    mSubProc = subProc;
    subProc->setWorkingDirectory(mOutPath);
    subProc->setParameters(QStringList() << "-8"<< "-m" << baseName+".zip" << baseName+".gms" << baseName+".g00");
    subProc->execute();
}

void EngineProcess::startUnpacking()
{
    GmsunzipProcess *subProc = new GmsunzipProcess(this);
    connect(subProc, &GmsunzipProcess::stateChanged, this, &EngineProcess::subProcStateChanged);
    connect(subProc, QOverload<int, QProcess::ExitStatus>::of(&GmsunzipProcess::finished), this, &EngineProcess::unpackCompleted);
    connect(subProc, &GmsunzipProcess::newStdChannelData, this, &EngineProcess::parseUnZipStdOut);
    connect(subProc, &GmsunzipProcess::newProcessCall, this, &EngineProcess::newProcessCall);

    mSubProc = subProc;
    subProc->setWorkingDirectory(mOutPath);
    subProc->setParameters(QStringList() << "-o" << "solver-output.zip");
    subProc->execute();
}

QString EngineProcess::modelName()
{
    return QFileInfo(mOutPath).completeBaseName();
}

} // namespace engine
} // namespace studio
} // namespace gams
