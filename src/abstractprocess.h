/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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

#include "common.h"

namespace gams {
namespace studio {

class AbstractProcess : public QObject
{
    Q_OBJECT

protected:
    AbstractProcess(const QString &appName, QObject *parent = nullptr);

public:
    virtual ~AbstractProcess() {}

    void setInputFile(const QString &file);
    QString inputFile() const;

    virtual void execute() = 0;
    virtual void interrupt();
    virtual void terminate();

    virtual QProcess::ProcessState state() const = 0;

    QString application() const {
        return mApplication;
    }

    QStringList parameters() const;
    void setParameters(const QStringList &parameters);

    virtual QStringList defaultParameters() const { // TODO (AF) review/needed?
        return QStringList();
    }

    QString workingDirectory() const;
    void setWorkingDirectory(const QString &workingDirectory);

    NodeId groupId() const;
    void setGroupId(const NodeId &groupId);

    int exitCode() const;

signals:
    void finished(NodeId origin, int exitCode);
    void newStdChannelData(const QByteArray &data);
    void stateChanged(QProcess::ProcessState newState);
    void newProcessCall(const QString &text, const QString &call);

protected slots:
    virtual void completed(int exitCode);
    virtual void readStdOut() = 0;
    virtual void readStdErr() = 0;

protected:
    virtual QString nativeAppPath();

    inline QString appCall(const QString &app, const QStringList &args) {
        return app + " " + args.join(" ");
    }

protected:
    NodeId mGroupId = NodeId();
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
    void readStdChannel(QProcess::ProcessChannel channel);

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
