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
#include "server.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QCollator>
#include <QFile>
#include <QDir>
#include <QTimer>

#include "logger.h"

namespace gams {
namespace studio {
namespace debugger {

const quint16 CFirstDebugPort = 3563;
const quint16 CLastDebugPort = 4563;
QSet<int> Server::mPortsInUse;

Server::Server(const QString &path, QObject *parent) : QObject(parent), mPath(path)
{
    init();
    mServer = new QTcpServer(this);
//    QTcpSocket::ReuseAddressHint
    connect(mServer, &QTcpServer::newConnection, this, &Server::newConnection, Qt::UniqueConnection);
}

Server::~Server()
{
    stopAndDelete();
    mServer = nullptr;
}

void Server::init()
{
    mCalls.insert(invalidReply, "invalidReply");
    mCalls.insert(addBP, "addBP");
    mCalls.insert(delBP, "delBP");

    mCalls.insert(run, "run");
    mCalls.insert(stepLine, "stepLine");
    mCalls.insert(pause, "pause");
    mCalls.insert(writeGDX, "writeGDX");

    mReplies.insert("invalidCall", invalidCall);
    mReplies.insert("linesMap", linesMap);
    mReplies.insert("linesMapDone", linesMapDone);
    mReplies.insert("paused", paused);
    mReplies.insert("gdxReady", gdxReady);
}

bool Server::start()
{
    if (mServer->isListening()) {
        logMessage("WARNING: The Debug-Server already listens to port" + QString::number(mServer->serverPort()));
        return true;
    }

    if (mPortsInUse.size() > 10) return false;

    quint16 port = CFirstDebugPort;
    while (mPortsInUse.contains(port) || !mServer->listen(QHostAddress::LocalHost, port)) {
        ++port;
        if (port > CLastDebugPort) {
            logMessage("Debug-Server could not find a free port!");
            return false;
        }
    }
    mPortsInUse << mServer->serverPort();

    logMessage("Debug-Server started. Listening at port " + QString::number(port));
    return true;
}

void Server::stopAndDelete()
{
    setState(Finished);
    deleteSocket();
    if (mPortsInUse.contains(mServer->serverPort()))
        mPortsInUse.remove(mServer->serverPort());
    if (isListening())
        mServer->close();
    mServer->deleteLater();
}

void Server::newConnection()
{
    if (!mServer) return;
    QTcpSocket *socket = mServer->nextPendingConnection();
    if (mSocket) {
        logMessage("Debug-Server: already connected to a socket");
        if (socket) socket->deleteLater();
        return;
    }
    if (!socket) return;
    mSocket = socket;
    connect(socket, &QTcpSocket::disconnected, this, [this]() {
        logMessage("Debug-Server: Socket disconnected");
        deleteSocket();
    });
    connect(socket, &QTcpSocket::readyRead, this, [this]() {
        QByteArray data = mSocket->readAll();
        QList<QByteArray> packets = data.split('\0');
        for (const QByteArray &packet : packets) {
            if (packet.isEmpty()) continue;
            if (mIncompletePacket.isEmpty())
                mIncompletePacket = packet;
            else
                mIncompletePacket += packet;
            if (mIncompletePacket.at(mIncompletePacket.size() - 1) != '\n')
                return;

            if (!handleReply(mIncompletePacket)) {
                if (!mIncompletePacket.contains("invalidCall"))
                    callProcedure(invalidReply, QStringList() << mIncompletePacket);
            }
            mIncompletePacket.clear();
        }
    });
    logMessage("Debug-Server: Socket connected to GAMS");
    setState(Prepare);
    emit connected();
}

QString Server::gdxTempFile() const
{
    return mPath + "/temp" + QString::number(mServer->serverPort()) + ".gdx";
}

void Server::setVerbose(bool verbose)
{
    mVerbose = verbose;
}

void Server::callProcedure(CallReply call, const QStringList &arguments)
{
    if (!mSocket || !mSocket->isOpen()) {
        QString additionals = arguments.count() > 1 ? QString(" (and %1 more)").arg(arguments.count()-1) : QString();
        logMessage("Debug-Server: Socket not open, can't process '" + mCalls.value(call, "undefinedCall")
                   + (arguments.isEmpty() ? "'" : (":" + arguments.at(0) + "'" + additionals)));
        return;
    }
    if (mVerbose)
        logMessage("Sending to GAMS: " + mCalls.value(call) + "\n" + arguments.join("\n"));
    QString keyword = mCalls.value(call);
    if (keyword.isEmpty()) {
        QString additionals = arguments.count() > 1 ? QString(" (and %1 more)").arg(arguments.count()-1) : QString();
        logMessage("Debug-Server: Undefined call '" + QString::number(int(call))
                   + (arguments.isEmpty() ? "'" : (":" + arguments.at(0) + "'" + additionals)));
        return;
    }
    QStringList remain(arguments);
    QString ready = keyword;
    bool progress = true;
    while (remain.length() || progress) {
        if (remain.length() && ready.length() + 2 + remain.first().length() < 255) {
            ready += '\n' + remain.takeFirst();
            progress = true;
        }
        if (!progress) {
            logMessage("Debug-Server: Call line too long. Can't send more than 255 characters at once '"
                       + QString::number(int(call)) + ":" + remain.first() + "'");
            break;
        }
        if (remain.isEmpty() || ready.length() + 2 + remain.first().length() >= 255) {
            mSocket->write(ready.toUtf8() + '\t');
            ready = keyword;
            progress = false;
        }
    }
}

bool Server::handleReply(const QString &replyData)
{
    if (mVerbose)
        logMessage("\nFrom GAMS: " + replyData);
    mIncompletePacket += replyData;
    QStringList reList = replyData.split('\n', Qt::SkipEmptyParts);
    CallReply reply = invalid;
    if (!reList.isEmpty()) {
        reply = mReplies.value(reList.at(0), invalid);
        if (reply == invalid && mLastReply == linesMap) {
            reply = linesMap;
        } else if (reply != invalid) {
            reList.removeFirst();
            mLastReply = reply;
        }
    }
    QStringList data;
    if (reList.size()) {
        data = reList.first().split('|');
    }

    bool ok = false;
    switch (reply) {
    case invalidCall:
        logMessage("Debug-Server: GAMS refused to process this request: " + reList.join(", "));
        break;
    case linesMap: {
        if (reList.size() < 1) {
            logMessage("Debug-Server: [linesMap] Missing data for breakable lines.");
            return false;
        }
        for (const QString &line : reList) {
            parseLinesMap(line);
        }
    }   break;
    case linesMapDone: {
        if (!mRemainData.isEmpty())
            parseLinesMap(" ");
        emit signalMapDone();
    }   break;
    case paused: {
        if (reList.size() < 1) {
            logMessage("Debug-Server: [paused] Missing continuous line for interrupt.");
            return false;
        }
        if (reList.size() > 1)
            logMessage("Debug-Server: [paused] Only one entry expected. Additional data ignored.");
        int line = data.at(0).toInt(&ok);
        if (!ok) {
            logMessage("Debug-Server: [paused] Can't parse continuous line number: " + data.at(0));
            return false;
        }

        setState(Paused);
        emit signalPaused(line);
    }   break;
    case gdxReady: {
        QString file;
        if (reList.size())
            file = QDir::fromNativeSeparators(data.first());

        if (file.isEmpty()) {
            logMessage("Debug-Server: [gdxReady] Missing name for GDX file.");
            return false;
        }
        if (!QFile::exists(file)) {
            logMessage("Debug-Server: [gdxReady] File not found: " + file);
            return false;
        }
        emit signalGdxReady(file);
    }   break;
    default:
        logMessage("Debug-Server: Unknown GAMS request: " + reList.join(", "));
        return false;
    }

    return true;
}

QString Server::toBpString(const QList<int> &lines)
{
    if (lines.isEmpty()) return QString();
    QString res;
    for (int line : lines) {
        if (!res.isEmpty())
            res += '|';
        res += QString::number(line);
    }
    return res;
}

void Server::parseLinesMap(const QString &breakData)
{
    if (breakData.isEmpty()) return;
    QStringList data;
    QChar c = breakData.at(0);
    if (mRemainData.isEmpty()) {
        data = breakData.split('|');
    } else if ((c < '0' || c > '9') && c != '=' && c != '|') {
        // This is probably a filename, then the remain data is complete and can be processed
        QStringList linePair = mRemainData.split("=");
        QList<int> lines;
        QList<int> coLNs;
        if (getPair(mRemainData, lines, coLNs))
            emit signalLinesMap(mBreakLinesFile, lines, coLNs);
        else {
            logMessage("Debug-Server: breakLines parse error: " + mRemainData);
        }
        if (mVerbose) {
            QString list("DATA-> " + mBreakLinesFile + " : ");
            for (int i = 0; i < lines.size(); ++i) {
                list += "|" + QString::number(lines.at(i)) + ":" + QString::number(coLNs.at(i));
            }
            logMessage(list);
        }
        mRemainData = QString();
        if (breakData == " ") return;
        data = breakData.split('|');
    } else {
        data = ("|" + mRemainData + breakData).split('|');
    }
    if (data.size() > 0)
        mRemainData = data.takeLast();
    else mRemainData = QString();
    if (data.isEmpty()) return;

    QString file = (data.first().isEmpty() ? mBreakLinesFile : QDir::fromNativeSeparators(data.first()));
    QList<int> lines;
    QList<int> coLNs;
    for (int i = 1; i < data.size(); ++i) {
        bool ok = getPair(data.at(i), lines, coLNs);
        if (!ok) logMessage("Debug-Server: breakLines parse error: " + data.at(i));
    }
    if (!lines.isEmpty()) {
        if (mVerbose) {
            QString list("DATA=> " + file + " : ");
            for (int i = 0; i < lines.size(); ++i) {
                list += "|" + QString::number(lines.at(i)) + ":" + QString::number(coLNs.at(i));
            }
            logMessage(list);
        }
        emit signalLinesMap(file, lines, coLNs);
    }
    mBreakLinesFile = file;
}

bool Server::getPair(const QString assignment, QList<int> &lines, QList<int> &coLNs)
{
    QStringList linePair = assignment.split("=");
    bool ok = (linePair.size() == 2 && !linePair.at(1).isEmpty());
    if (ok) {
        int line = linePair.at(0).toInt(&ok);
        if (ok) {
            int coLN = linePair.at(1).toInt(&ok);
            if (ok) {
                lines << line;
                coLNs << coLN;
            }
        }
    }
    return ok;
}

void Server::setState(DebugState state)
{
    if (mState != state) {
        mState = state;
        emit stateChanged(state);
    }
}

DebugState Server::state() const
{
    return mState;
}

void Server::addBreakpoint(int contLine)
{
    callProcedure(addBP, {QString::number(contLine)});
}

void Server::addBreakpoints(const QList<int> &contLines)
{
    QString bps = toBpString(contLines);
    if (!bps.isEmpty())
        callProcedure(addBP, {bps});
}

void Server::delBreakpoint(int contLine)
{
    callProcedure(delBP, {QString::number(contLine)});
}

void Server::clearBreakpoints()
{
    callProcedure(delBP);
}

void Server::sendRun()
{
    callProcedure(run);
    setState(Running);
}

void Server::sendStepLine()
{
    callProcedure(stepLine);
    setState(Running);
}

void Server::sendPause()
{
    callProcedure(pause);
}

void Server::sendWriteGdx(const QString &gdxFile)
{
    callProcedure(writeGDX, {gdxFile});
}

bool Server::isListening()
{
    return mServer && mServer->isListening();
}

quint16 Server::port()
{
    return mServer ? mServer->serverPort() : -1;
}

void Server::logMessage(const QString &message)
{
    emit addProcessLog(message.toUtf8() + '\n');
}

void Server::deleteSocket()
{
    if (mSocket) {
        QTcpSocket *socket = mSocket;
        mSocket = nullptr;
        socket->close();
        socket->deleteLater();
    }
}


} // namespace debugger
} // namespace studio
} // namespace gams
