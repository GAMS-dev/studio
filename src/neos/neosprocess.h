/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMS_STUDIO_NEOS_NEOSPROCESS_H
#define GAMS_STUDIO_NEOS_NEOSPROCESS_H

#include "process.h"
#include <QTimer>

namespace gams {
namespace studio {
namespace neos {

enum Priority {
    prioShort,
    prioLong
};

class NeosManager;

/// \brief The NeosProcess controls all steps to run a job on NEOS
/// This class works in four process steps:
/// 1. compile gms on the local machine
/// 2. monitor the remote job
/// 3. get result file
/// 4. unpack result file and finish
class NeosProcess final : public AbstractGamsProcess
{
    Q_OBJECT
public:
    NeosProcess(QObject *parent = nullptr);
    ~NeosProcess() override;
    void setPriority(Priority prio) { mPrio = prio; }
    void setForceGdx(bool forceGdx);
    void setMail(const QString &eMail) { mMail = eMail; }
    QString mail() { return mMail; }

    void execute() override;
    void interrupt() override;
    void terminate() override;
    void terminateLocal() override;
    TerminateOption terminateOption() override { return termIgnored; }

    void setParameters(const QStringList &parameters) override;
    QProcess::ProcessState state() const override;
    void validate();
    void setIgnoreSslErrors();
    void setStarting();

signals:
    void procStateChanged(gams::studio::AbstractProcess *proc, gams::studio::ProcState progress);
    void requestAcceptSslErrors();
    void sslValidation(QString errorMessage);


protected slots:
    void completed(int exitCode) override;
    void rePing(const QString &value);
    void reVersion(const QString &value);
    void reSubmitJob(const int &jobNumber, const QString &jobPassword);
    void reGetJobStatus(const QString &status);
    void reGetCompletionCode(const QString &code);
    void reGetJobInfo(const QStringList &info);
    void reKillJob(const QString &text);
    void reGetIntermediateResultsNonBlocking(const QByteArray &data);
    void reGetFinalResultsNonBlocking(const QByteArray &data);
    void reGetOutputFile(const QByteArray &data);
    void reError(const QString &errorText);

private slots:
    void pullStatus();
    void compileCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void unpackCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void sslErrors(const QStringList &errors);
    void parseUnzipStdOut(const QByteArray &data);
    void unzipStateChanged(QProcess::ProcessState newState);

private:
    void setProcState(ProcState newState);
    QStringList compileParameters();
    QStringList remoteParameters();
    void solverToCategories(QStringList &params);
    void cleanCategories(const QStringList &params, QStringList &catParams);
    QStringList allCat4Solver(QString solver);
    QByteArray convertReferences(const QByteArray &data);
    void startUnpacking();

    NeosManager *mManager;
    bool mForceGdx = false;
    bool mHasGdx = false;
    bool mInLogClone = false;
    QString mMail;
    QString mOutPath;
    QString mJobNumber;
    QString mJobPassword;
    Priority mPrio;
    ProcState mProcState;
    QTimer mPullTimer;
    QProcess::ProcessState mPreparationState;

    AbstractGamsProcess *mSubProc = nullptr;
};

} // namespace neos
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NEOS_NEOSPROCESS_H
