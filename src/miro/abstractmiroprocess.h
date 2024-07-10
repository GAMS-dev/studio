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
#ifndef ABSTRACTMIROPROCESS_H
#define ABSTRACTMIROPROCESS_H

#include "process/abstractprocess.h"

namespace gams {
namespace studio {
namespace miro {

class AbstractMiroProcess : public AbstractProcess
{
    Q_OBJECT

public:
    AbstractMiroProcess(const QString &application, QObject *parent = nullptr);

    void execute() override;
    void interrupt() override;
    void terminate() override;

    QProcess::ProcessState state() const override;

    void setMiroPath(const QString &miroPath);

    QString modelName() const;
    void setModelName(const QString &modelFile);

    QString modelPath() const;

signals:
    void executeMiro();

protected slots:
    void readStdOut() override;
    void readStdErr() override;
    void completed(int exitCode) override;
    virtual void subProcessCompleted(int exitCode);
    void executeNext();

protected:
    QString nativeAppPath() override;

    virtual QProcessEnvironment miroProcessEnvironment() = 0;

    void readStdChannel(QProcess &process, QProcess::ProcessChannel channel);

private:
    void gamsInterrupt();

private:
    QString mMiroPath;
    QString mModelName;
    QProcess mMiro;
};

}
}
}

#endif // ABSTRACTMIROPROCESS_H
