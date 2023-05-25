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

class QTcpServer;
class QTcpSocket;

namespace gams {
namespace studio {
namespace debugger {

enum Call {
    invalid,
    invalidReply,
    writeGDX,
    addBP,
    delBP,
    addBPs,
    clearBPs,
    interrupt,
};

enum Reply {
    invalid,
    invalidCall,
    breakAt,
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
    inline QString toString() {
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
    void signalReady();
    void signalInterruptedAt(const QString &file, int lineNr);

public slots:
    void addBreakpoint(const Breakpoint &bp);
    void addBreakpoints(const QList<Breakpoint> &bps);
    void removeBreakpoint(const Breakpoint &bp);
    void clearBreakpoints();
    void sendInterrupt();

private slots:
    void newConnection();

private:
    void init();
    void sortBreakpoints();
    void logMessage(const QString &message);
    void deleteSocket();
    void callProcedure(Call call, const QStringList &arguments = QStringList());
    bool handleReply(const QString &replyData);

    QTcpServer *mServer = nullptr;
    QTcpSocket* mSocket = nullptr;
    QHash<Call, QString> mCalls;
    QHash<QString, Reply> mReplies;

    QList<Breakpoint> mBreakpoints;

    static QSet<int> mPortsInUse;

};

} // namespace debugger
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_DEBUGGER_SERVER_H
