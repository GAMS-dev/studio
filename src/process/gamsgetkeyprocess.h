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
#ifndef GAMSGETKEYPROCESS_H
#define GAMSGETKEYPROCESS_H

#include <QProcess>
#include <QMutex>

namespace gams {
namespace studio {

class GamsGetKeyProcess final
        : public QObject
{
    Q_OBJECT

public:
    GamsGetKeyProcess(QObject *parent = nullptr);

    ~GamsGetKeyProcess();

    QString alpId() const;
    void setAlpId(const QString& id);

    QString checkoutDuration() const;
    void setCheckoutDuration(const QString& duration);

    QString onPremSever() const;
    void setOnPremSever(const QString& address);

    QString onPremCertPath() const;
    void setOnPremCertPath(const QString &newOnPremCertPath);

    bool verboseOutput() const;
    void setVerboseOutput(bool enable);

    void execute();

    QString content() const;

    QString logMessages() const;

    void clearState();

    void writeLogToFile(const QString &data);

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
    QString mAlpId;
    QString mCheckoutDuration;
    QString mOnPremSever;
    QString mOnPremCertPath;
    QStringList mLogMessages;
    QStringList mContent;
    bool mVerboseOutput;
    QMutex mOutputMutex;
};

}
}

#endif // GAMSGETKEYPROCESS_H
