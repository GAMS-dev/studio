/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "neosprocess.h"
#include "neosmanager.h"
#include "logger.h"
#include "commonpaths.h"
#include "process/gmsunzipprocess.h"
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include "support/solverconfiginfo.h"

#ifdef _WIN64
#include "Windows.h"
#endif

namespace gams {
namespace studio {
namespace neos {


NeosProcess::NeosProcess(QObject *parent) : AbstractGamsProcess("gams", parent), mProcState(ProcCheck)
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
    mPreparationState = QProcess::NotRunning;
}

NeosProcess::~NeosProcess()
{
    delete mManager;
}

void NeosProcess::execute()
{
    mInLogClone = false;
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
    setProcState(Proc1Compile);
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
    bool hasPw = false;
    while (i.hasNext()) {
        QString par = i.next();
        if (par.startsWith("xsave=", Qt::CaseInsensitive) || par.startsWith("xs=", Qt::CaseInsensitive)) {
            needsXSave = false;
            i.setValue("xsave=\"" + fi.fileName() + '"');
        } else if (par.startsWith("action=", Qt::CaseInsensitive) || par.startsWith("a=", Qt::CaseInsensitive)) {
            needsActC = false;
            i.setValue("action=c");
        } else if (par.startsWith("previousWork=", Qt::CaseInsensitive)) {
            hasPw = true;
            continue;
        }
    }
    if (needsXSave) params << ("xsave=\"" + fi.fileName() + '"');
    if (!hasPw) params << ("previousWork=1");
    if (needsActC) params << ("action=c");
    return params;
}

QStringList NeosProcess::remoteParameters()
{
    QStringList params = parameters();
    if (params.size()) params.removeFirst();
    QMutableListIterator<QString> i(params);
    bool needsGdx = mForceGdx;
    mHasGdx = mForceGdx;
    while (i.hasNext()) {
        QString par = i.next();
        if (par.startsWith("action=", Qt::CaseInsensitive) || par.startsWith("a=", Qt::CaseInsensitive)) {
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
        } else if (par.startsWith("gdx=", Qt::CaseInsensitive)) {
            mHasGdx = true;
            needsGdx = false;
            i.remove();
            continue;
        }
    }
    if (needsGdx) params << ("gdx=default");
    return params;
}

void NeosProcess::solverToCategories(QStringList &params)
{
    // replaces all occurrencies of "solver=%s" by a list of matching "%cat=%s"
    for (int i = 0; i < params.size(); ++i) {
        QString par = params.at(i);
        if (par.startsWith("solver=", Qt::CaseInsensitive)) {
            QString solver = par.split("=").last();
            QStringList catParams = allCat4Solver(solver);
            cleanCategories(params, catParams);
            if (catParams.isEmpty())
                params.removeAt(i--);
            for (QString &catPar : catParams)
                params.insert(++i, catPar);
        }
    }
}

void NeosProcess::cleanCategories(const QStringList &params, QStringList &catParams)
{
    for (const QString &par : params) {
        QString pre = par.split("=").first() + "=";
        for (int i = 0; i < catParams.size(); ++i) {
            if (catParams.at(i).startsWith(pre, Qt::CaseInsensitive))
                catParams.removeAt(i--);
        }
    }
}

QStringList NeosProcess::allCat4Solver(const QString &solver)
{
    QStringList res;
    support::SolverConfigInfo solveInfo;
    int iSolver = solveInfo.solverId(solver);
    if (iSolver == 0)
        return res;
    for (int i = 0; i < solveInfo.modelTypes(); ++i) {
        if (solveInfo.solverCapability(iSolver, i)) {
            res << solveInfo.modelTypeName(i) + "=" + solver;
        }
    }
    return res;
}

void NeosProcess::compileCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit || exitCode) {
        DEB() << "Error on compilation, exitCode " << QString::number(exitCode);
        setProcState(ProcIdle);
        completed(-1);
        return;
    }
    if (mProcState == Proc1Compile) {
        QStringList params = remoteParameters();
        solverToCategories(params);
        QString g00 = mOutPath + ".g00";
        bool ok = mManager->submitJob(g00, mMail, params.join(" "), mPrio==prioShort, mHasGdx);
        if (!ok) completed(-1);
    } else {
        DEB() << "Wrong step order: step 1 expected, step " << mProcState << " faced.";
    }
}

void NeosProcess::unpackCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    setProcState(ProcIdle);
    completed(exitCode);
}

void NeosProcess::sslErrors(const QStringList &errors)
{
    QString data("\n*** SSL errors:\n%1\n");
    emit newStdChannelData(data.arg(errors.join("\n")).toUtf8());
    if (mProcState == ProcCheck) {
        emit sslValidation(errors.join("\n").toUtf8());
    }
}

void NeosProcess::parseUnzipStdOut(const QByteArray &data)
{
    if (data.startsWith(" extracting: ") || data.startsWith("  inflating: ")) {
//        QByteArray preText = "--- " + data.left(data.indexOf(":")+1).trimmed() + " .";
        QByteArray fName = data.trimmed();
        fName = QString(QDir::separator()).toUtf8() + fName.right(fName.length() - fName.indexOf(':') -2);
        QByteArray folder = mOutPath.split(QDir::separator(), Qt::SkipEmptyParts).last().toUtf8();
        folder.prepend(QDir::separator().toLatin1());
        if (fName.endsWith("lxi")) {
            emit newStdChannelData("--- skipping: ."+ folder + fName);
            if (data.endsWith("\n")) emit newStdChannelData("\n");
         } else {
            emit newStdChannelData("--- extracting: ."+ folder + fName +"[FIL:\""+mOutPath.toUtf8()+fName+"\",0,0]");
            if (data.endsWith("\n")) emit newStdChannelData("\n");
        }
    } else
        emit newStdChannelData(data);
}

void NeosProcess::unzipStateChanged(QProcess::ProcessState newState)
{
    if (newState == QProcess::NotRunning) {
        setProcState(ProcIdle);
        mSubProc->deleteLater();
        completed(mSubProc->exitCode());
    }
}

void NeosProcess::interrupt()
{
    bool ok;
    mManager->killJob(ok); // This is currently ignored by the server
    if (!ok) interruptIntern();
    setProcState(ProcIdle);
    completed(-1);
}

void NeosProcess::terminate()
{
    bool ok = false;
    mManager->killJob(ok); // This is currently ignored by the server
    if (!ok) interruptIntern(true);
    setProcState(ProcIdle);
    completed(-1);
}

void NeosProcess::terminateLocal()
{
    interruptIntern();
    setProcState(ProcIdle);
    completed(-1);
}

void NeosProcess::setParameters(const QStringList &parameters)
{
    mOutPath = workingDirectory();
    if (parameters.size()) {
        QString name = parameters.size() ? parameters.first() : QString();
        if (name.startsWith('"'))
            name = name.mid(1, name.length()-2);
        name = QFileInfo(name).completeBaseName();
        if (!name.isEmpty()) mOutPath += QDir::separator() + name;
    }
    AbstractProcess::setParameters(parameters);
}

QProcess::ProcessState NeosProcess::state() const
{
    return (mProcState <= ProcIdle) ? mPreparationState : QProcess::Running;
}

void NeosProcess::validate()
{
    mManager->ping();
}

void NeosProcess::setIgnoreSslErrors()
{
    mManager->setIgnoreSslErrors();
    if (mProcState == ProcCheck) {
        setProcState(ProcIdle);
    }
}

void NeosProcess::setStarting()
{
    if (mPreparationState == QProcess::NotRunning) {
        mPreparationState = QProcess::Starting;
        emit stateChanged(QProcess::Starting);
    }
}

void NeosProcess::completed(int exitCode)
{
    mPreparationState = QProcess::NotRunning;
    AbstractGamsProcess::completed(exitCode);
    emit stateChanged(QProcess::NotRunning);
}

void NeosProcess::rePing(const QString &value)
{
    Q_UNUSED(value)
    if (mProcState == ProcCheck) {
        setProcState(ProcIdle);
        emit sslValidation(QString());
    }
}

void NeosProcess::reVersion(const QString &value)
{
    Q_UNUSED(value)
    DEB() << "VERSION: " << value;
}

void NeosProcess::reSubmitJob(const int &jobNumber, const QString &jobPassword)
{
    if (!jobNumber) {
        emit newStdChannelData("NEOS: " + jobPassword.toUtf8());
        terminate();
        return;
    }
    DEB() << "SUBMITED: " << jobNumber << " - pw: " << jobPassword;

    QString credentials;
    if (mPrio == prioLong) {
        credentials = QString("\nJob %1 dispatched\npassword: %2").arg(jobNumber).arg(jobPassword);
    }
    QString newLstEntry("\n--- switch to NEOS .%1%2%1solve.lst[LS2:\"%3\"]%4\n");
    QString name = mOutPath.split(QDir::separator(), Qt::SkipEmptyParts).last();
    emit newStdChannelData(newLstEntry.arg(QDir::separator(), name, mOutPath+"/solve.lst", credentials).toUtf8());
    // TODO(JM) store jobnumber and password for later resuming

    // monitoring starts automatically after successfull submission
    setProcState(Proc2Pack);
    setProcState(Proc4Monitor);
}


enum JobStatusEnum {jsInvalid, jsDone, jsRunning, jsWaiting, jsUnknownJob, jsBadPassword};

static const QHash<QString, JobStatusEnum> CJobStatus {
    {"invalid", jsInvalid}, {"done", jsDone}, {"running", jsRunning}, {"waiting", jsWaiting},
    {"Unknown Job", jsUnknownJob}, {"Bad Password", jsBadPassword}
};

void NeosProcess::reGetJobStatus(const QString &status)
{
    int iStatus = CJobStatus.value(status, jsInvalid);
    switch (iStatus) {
    case jsDone: {
        if (mProcState == Proc4Monitor) {
            mManager->getCompletionCode();
            if (mPullTimer.isActive()) mPullTimer.stop();
            setProcState(Proc5GetResult);
        }
    }   break;
    case jsRunning:
    case jsWaiting:
        if (!mPullTimer.isActive()) {
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
        if (mPrio == prioLong)
            mManager->getFinalResultsNonBlocking();

        // TODO(JM) for large result-file this may take a while, check if neos supports progress monitoring
        mManager->getOutputFile("solver-output.zip");
    }   break;
    default:
        emit newStdChannelData("\n*** Neos error-exit: "+code.toUtf8()+'\n');
        bool hasPrevWork = false;
        for (const QString &param : parameters()) {
            if (param.startsWith("PreviousWork", Qt::CaseInsensitive))
                hasPrevWork = true;
        }
        if (!hasPrevWork)
            emit newStdChannelData("    (you may try adding the parameter \"PreviousWork=1\")\n");
        completed(-1);
        setProcState(ProcIdle);
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
        emit newStdChannelData("[]"+res);
}

void NeosProcess::reGetFinalResultsNonBlocking(const QByteArray &data)
{
    QByteArray res = convertReferences(data);
    if (!res.isEmpty())
        emit newStdChannelData("[]"+res);
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
    emit newStdChannelData("Network error: " + errorText.toUtf8() +'\n');
    completed(-1);
}

void NeosProcess::pullStatus()
{
    if (mPrio == prioShort)
        mManager->getIntermediateResultsNonBlocking();
    mManager->getJobStatus();
}

void NeosProcess::setProcState(ProcState newState)
{
    if (newState != ProcIdle && int(newState) != int(mProcState)+1) {
        DEB() << "Warning: NeosState jumped from " << mProcState << " to " << newState;
    }
    QProcess::ProcessState stateBefore = state();
    mProcState = newState;
    if (stateBefore != state())
        emit stateChanged(mProcState == ProcIdle ? mPreparationState : QProcess::Running);
    emit procStateChanged(this, mProcState);
}

QByteArray NeosProcess::convertReferences(const QByteArray &data)
{
    QByteArray res;
    if (mInLogClone) return res;
    res.reserve(data.size()+mOutPath.length());
    QByteArray remotePath("/var/lib/condor/execute/dir_+/gamsexec/");
    QByteArray lstTag("[LST:");
    QByteArray logClone("%% GAMS LOGFILE %%%%");
    int iRP = 0;
    int iLT = 0;
    int iLC = 0;
    int iCount = 0;  // count of chars currently not copied

    for (int i = 0; i < data.size(); ++i) {
        if (data.at(i) == logClone.at(iLC)) {
            ++iLC;
            if (iLC == logClone.size()) {
                mInLogClone = true;
                break;
            }
            ++iCount;
            continue;
        } else if (iLC > 0) {
            res.append(data.mid(i+1-iCount, iCount));
            iCount = 0;
            iLC = 0;
        }
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
            QByteArray model("MODEL.");
            if (data.size() > i+1 + model.size() && data.mid(i+1, model.size()).compare(model) == 0) {
                res.append(mOutPath.toUtf8());
                res.append(".");
                i += model.size();
            } else {
                res.append(mOutPath.toUtf8());
                res.append(QDir::separator().toLatin1());
            }

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
    GmsunzipProcess *subProc = new GmsunzipProcess(this);
    connect(subProc, &GmsunzipProcess::stateChanged, this, &NeosProcess::unzipStateChanged);
    connect(subProc, QOverload<int, QProcess::ExitStatus>::of(&GmsunzipProcess::finished), this, &NeosProcess::unpackCompleted);
    connect(subProc, &GmsunzipProcess::newStdChannelData, this, &NeosProcess::parseUnzipStdOut);
    connect(subProc, &GmsunzipProcess::newProcessCall, this, &NeosProcess::newProcessCall);

    mSubProc = subProc;
    subProc->setWorkingDirectory(mOutPath);
    subProc->setParameters(QStringList() << "-o" << "solver-output.zip");
    subProc->execute();
}

void NeosProcess::setForceGdx(bool forceGdx)
{
    mForceGdx = forceGdx;
}

} // namespace neos
} // namespace studio
} // namespace gams
