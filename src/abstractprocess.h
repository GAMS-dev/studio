/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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

class AbstractProcess
        : public QObject
{
    Q_OBJECT

protected:
    AbstractProcess(const QString &appName, QObject *parent = Q_NULLPTR);
    virtual ~AbstractProcess() {}

public:
    void setInputFile(const QString &file);
    QString inputFile() const;

    virtual void execute() = 0;
    QProcess::ProcessState state() const;

    NodeId groupId() const;
    void setGroupId(const NodeId &groupId);

    int exitCode() const;

signals:
    void finished(NodeId origin, int exitCode);
    void newStdChannelData(const QByteArray &data);
    void stateChanged(QProcess::ProcessState newState);

protected slots:
    void completed(int exitCode);
    void readStdOut();
    void readStdErr();
    void readStdChannel(QProcess::ProcessChannel channel);

protected:
    QString nativeAppPath();

protected:
    NodeId mGroupId = NodeId();
    QProcess mProcess;
    QMutex mOutputMutex;

private:
    QString mAppName;
    QString mInputFile;
};

} // namespace studio
} // namespace gams

#endif // ABSTRACTPROCESS_H
