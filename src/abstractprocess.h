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

#include "file/projectabstractnode.h"

namespace gams {
namespace studio {

class ProjectGroupNode;

class AbstractProcess
        : public QObject
{
    Q_OBJECT

protected:
    AbstractProcess(const QString &app, QObject *parent = Q_NULLPTR);
    virtual ~AbstractProcess() {}

public:
    QString app() const;

    void setInputFile(const QString &file);
    QString inputFile() const;

    virtual void execute() = 0;
    QProcess::ProcessState state() const;

    FileId groupId() const;
    void setGroupId(const FileId &groupId);

signals:
    void finished(AbstractProcess *process, int exitCode);
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
    FileId mGroupId = -1;
    QString mInputFile;
    QProcess mProcess;
    QMutex mOutputMutex;

private:
    QString mApp;
};

} // namespace studio
} // namespace gams

#endif // ABSTRACTPROCESS_H
