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

    // configuring Call (Server -> GAMS)
    invalidReply,
    addBP,          //  addBP \n file:line[:line] [\n file:line[:line]]  (shortstring = 255 characters) (relative to workdir)
    delBP,          //  delBP \n [file[:line]]

    // action Call (Server -> GAMS)
    run,
    stepLine,
    interrupt,
    writeGDX,

    // Reply (GAMS -> Server)
    invalidCall,
    paused,         // paused \n file:line
    gdxReady,       // gdxReady \n file
    finished,
};
///
/// \brief The Server class allows debug communication to a GAMS instance
/// \details
/// The Server class listens to a port to allow GAMS to connect. When the connection is established, the Server can
/// send several kind of <b>Call</b> to GAMS and accepts some kinds of <b>Reply</b>.
///
/// The Data is formatted in a multi-line string:
/// - the first line is the keyword of the Call/Reply
/// - the data follows in subsequent lines, one line for each data set
/// - a data set is split by the colon ':'
///
/// example to tell GAMS to add a breakpoint for trnsport at line 46, Studio sends:
///
/// addBP
/// <path>/trnsport.gms:46
///
class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(const QString &path, QObject *parent = nullptr);
    ~Server() override;
    bool isListening();
    quint16 port();
    bool start();
    void stop();
    QString gdxTempFile() const;

signals:
    void addProcessData(const QByteArray &data);
    void signalGdxReady(const QString &gdxFile);
    void signalPaused(const QString &file, int lineNr);

public slots:
    void addBreakpoint(const QString &filename, int line);
    void addBreakpoints(const QHash<QString, QSet<int> > &breakpoints);
    void delBreakpoint(const QString &filename, int line);
    void clearBreakpoints(const QString file = QString());

    void sendRun();
    void sendStepLine();
    void sendInterrupt();
    void sendWriteGdx(const QString &gdxFile);

private slots:
    void newConnection();

private:
    void init();
    void logMessage(const QString &message);
    void deleteSocket();
    void callProcedure(CallReply call, const QStringList &arguments = QStringList());
    bool handleReply(const QString &replyData);
    QString toBpString(const QString &file, QSet<int> lines);

    QString mPath;
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
