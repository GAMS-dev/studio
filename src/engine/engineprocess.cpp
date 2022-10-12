/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
 */
#include "engineprocess.h"
#include "client/OAIJobsApi.h"
#include "enginemanager.h"
#include "logger.h"
#include "commonpaths.h"
#include "process/gmsunzipprocess.h"
#include "process/gmszipprocess.h"
#include "file/filetype.h"

#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QNetworkConfiguration>

#ifdef _WIN32
#include "Windows.h"
#endif

namespace gams {
namespace studio {
namespace engine {

EngineProcess::EngineProcess(QObject *parent) : AbstractGamsProcess("gams", parent), mProcState(ProcCheck)
{
    disconnect(&mProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(completed(int)));
    connect(&mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &EngineProcess::compileCompleted);

    mManager = new EngineManager(this);
    connect(mManager, &EngineManager::reVersion, this, &EngineProcess::reVersion);
    connect(mManager, &EngineManager::reVersion, this, &EngineProcess::reVersionIntern);
    connect(mManager, &EngineManager::reVersionError, this, &EngineProcess::reVersionError);
    connect(mManager, &EngineManager::reUserInstances, this, &EngineProcess::reUserInstances);
    connect(mManager, &EngineManager::reUserInstancesError, this, &EngineProcess::reUserInstancesError);
    connect(mManager, &EngineManager::reQuota, this, &EngineProcess::reQuota);
    connect(mManager, &EngineManager::reQuotaError, this, &EngineProcess::reQuotaError);
    connect(mManager, &EngineManager::sslErrors, this, &EngineProcess::sslErrors);
    connect(mManager, &EngineManager::reAuthorize, this, &EngineProcess::reAuthorize);
    connect(mManager, &EngineManager::reAuthorizeError, this, &EngineProcess::authorizeError);
    connect(mManager, &EngineManager::rePing, this, &EngineProcess::rePing);
    connect(mManager, &EngineManager::reError, this, &EngineProcess::reError);
    connect(mManager, &EngineManager::reKillJob, this, &EngineProcess::reKillJob, Qt::QueuedConnection);
    connect(mManager, &EngineManager::reListJobs, this, &EngineProcess::reListJobs);
    connect(mManager, &EngineManager::reListJobsError, this, &EngineProcess::reListJobsError);
    connect(mManager, &EngineManager::reListNamspaces, this, &EngineProcess::reListNamspaces);
    connect(mManager, &EngineManager::reListNamespacesError, this, &EngineProcess::reListNamespacesError);
    connect(mManager, &EngineManager::reCreateJob, this, &EngineProcess::reCreateJob);
    connect(mManager, &EngineManager::reGetJobStatus, this, &EngineProcess::reGetJobStatus);
    connect(mManager, &EngineManager::reGetOutputFile, this, &EngineProcess::reGetOutputFile);
    connect(mManager, &EngineManager::reGetLog, this, &EngineProcess::reGetLog);
    connect(mManager, &EngineManager::jobIsQueued, this, &EngineProcess::jobIsQueued);
    connect(mManager, &EngineManager::allPendingRequestsCompleted, this, &EngineProcess::allPendingRequestsCompleted);

    setIgnoreSslErrorsCurrentUrl(false);
    mPollTimer.setInterval(1000);
    mPollTimer.setSingleShot(true);
    connect(&mPollTimer, &QTimer::timeout, this, &EngineProcess::pollStatus);
}

EngineProcess::~EngineProcess()
{
    delete mManager;
}

void EngineProcess::startupInit()
{
    EngineManager::startupInit();
}

void EngineProcess::execute()
{
    QDir dir(mOutPath);
    if (dir.exists() && !modelName().isEmpty()) {
        emit newStdChannelData("\nCan't create directory " + mOutPath.toUtf8() + '\n');
        setProcState(ProcIdle);
        return;
    }
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

QStringList EngineProcess::compileParameters() const
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
    bool needsPw = mForcePreviousWork;
    while (i.hasNext()) {
        QString par = i.next();
        if (par.startsWith("xsave=", Qt::CaseInsensitive) || par.startsWith("xs=", Qt::CaseInsensitive)) {
            needsXSave = false;
            i.setValue("xsave=" + modelName());
        } else if (par.startsWith("action=", Qt::CaseInsensitive) || par.startsWith("a=", Qt::CaseInsensitive)) {
            needsActC = false;
            i.setValue("action=c");
        } else if (par.startsWith("previousWork=", Qt::CaseInsensitive)) {
            needsPw = false;
            continue;
        }
    }
    if (needsXSave) params << ("xsave=" + modelName());
    if (needsActC) params << ("action=c");
    if (needsPw) params << ("previousWork=1");
    return params;
}

QStringList EngineProcess::remoteParameters() const
{
    QStringList params = parameters();
    if (params.size()) params.removeFirst();
    QMutableListIterator<QString> i(params);
    bool needsRestart = true;
    bool needsGdx = mForceGdx;
    while (i.hasNext()) {
        QString par = i.next();
         if (par.startsWith("restart=", Qt::CaseInsensitive)) {
            needsRestart = false;
            continue;
        } else if (par.startsWith("gdx=", Qt::CaseInsensitive)) {
            needsGdx = false;
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
    if (needsGdx) params << ("gdx=default");
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
        DEB() << remoteParameters().join("\n   ");
        setProcState(Proc2Pack);
        startPacking();
    } else {
        DEB() << "Wrong step order: step 1 expected, step " << mProcState << " faced.";
        completed(-1);
    }
}

void EngineProcess::packCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    mSubProc->deleteLater();
    mSubProc = nullptr;
    if (exitCode || exitStatus == QProcess::CrashExit) {
        emit newStdChannelData("\nErrors while packing. exitCode: " + QString::number(exitCode).toUtf8());
        completed(exitCode);
    } else if (mProcState == Proc2Pack) {
        setProcState(Proc2Pack2);
        startPacking2();
    }
}

void EngineProcess::pack2Completed(int exitCode, QProcess::ExitStatus exitStatus)
{
    mSubProc->deleteLater();
    mSubProc = nullptr;
    if (exitCode || exitStatus == QProcess::CrashExit) {
        emit newStdChannelData("\nErrors while packing2. exitCode: " + QString::number(exitCode).toUtf8());
        completed(exitCode);
    } else if (mProcState == Proc2Pack2) {
        QString modlName = modelName();
        QString zip = mOutPath + QDir::separator() + modlName + ".zip";

        QString instance;
        if (mInKubernetes) instance = mUserInstance;
        mManager->submitJob(modlName, mNamespace, zip, remoteParameters(), instance);
        setProcState(Proc3Queued);
    }
}

void EngineProcess::unpackCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)

    QFile gms(mOutPath+'/'+modelName()+".gms");
    if (gms.exists()) gms.remove();
    QFile g00(mOutPath+'/'+modelName()+".g00");
    if (g00.exists()) g00.remove();
    handleResultFiles();

    setProcState(ProcIdle);
    completed(exitCode);
}

void EngineProcess::sslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    Q_UNUSED(reply)
    QString data("\n*** SSL errors:\n");
    int sslError = 0;
    for (const QSslError &err : errors) {
        data.append(QString(" [%1] %2\n").arg(err.error()).arg(err.errorString()));
        if (err.error() == QSslError::SelfSignedCertificate ||
                err.error() == QSslError::SelfSignedCertificateInChain ||
                err.error() == QSslError::CertificateStatusUnknown)
            sslError = err.error();
    }
    emit newStdChannelData(data.toUtf8());
    if (sslError)
        emit sslSelfSigned(sslError);
}

void EngineProcess::parseUnZipStdOut(const QByteArray &data)
{
    if (data.startsWith(" extracting: ")) {
        QByteArray fName = data.trimmed();
        fName = QString(QDir::separator()).toUtf8() + fName.right(fName.length() - fName.indexOf(':') -2);
        QByteArray folder = mOutPath.split(QDir::separator(), Qt::SkipEmptyParts).last().toUtf8();
        folder.prepend(QDir::separator().toLatin1());
        if (fName.startsWith(folder) && (fName.endsWith("gms") || fName.endsWith("g00"))) {
            emit newStdChannelData("--- skipping: ."+ folder + fName);
         } else {
            emit newStdChannelData("--- extracting: ."+ folder + fName);
        }
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

void EngineProcess::reVersionIntern(const QString &engineVersion, const QString &gamsVersion, bool isInKubernetes)
{
    Q_UNUSED(engineVersion)
    mInKubernetes = isInKubernetes;
    mGamsVersion = gamsVersion;
}

void EngineProcess::interrupt()
{
    bool ok = !mManager->getJobToken().isEmpty();
    if (ok)
        emit mManager->syncKillJob(false);
    else
        AbstractGamsProcess::interrupt();
}

void EngineProcess::terminate()
{
    bool ok = !mManager->getJobToken().isEmpty();
    if (ok)
        emit mManager->syncKillJob(true);
    else
        AbstractGamsProcess::terminate();
    // ensure termination
    setProcState(ProcIdle);
    completed(-1);
}

void EngineProcess::terminateLocal()
{
    AbstractGamsProcess::terminate();
    setProcState(ProcIdle);
    completed(-1);
}

void EngineProcess::setParameters(const QStringList &parameters)
{
    if (parameters.size()) {
        mMainFile = parameters.first();
        if (mMainFile.startsWith('"') && mMainFile.endsWith('"'))
            mMainFile = mMainFile.mid(1, mMainFile.length()-2);
        mMainFile = QDir(workingDirectory()).relativeFilePath(mMainFile);
        if (mMainFile.startsWith("..")) {
            emit newStdChannelData("\nThe run file isn't located inside the working directory or it's subfolders.\n");
            mOutPath = "";
            return;
        }
        mModelName = QFileInfo(mMainFile).completeBaseName();
        QString tempName = workingDirectory() + "/" + modelName() + "-temp";
        int n = 0;
        QDir outDir(tempName);
        while (outDir.exists()) {
            outDir = QDir(tempName + QString::number(++n));
        }
        mOutPath = QDir::toNativeSeparators(outDir.path());
        mWorkPath = QDir::toNativeSeparators(workingDirectory());
    } else {
        mOutPath = QString();
    }
    AbstractProcess::setParameters(parameters);
}

void EngineProcess::forcePreviousWork()
{
    mForcePreviousWork = true;
}

void EngineProcess::setHasPreviousWorkOption(bool value)
{
    mHasPreviousWorkOption = value;
}

QProcess::ProcessState EngineProcess::state() const
{
    return (mProcState <= ProcIdle) ? QProcess::NotRunning : QProcess::Running;
}

bool EngineProcess::setUrl(const QString &url)
{
    QString scheme = "https";
    int sp1 = url.indexOf("://")+1;
    if (sp1 > 0) {
        scheme = url.left(sp1-1);
        sp1 += 2;
    }

    int sp2 = url.indexOf('/', sp1);
    if (sp2 < 0) sp2 = url.length();
    QString host = url.mid(sp1, sp2-sp1);
    int sp3 = host.indexOf(':');

    QString port = scheme.compare("https", Qt::CaseInsensitive)==0 ? "443" : "80";
    if (sp3 > 0) {
        port = host.right(host.length()-sp3-1);
        host = host.left(sp3);
    }

    QString basePath = url.right(url.length()-sp2);

    QString completeUrl = scheme+"://"+host+":"+port+basePath;
    mManager->setUrl(completeUrl);
    return mManager->url().isValid();
}

QUrl EngineProcess::url()
{
    return mManager->url();
}

void EngineProcess::authorize(const QString &username, const QString &password, int expireMinutes)
{
    mManager->authorize(username, password, expireMinutes);
    setProcState(ProcCheck);
}

void EngineProcess::initUsername(const QString &user)
{
    mManager->initUsername(user);
}

void EngineProcess::setAuthToken(const QString &bearerToken)
{
    mAuthToken = bearerToken;
    mManager->setAuthToken(bearerToken);
}

void EngineProcess::setNamespace(const QString &nSpace)
{
    mNamespace = nSpace;
}

void EngineProcess::setIgnoreSslErrorsCurrentUrl(bool ignore)
{
    mManager->setIgnoreSslErrorsCurrentUrl(ignore);
    if (mProcState == ProcCheck) {
        setProcState(ProcIdle);
    }
}

bool EngineProcess::isIgnoreSslErrors() const
{
    return mManager->isIgnoreSslErrors();
}

void EngineProcess::listJobs()
{
    mManager->listJobs();
}

void EngineProcess::listNamespaces()
{
    mManager->listNamespaces();
}

void EngineProcess::sendPostLoginRequests()
{
    mManager->listNamespaces();
    if (mInKubernetes) {
        mManager->getUserInstances();
        mManager->getQuota();
    }
}

void EngineProcess::getVersions()
{
    mManager->getVersion();
}

void EngineProcess::completed(int exitCode)
{
    disconnect(&mPollTimer, &QTimer::timeout, this, &EngineProcess::pollStatus);
    mPollTimer.stop();
    mManager->cleanup();
    setProcState(ProcIdle);
    mQueuedTimer.invalidate();
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

void EngineProcess::reCreateJob(const QString &message, const QString &token)
{
    Q_UNUSED(message)
    mQueuedTimer.start();
    mManager->setToken(token);
    emit newStdChannelData(QString("\n--- GAMS Engine at %1\n").arg(mManager->url().toString()).toUtf8());
    QString newLstEntry("--- switch LOG to %1-server.lst[LS2:\"%2\"]\nTOKEN: %3\n\n");
    QString lstPath = mWorkPath+"/"+modelName()+"-server.lst";
    emit newStdChannelData(newLstEntry.arg(modelName(), lstPath, token).toUtf8());
    // TODO(JM) store token for later resuming
    // monitoring starts automatically after successfull submission
    pollStatus();
}

const QHash<QString, EngineProcess::JobStatusEnum> EngineProcess::CJobStatus {
    {"invalid", jsInvalid}, {"done", jsDone}, {"running", jsRunning}, {"waiting", jsWaiting},
    {"Unknown Job", jsUnknownJob}, {"Bad Password", jsBadPassword}
};

void EngineProcess::reGetJobStatus(qint32 status, qint32 gamsExitCode)
{
    EngineManager::StatusCode engineStatus = EngineManager::StatusCode(status);

    if (engineStatus > EngineManager::Queued && mProcState == Proc3Queued) {
        setProcState(Proc4Monitor);
    }
    if (engineStatus == EngineManager::Finished && mProcState == Proc4Monitor) {
        mManager->getLog();
        if (gamsExitCode) {
            QByteArray code = QString::number(gamsExitCode).toLatin1();
            emit newStdChannelData("\nGAMS terminated with exit code " +code+ "\n");
        }
        setProcState(Proc5GetResult);
        mManager->getOutputFile();
    } else if (mProcState >= Proc3Queued) {
        jobIsQueued();
    }
}

void EngineProcess::reKillJob(const QString &text)
{
    emit newStdChannelData('\n'+text.toUtf8()+'\n');
    // TODO(JM) set completed HERE
}

void EngineProcess::reGetLog(const QByteArray &data)
{
    if (mQueuedTimer.isValid()) {
        emit newStdChannelData("\n");
        mQueuedTimer.invalidate();
    }
    QByteArray res = convertReferences(data);
    if (!res.isEmpty())
        emit newStdChannelData("[]"+res);
}

void EngineProcess::reQuota(const QList<gams::studio::engine::QuotaData *> data)
{
    QPair<QString, QList<int>> diskRemain("", {-1, -1});
    QPair<QString, QList<int>> volRemain("", {-1, -1});
    QPair<QString, QList<int>> parallel("", {-1, -1});

    for (QuotaData *q : data) {
        if (q->disk.remain() >= 0) {
            if (diskRemain.second.at(0) < 0 || diskRemain.second.at(0) >= q->disk.remain()) {
                if (diskRemain.second.at(0) > q->disk.remain()) diskRemain.first = "";
                diskRemain.first += (diskRemain.first.isEmpty() ? "" : " and ") + q->name;
                diskRemain.second[0] = q->disk.remain();
                diskRemain.second[1] = q->disk.max;
            }
        }
        if (q->volume.remain() >= 0) {
            if (volRemain.second.at(0) < 0 || volRemain.second.at(0) >= q->volume.remain()) {
                if (volRemain.second.at(0) > q->volume.remain()) volRemain.first = "";
                volRemain.first += (volRemain.first.isEmpty() ? "" : " and ") + q->name;
                volRemain.second[0] = q->volume.remain();
                volRemain.second[1] = q->volume.max;
            }
        }
//        if (q->parallel >= 0) {
//            if (parallel.second.at(0) < 0 || parallel.second.at(0) > q->volume.remain()) {
//                if (parallel.second.at(0) > q->volume.remain()) parallel.first = "";
//                parallel.first += (parallel.first.isEmpty() ? "" : " and ") + q->name;
//                parallel.second[0] = q->parallel;
//                parallel.second[1] = q->parallel;
//            }
//        }
    }
    QStringList availDisk;
    if (diskRemain.second.at(0) >= 0) {
        QString val;
        if (diskRemain.second.at(0) > 100000)
            val = QString::number(diskRemain.second.at(0) / 1000000, 'g', 2) + " MB";
        else
            val = QString::number(diskRemain.second.at(0) / 1000, 'g', 3) + " kb";
        availDisk << val;
        availDisk << diskRemain.first;
    }
    QStringList availVolume;
    if (volRemain.second.at(0) >= 0) {
        QString val;
        int h = (volRemain.second.at(0) / 3600);
        int m = ((volRemain.second.at(0) - (h*3600)) / 60);
        int s = int(volRemain.second.at(0)) - (h*3600) - (m*60);
        DEB() << "seconds " << volRemain.second.at(0) << "   h:" << h;
        if (h)
            val = QString::number(h) + (m>9 ? ":" : ":0") + QString::number(m) + " h";
        else
            val = QString::number(m) + (s>9 ? "." : ".0") + QString::number(s) + " min";
        availVolume << val;
        availVolume << volRemain.first;
    }
//    QStringList availParallel;
//    if (volRemain.second.at(0) >= 0) {
//        availParallel << QString::number(parallel.second.at(0));
//        availParallel << parallel.first;
//    }
    emit quotaHint(availDisk, availVolume);
}

void EngineProcess::reQuotaError(const QString &errorText)
{
    emit quotaHint(QStringList() << "unknown" << errorText, QStringList() << "" << "");
}

void EngineProcess::jobIsQueued()
{
    if (mQueuedTimer.isValid()) {
        int elapsed = int(mQueuedTimer.elapsed() / 1000L);
        if (elapsed > 1)
            emit newStdChannelData(("\r--- Job queued (" + QString::number(elapsed) + " sec)").toUtf8());
    }
}

void EngineProcess::reGetOutputFile(const QByteArray &data)
{
    disconnect(&mPollTimer, &QTimer::timeout, this, &EngineProcess::pollStatus);
    mPollTimer.stop();
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
    disconnect(&mPollTimer, &QTimer::timeout, this, &EngineProcess::pollStatus);
    mPollTimer.stop();
    emit newStdChannelData("\n"+errorText.toUtf8()+"\n");
    completed(-1);
}

void EngineProcess::reAuthorize(const QString &token)
{
    mManager->setAuthToken(token);
    if (!token.isEmpty()) {
        mManager->listJobs();
        mManager->listNamespaces();
        setProcState(ProcCheck);
    }
    emit authorized(token);
}

void EngineProcess::pollStatus()
{
    if (mProcState > Proc3Queued) mManager->getLog();
    mManager->getJobStatus();
    mPollTimer.start();
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

int indexOfEnd(const QByteArray &data, const QByteArray &pattern, const int start)
{
    int match = 0;
    for (int i = start; i < data.length(); ++i) {
        if (match == pattern.length()) return i;
        if (data.at(i) == pattern.at(match)) ++match;
        else match = 0;
    }
    return -1;
}

QByteArray EngineProcess::convertReferences(const QByteArray &data)
{
    int scanDirIndex = -1;
    int endOfParameters = (!mRemoteWorkDir.isEmpty() && !mInParameterBlock) ? 0 : data.length();
    if (mRemoteWorkDir.isEmpty()) {
        scanDirIndex = data.indexOf("--- GAMS Parameters defined");
        if (scanDirIndex >= 0) mInParameterBlock = true;
    }
    if (mInParameterBlock) {
        if (data.size())
            scanDirIndex = qMax(indexOfEnd(data, "    Input ", scanDirIndex),
                                indexOfEnd(data, "    Restart ", scanDirIndex));
        int end = 0;
        if (scanDirIndex >= 0) {
            end = scanDirIndex;
            for ( ; end < data.length(); ++end) {
                if (data.at(end) == '\n' || data.at(end) == '\r')
                    break;
            }
            mRemoteWorkDir = data.mid(scanDirIndex, end-scanDirIndex);
            mRemoteWorkDir = mRemoteWorkDir.left(qMax(mRemoteWorkDir.lastIndexOf('/'), mRemoteWorkDir.lastIndexOf('\\'))+1);
        }
        if (!mRemoteWorkDir.isEmpty()) {
            // remote working dir found, now find end of parameters
            while (mInParameterBlock && data.length() > end+5) {
                while (data.length() > end && (data.at(end) == '\n' || data.at(end) == '\r')) ++end;
                if (data.length() > end+4 && data.mid(end,4) == "    ") {
                    while (data.length() > end && (data.at(end) != '\n' && data.at(end) != '\r')) ++end;
                    continue;
                } else {
                    mInParameterBlock = false;
                    endOfParameters = end;
                    break;
                }
            }
        }
    }
    QByteArray res;
    res.reserve(data.size()+mOutPath.length());
    QByteArray lstTag("[LST:");
    int iRP = 0;
    int iLT = 0;
    int iCount = 0;  // count of chars currently not copied

    for (int i = 0; i < data.size(); ++i) {
        if (!mRemoteWorkDir.isEmpty() && i > endOfParameters) {
            if (iRP == mRemoteWorkDir.length()) {
                // add local path
                iRP = 0;
            }
            // Check if still in remotePath pattern
            if (data.at(i) >= '0' && data.at(i) <= '9') {
                if (mRemoteWorkDir.at(iRP) != '+') iRP = 0;
            } else {
                if (mRemoteWorkDir.at(iRP) == '+') ++iRP;
                if (iRP < mRemoteWorkDir.length()) {
                    if (mRemoteWorkDir.at(iRP) == data.at(i)) ++iRP;
                    else iRP = 0;
                }
            }
        }
        // Check if still in lstTag pattern
        if (lstTag.at(iLT) == data.at(i)) ++iLT;
        else iLT = 0;
        ++iCount;
        if (!mRemoteWorkDir.isEmpty() && iRP == mRemoteWorkDir.size()) {
            res.append(mWorkPath.toUtf8());
            res.append(QDir::separator().toLatin1());
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
    // moves created (necessary) files into zip transfer file
    GmszipProcess *subProc = new GmszipProcess(this);
    connect(subProc, QOverload<int, QProcess::ExitStatus>::of(&GmszipProcess::finished), this, &EngineProcess::packCompleted);
    connect(subProc, &GmsunzipProcess::newStdChannelData, this, &EngineProcess::parseUnZipStdOut);
    connect(subProc, &GmsunzipProcess::newProcessCall, this, &EngineProcess::newProcessCall);

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
    file.setFileName(mWorkPath + '/' + modelName() + ".g00");
    if (!file.rename(mOutPath+'/'+baseName+".g00")) {
        emit newStdChannelData("\n*** Can't move file to subdirectory: "+file.fileName().toUtf8()+'\n');
        completed(-1);
        return;
    }

    mSubProc = subProc;
    subProc->setWorkingDirectory(mOutPath);
    QStringList params;
    params << "-8"<< "-m" << baseName+".zip" << baseName+".gms" << baseName+".g00";
    subProc->setParameters(params);
    subProc->execute();
}

void EngineProcess::startPacking2()
{
    QStringList params;
    QString baseName = modelName();
    params << "-8"<< QDir(mOutPath).dirName()+"/"+baseName+".zip";
    if (!addFilenames(mWorkPath + '/' + modelName() + ".efi", params)) {
        pack2Completed(0, QProcess::ExitStatus::NormalExit);
        return;
    }

    // copies additional files (from *.efi) into zip transfer file
    GmszipProcess *subProc = new GmszipProcess(this);
    connect(subProc, QOverload<int, QProcess::ExitStatus>::of(&GmszipProcess::finished), this, &EngineProcess::pack2Completed);
    connect(subProc, &GmsunzipProcess::newStdChannelData, this, &EngineProcess::parseUnZipStdOut);
    connect(subProc, &GmsunzipProcess::newProcessCall, this, &EngineProcess::newProcessCall);

    mSubProc = subProc;
    subProc->setWorkingDirectory(workingDirectory());
    subProc->setParameters(params);
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

void EngineProcess::handleResultFiles()
{
    QString model = modelName();
    QDir srcDir(QDir::fromNativeSeparators(mOutPath), "");
    QDir destDir(QDir::fromNativeSeparators(mWorkPath));
    mkDirsAndMoveFiles(srcDir, destDir, true);
}

void EngineProcess::mkDirsAndMoveFiles(const QDir &srcDir, const QDir &destDir, bool inBase)
{
    moveFiles(srcDir, destDir, inBase);

    for (QFileInfo fi : srcDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString subDir = fi.fileName();
        destDir.mkdir(subDir);
        QDir destSub = QDir(destDir.filePath(subDir));
        if (destSub.exists()) {
            QDir srcSub = QDir(fi.filePath());
            if (!srcSub.exists()) {
                emit newStdChannelData("\n*** Can't find directory: "+fi.filePath().toUtf8()+"_\n");
            }
            mkDirsAndMoveFiles(srcSub, destSub);
        } else {
            emit newStdChannelData("\n*** Can't create directory: "+destSub.path().toUtf8()+"_\n");
        }
    }
    if (srcDir.isEmpty()) {
        QDir parDir(srcDir.path());
        parDir.cdUp();
        parDir.rmdir(srcDir.dirName());
    }
}

void EngineProcess::moveFiles(const QDir &srcDir, const QDir &destDir, bool inBase)
{
    QDir workDir(mWorkPath);
    for (QFileInfo srcFi : srcDir.entryInfoList(QDir::Files)) {
        QFile srcFile(srcFi.filePath());
        QString destFileName;
        Qt::CaseSensitivity cs = FileType::fsCaseSense();
        if (inBase && (srcFi.fileName().compare("solver.log", cs) == 0)) {
            destFileName = modelName() + '-' + srcFi.fileName();
        } else if (inBase && (srcFi.fileName().compare("solver-output.zip", cs) == 0 ||
                  (srcFi.completeBaseName().compare(modelName(), cs) == 0 && srcFi.suffix().compare("zip", cs) == 0))) {
            QFile delFil(srcFi.filePath());
            delFil.remove();
            continue;
        } else if (inBase && srcFi.completeBaseName().compare(modelName(), cs) == 0 &&
                   (srcFi.suffix().compare("lst", cs) == 0 || srcFi.suffix().compare("lxi", cs) == 0)) {
            destFileName = modelName() + "-server" + '.' + srcFi.suffix();
        } else {
            destFileName = srcFi.fileName();
        }
        QFile destFile(destDir.filePath(destFileName));
        if (srcFi.suffix().compare("gdx", cs) == 0)
            emit releaseGdxFile(destFile.fileName());

        QFile bk(destFile.fileName()+"_");
        QFileInfo destFi(destFile);
        if (destFile.exists()) {
            if (mProtectedFiles.contains(destFi)) {
                emit newStdChannelData("*** Skip updating "+destFile.fileName().toUtf8()+"\n");
                srcFile.remove();
                continue;
            }
            if (bk.exists()) {
                if (!bk.remove()) {
                    emit newStdChannelData("*** Can't remove backup file: "+bk.fileName().toUtf8()+"\n");
                    continue;
                }
            }
            QFile destFileCurrent(destDir.filePath(destFileName));
            if (!destFileCurrent.rename(bk.fileName())) {
                emit newStdChannelData("*** Can't rename file: "+destFileCurrent.fileName().toUtf8()+" to "+bk.fileName().toUtf8()+"\n");
                continue;
            }
        }
        if (!srcFile.rename(destFile.fileName())) {
            emit newStdChannelData("*** Can't move file: "+srcFile.fileName().toUtf8()+" to "+destFile.fileName().toUtf8()+'\n');
            continue;
        } else {
            QString link = destFi.filePath();
            if (destFi.suffix().toLower() == "lxi")
                link = destFi.path() + QDir::separator() + destFi.completeBaseName() + ".lst";
            emit newStdChannelData("*** Local file updated: " + workDir.relativeFilePath(destFile.fileName()).toUtf8()
                                   + "[FIL:\"" + link.toUtf8()+"\",0,0]\n");
        }
        if (bk.exists()) bk.remove();
        if (srcFi.suffix().compare("gdx", cs) == 0)
            emit reloadGdxFile(destFile.fileName());
    }
}

QString EngineProcess::modelName() const
{
    return mModelName;
}

bool EngineProcess::addFilenames(const QString &efiFile, QStringList &list)
{
    int listSize = list.size();
    QFile file(efiFile);
    if (!file.exists()) return false;
    if (!file.open(QFile::ReadOnly | QIODevice::Text)) {
        emit newStdChannelData("*** Can't read file: "+file.fileName().toUtf8()+'\n');
        return false;
    }
    mProtectedFiles.clear();
    QTextStream in(&file);
    QString path = QFileInfo(file).path();
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;

        bool writeBack = false;
        if (line.endsWith(" <")) {
            line = line.left(line.length() - 1).trimmed();
            writeBack = true;
        }
        if (QDir(workingDirectory()).relativeFilePath(line).compare(mMainFile) == 0)
            continue;
        QFileInfo fi(line);
        if (fi.isAbsolute()) {
            if (fi.exists()) {
                list << line;
                if (!writeBack) mProtectedFiles << fi;
            }
        } else if (QFile::exists(path+"/"+line)) {
            list << line;
            if (!writeBack) mProtectedFiles << QFileInfo(path+"/"+line);
        } else {
            emit newStdChannelData("*** Can't add file: "+line.toUtf8()+'\n');
        }
    }
    file.close();
    return list.size() > listSize;
}

void EngineProcess::setSelectedInstance(const QString &selectedInstance)
{
    mUserInstance = selectedInstance;
}

bool EngineProcess::inKubernetes() const
{
    return mInKubernetes;
}

bool EngineProcess::forceGdx() const
{
    return mForceGdx;
}

void EngineProcess::setForceGdx(bool forceGdx)
{
    mForceGdx = forceGdx;
}

void EngineProcess::abortRequests()
{
    mManager->abortRequests();
}

} // namespace engine
} // namespace studio
} // namespace gams
