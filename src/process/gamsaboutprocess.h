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
#ifndef GAMSABOUTPROCESS_H
#define GAMSABOUTPROCESS_H

#include <QProcess>
#include <QMutex>

namespace gams {
namespace studio {

class GamsAboutProcess final
        : public QObject
{
    Q_OBJECT

public:
    GamsAboutProcess(QObject *parent = nullptr);

    ~GamsAboutProcess();

    void execute();

    QString content() const;

    QString logMessages() const;

    void clearState();

signals:
    void finished(int exitCode);

private slots:
    void readStdOut();
    void readStdErr();

private:
    QString application() const;
    bool isAppAvailable(const QString &app);
    void readStdChannel(QProcess::ProcessChannel channel);
    QString nativeAppPath();

private:
    QString mApplication;
    QProcess mProcess;
    QStringList mLogMessages;
    QStringList mContent;
    QMutex mOutputMutex;
};

}
}

#endif // GAMSABOUTPROCESS_H
