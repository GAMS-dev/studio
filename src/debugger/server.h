/*
 * This file is part of the GAMS Studio project.
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

enum DebugStartMode {
    NoDebug,
    RunDebug,
    StepDebug,
};

enum DebugState {
    None,
    Prepare,
    Running,
    Paused,
    Finished
};

enum CallReply {
    invalid,

    // configuring Call (Server -> GAMS)
    invalidReply,   //  invalidReply \n -the-invalid-reply-
//    getLinesMap,    //  getLinesMap
    addBP,          //  addBP \n contLN[|contLN] (shortstring = 255 characters)
    delBP,          //  delBP \n [contLN]

    // action Call (Server -> GAMS)
    run,            //  run
    stepLine,       //  stepLine
    pause,          //  pause
    writeGDX,       //  writeGDX \n file (contains port of comunication)

    // Reply (GAMS -> Server)
    invalidCall,    //  invalidCall \n -the-invalid-call-
    linesMap,       //  linesMap \n file|line=contLN[|line=contLN] [\n file|line=contLN[|line=contLN]]
                    //   (where contLN is the internal continous-line-number that CMEX is working with.)
                    //   (sends all lines known by CMEX, packages can be split before '|'. Until we haven't another
                    //    Reply with multiple lines, we omit the repeat of "breakLines" keyword)
    linesMapDone,   //  linesMapDone (when all breakLines have been sent)
    paused,         //  paused \n contLN
    gdxReady,       //  gdxReady \n file
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
/// - a data set is split by the pipe '|'
/// - an assignment uses '=' (e.g. in breakLines reply)
///
/// The initialization handshake procedure:
/// 1. Studio: calls GAMS with the debugPort parameter
/// 2. GAMS  : connects the socket
/// 3. Studio: accepts the socket
/// 4. GAMS  : sends the "linesMap" (probably multiple packets)
/// 5. GAMS  : sends "linesMapDone" when done
/// 6. Studio: sends "addBP" to send the breakpoints (optional)
/// 7. Studio: sends "run" or "stepLine"
///
/// example to tell GAMS to add a breakpoint for trnsport at line 46, which is also the continuous line 46, Studio sends:
///
/// addBP
/// 46
///
class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(const QString &path, QObject *parent = nullptr);
    ~Server() override;
    DebugState state() const;
    bool isListening();
    quint16 port();
    bool start();
    QString gdxTempFile() const;

signals:
    void connected();
    void addProcessLog(const QByteArray &data);
    void signalLinesMap(const QString &file, const QList<int> &fileLines, const QList<int> &continuousLines);
    void signalMapDone();
    void signalGdxReady(const QString &gdxFile);
    void signalPaused(int contLine);
    void signalStop();
    void stateChanged(DebugState state);

public slots:
    void addBreakpoint(int contLine);
    void addBreakpoints(const QList<int> &contLines);
    void delBreakpoint(int contLine);
    void clearBreakpoints();

    void sendRun();
    void sendStepLine();
    void sendPause();
    void sendWriteGdx(const QString &gdxFile);

    void stopAndDelete();

private slots:
    void newConnection();

private:
    void init();
    void logMessage(const QString &message);
    void deleteSocket();
    void callProcedure(CallReply call, const QStringList &arguments = QStringList());
    bool handleReply(const QString &replyData);
    QString toBpString(const QList<int> &lines);
    void parseLinesMap(const QString &breakData);
    void setState(DebugState state);

    QString mPath;
    QTcpServer *mServer = nullptr;
    QTcpSocket *mSocket = nullptr;
    QHash<CallReply, QString> mCalls;
    QHash<QString, CallReply> mReplies;
    QString mBreakLinesFile;
    DebugState mState = None;
    int mDelayCounter = 0;

    static QSet<int> mPortsInUse;

};

} // namespace debugger
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_DEBUGGER_SERVER_H
