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
#include "server.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QCollator>
#include <QFile>

namespace gams {
namespace studio {
namespace debugger {

const quint16 CFirstDebugPort = 3563;
const quint16 CLastDebugPort = 4563;
QSet<int> Server::mPortsInUse;

Server::Server(QObject *parent) : QObject(parent)
{
    init();
    mServer = new QTcpServer(this);
    connect(mServer, &QTcpServer::newConnection, this, &Server::newConnection, Qt::UniqueConnection);
}

Server::~Server()
{
    stop();
    mServer->deleteLater();
    mServer = nullptr;
}

void Server::init()
{
    mCalls.insert(invalidReply, "invalidReply");
    mCalls.insert(writeGDX, "writeGDX");
    mCalls.insert(addBP, "addBP");
    mCalls.insert(delBP, "delBP");
    mCalls.insert(addBPs, "addBPs");
    mCalls.insert(clearBPs, "clearBPs");

    mReplies.insert("invalidCall", invalidCall);
    mReplies.insert("breakAt", breakAt);
    mReplies.insert("gdxReady", gdxReady);
    mReplies.insert("finished", finished);
}

bool Server::start()
{
    if (mServer->isListening()) {
        logMessage("WARNING: The Debug-Server already listens to port" + mServer->serverPort());
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

void Server::stop()
{
    deleteSocket();
    mPortsInUse.remove(mServer->serverPort());
    if (isListening())
        mServer->close();
    logMessage("Debug-Server stopped.");
}

void Server::callProcedure(Call call, const QStringList &arguments)
{

    if (!mSocket->isOpen()) {
        QString additionals = arguments.count() > 1 ? (" (and " + arguments.count()-1 + " more)") : "";
        logMessage("Debug-Server: Socket not open, can't process '" + mCalls.value(call, "undefinedCall")
                   + (arguments.isEmpty() ? "'" : (":" + arguments.at(0) + "'" + additionals)));
        return;
    }
    QString keyword = mCalls.value(call);
    if (keyword.isEmpty()) {
        QString additionals = arguments.count() > 1 ? (" (and " + arguments.count()-1 + " more)") : "";
        logMessage("Debug-Server: Undefined call '" + int(call) +
                   + (arguments.isEmpty() ? "'" : (":" + arguments.at(0) + "'" + additionals)));
        return;
    }
    mSocket->write((keyword + (arguments.isEmpty() ? "" : ('\n' + arguments.join('\n')))).toUtf8());
}

bool Server::handleReply(const QString &replyData)
{
    QStringList reList = replyData.split('\n');
    Reply reply = Reply::invalid;
    if (!reList.isEmpty()) {
        reply = mReplies.value(reList.at(0), Reply::invalid);
        if (reply != Reply::invalid)
            reList.removeFirst();
    }
    bool ok = false;
    switch (reply) {
    case invalidCall:
        logMessage("Debug-Server: GAMS refused to process this request: " + reList.join(", "));
        break;
    case breakAt:
        if (reList.size() < 1) {
            logMessage("Debug-Server: [breakAt] Missing data for interrupt.");
            return false;
        }
        if (reList.size() > 1)
            logMessage("Debug-Server: [breakAt] Only one entry expected. Additional data ignored.");
        QStringList data = reList.first().split(':');
        QString file = data.first();
        if (file.isEmpty()) {
            logMessage("Debug-Server: [breakAt] Missing filename");
            return false;
        }
        if (!QFile::exists(file)) {
            logMessage("Debug-Server: [breakAt] File not found: " + file);
            return false;
        }
        if (data.size() < 2) {
            logMessage("Debug-Server: [breakAt] Missing line number for file " + data.first());
        }

        int line = data.at(1).toInt(&ok);
        if (!ok) {
            logMessage("Debug-Server: [breakAt] Can't parse line number: " + data.at(1));
            return;
        }
        emit signalInterruptedAt(file, line);
        break;
    case gdxReady:
        emit signalReady();
        break;
    case finished:
        break;
    default:
        logMessage("Debug-Server: Unknown GAMS request: " + reList.join(", "));
        break;
    }

    if (data.compare("ready", Qt::CaseInsensitive) == 0) {
    } else if (data.startsWith("interrupt@") == 0) {
        bool ok;
        int lineSep = data.lastIndexOf(':');
        QString file = data.mid(10, lineSep - 11);
        int line = data.last(data.size() - lineSep - 1).toInt(&ok);
        if (ok)
            emit signalInterruptedAt(file, line);
    } else if (data.compare("exit", Qt::CaseInsensitive) == 0) {
        logMessage("Debug-Server: Stop request from GAMS.");
        stop();
    }


    return false;
}

void Server::addBreakpoint(const Breakpoint &bp)
{
    if (mBreakpoints.contains(bp))
        return;
    mBreakpoints << bp;
    callProcedure(addBP, bp.toString());
}

void Server::addBreakpoints(const QList<Breakpoint> &bps)
{
    QStringList args;
    for (const Breakpoint &breakpoint : bps) {
        mBreakpoints << breakpoint;
        args << breakpoint.toString();
    }
    callProcedure(addBPs, args);
}

void Server::removeBreakpoint(const Breakpoint &bp)
{
    mBreakpoints.removeAll(bp);
    callProcedure(delBP, bp.toString());
}

void Server::clearBreakpoints()
{
    mBreakpoints.clear();
    callProcedure(clearBPs);
}

void Server::sendInterrupt()
{
    callProcedure(interrupt);
}

bool Server::isListening()
{
    return mServer && mServer->isListening();
}

quint16 Server::port()
{
    return mServer ? mServer->serverPort() : -1;
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
    connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
        logMessage("Debug-Server: Socket disconnected");
        deleteSocket();
    });
    connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
        QByteArray data = socket->readAll();
        if (!handleReply(data)) {
            callProcedure(invalidReply, QStringList() << data);
            logMessage("Debug-Server: Unknown request: " + data);
        }
    });
}

void Server::sortBreakpoints()
{
    std::sort(mBreakpoints.begin(), mBreakpoints.end(), [&](const Breakpoint& b1, const Breakpoint& b2) {
        int comp = b1.file.compare(b2.file);
        if (comp != 0) return comp;
        comp = b1.line - b2.line;
        if (comp != 0) return comp;
        return b1.index - b2.index;
    });
}

void Server::logMessage(const QString &message)
{
    emit addProcessData(message.toUtf8() + '\n');
}

void Server::deleteSocket()
{
    if (mSocket) {
        mSocket->close();
        mSocket->deleteLater();
        mSocket = nullptr;
    }
}


} // namespace debugger
} // namespace studio
} // namespace gams
