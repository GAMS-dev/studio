/**
 * GAMS Studio
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
#ifndef ABSTRACTPROCESS_H
#define ABSTRACTPROCESS_H

#include <QObject>
#include <QProcess>
#include <QMutex>

#include "../common.h"

namespace gams {
namespace studio {

class AbstractProcess : public QObject
{
    Q_OBJECT

protected:
    AbstractProcess(const QString &appName, QObject *parent = nullptr);

public:
    enum TerminateOption { termLocal, termRemote, termIgnored };

    ~AbstractProcess() override {}

    void setInputFile(const QString &file);
    QString inputFile() const;

    virtual void execute() = 0;
    virtual void interrupt();
    virtual void terminate();
    virtual void terminateLocal() { terminate(); }
    virtual TerminateOption terminateOption() { return termLocal; }

    virtual QProcess::ProcessState state() const = 0;

    QString application() const {
        return mApplication;
    }

    const QStringList parameters() const;
    virtual void setParameters(const QStringList &parameters);

    virtual QStringList defaultParameters() const {
        return QStringList();
    }

    QString workingDirectory() const;
    void setWorkingDirectory(const QString &workingDirectory);

    NodeId projectId() const;
    void setProjectId(const NodeId &projectId);

    int exitCode() const;

signals:
    void finished(int nodeId, int exitCode); // translated NodeId to int and back to get it through queued connection
    void newStdChannelData(const QByteArray &data);
    void stateChanged(QProcess::ProcessState newState);
    void newProcessCall(const QString &text, const QString &call);
    void interruptGenerated();

protected slots:
    virtual void completed(int exitCode);
    virtual void readStdOut() = 0;
    virtual void readStdErr() = 0;

protected:
    virtual QString nativeAppPath();
    inline QString appCall(const QString &app, const QStringList &args) {
        return app + " " + args.join(" ");
    }
    void interruptIntern(bool hardKill = false);

protected:
    NodeId mProjectId = NodeId();
    QProcess mProcess;
    QMutex mOutputMutex;

private:
    QString mApplication;
    QString mInputFile;
    QString mWorkingDirectory;
    QStringList mParameters;
};

class AbstractSingleProcess : public AbstractProcess
{
    Q_OBJECT

public:
    AbstractSingleProcess(const QString &application, QObject *parent = nullptr);

    QProcess::ProcessState state() const override;

protected:
    virtual void readStdChannel(QProcess::ProcessChannel channel);

protected slots:
    void readStdOut() override;
    void readStdErr() override;
};

class AbstractGamsProcess : public AbstractSingleProcess
{
    Q_OBJECT

public:
    AbstractGamsProcess(const QString &application, QObject *parent = nullptr);

protected:
    QString nativeAppPath() override;
};

} // namespace studio
} // namespace gams

#endif // ABSTRACTPROCESS_H
