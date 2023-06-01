/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMS_STUDIO_DEBUGGER_SERVER_H
#define GAMS_STUDIO_DEBUGGER_SERVER_H

#include <QObject>
#include <QList>
#include <QHash>

class QTcpServer;
class QTcpSocket;

namespace gams {
namespace studio {
namespace debugger {

enum CallReply {
    invalid,
    invalidReply,
    writeGDX,
    addBP,
    delBP,
    addBPs,
    clearBPs,
    interrupt,

    invalidCall,
    paused,
    gdxReady,
    finished,
};

struct Breakpoint
{
    Breakpoint() {}
    Breakpoint(const QString _file, int _line, int _index = 0) : file(_file), line(_line), index(_index) {}
    inline bool operator ==(const Breakpoint &other) const {
        return file.compare(other.file) == 0 && line == other.line && index == other.index;
    }
    inline bool operator !=(const Breakpoint &other) const {
        return *this == other;
    }
    inline QString toString() const {
        return file + ':' + QString::number(line) + ':' + QString::number(index);
    }

    QString file;
    int line = 0;
    int index = 0;
};

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    ~Server() override;
    bool isListening();
    quint16 port();
    bool start();
    void stop();

signals:
    void addProcessData(const QByteArray &data);
    void signalGdxReady(const QString &gdxFile);
    void signalPaused(const QString &file, int lineNr);

public slots:
    void addBreakpoint(const QString &filename, int line);
    void addBreakpoints(const QHash<QString, QSet<int> > &breakpoints);
    void delBreakpoint(const QString &filename, int line);
    void clearBreakpoints(const QString file = QString());
    void sendInterrupt();

private slots:
    void newConnection();

private:
    void init();
    void logMessage(const QString &message);
    void deleteSocket();
    void callProcedure(CallReply call, const QStringList &arguments = QStringList());
    bool handleReply(const QString &replyData);
    QString toBpString(const QString &file, QSet<int> lines);

    QTcpServer *mServer = nullptr;
    QTcpSocket *mSocket = nullptr;
    QHash<CallReply, QString> mCalls;
    QHash<QString, CallReply> mReplies;

    static QSet<int> mPortsInUse;

};

} // namespace debugger
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_DEBUGGER_SERVER_H
