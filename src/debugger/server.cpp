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

Server::Server(const QString &path, QObject *parent) : QObject(parent)
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
    mCalls.insert(addBP, "addBP");
    mCalls.insert(delBP, "delBP");
    mCalls.insert(addBPs, "addBPs");
    mCalls.insert(clearBPs, "clearBPs");

    mCalls.insert(run, "run");
    mCalls.insert(stepLine, "stepLine");
    mCalls.insert(interrupt, "interrupt");
    mCalls.insert(writeGDX, "writeGDX");

    mReplies.insert("invalidCall", invalidCall);
    mReplies.insert("breakAt", paused);
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

QString Server::gdxTempFile() const
{
    return mPath + "/temp" + QString::number(mServer->serverPort()) + ".gdx";
}

void Server::callProcedure(CallReply call, const QStringList &arguments)
{

    if (!mSocket->isOpen()) {
        QString additionals = arguments.count() > 1 ? QString(" (and %1 more)").arg(arguments.count()-1)
                                                    : QString();
        logMessage("Debug-Server: Socket not open, can't process '" + mCalls.value(call, "undefinedCall")
                   + (arguments.isEmpty() ? "'" : (":" + arguments.at(0) + "'" + additionals)));
        return;
    }
    QString keyword = mCalls.value(call);
    if (keyword.isEmpty()) {
        QString additionals = arguments.count() > 1 ? QString(" (and %1 more)").arg(arguments.count()-1)
                                                    : QString();
        logMessage("Debug-Server: Undefined call '" + int(call)
                   + (arguments.isEmpty() ? "'" : (":" + arguments.at(0) + "'" + additionals)));
        return;
    }
    mSocket->write((keyword + (arguments.isEmpty() ? "" : ('\n' + arguments.join('\n')))).toUtf8());
}

bool Server::handleReply(const QString &replyData)
{
    QStringList reList = replyData.split('\n');
    CallReply reply = invalid;
    if (!reList.isEmpty()) {
        reply = mReplies.value(reList.at(0), invalid);
        if (reply != invalid)
            reList.removeFirst();
    }
    QStringList data;
    QString file;
    if (reList.size()) {
        data = reList.first().split(':');
        file = data.first();
    }

    bool ok = false;
    switch (reply) {
    case invalidCall:
        logMessage("Debug-Server: GAMS refused to process this request: " + reList.join(", "));
        break;
    case paused: {
        if (reList.size() < 1) {
            logMessage("Debug-Server: [paused] Missing data for interrupt.");
            return false;
        }
        if (reList.size() > 1)
            logMessage("Debug-Server: [paused] Only one entry expected. Additional data ignored.");
        if (file.isEmpty()) {
            logMessage("Debug-Server: [paused] Missing filename");
            return false;
        }
        if (!QFile::exists(file)) {
            logMessage("Debug-Server: [paused] File not found: " + file);
            return false;
        }
        if (data.size() < 2) {
            logMessage("Debug-Server: [paused] Missing line number for file " + data.first());
        }

        int line = data.at(1).toInt(&ok);
        if (!ok) {
            logMessage("Debug-Server: [paused] Can't parse line number: " + data.at(1));
            return false;
        }
        emit signalPaused(file, line);
    }   break;
    case gdxReady:
        if (file.isEmpty()) {
            logMessage("Debug-Server: [gdxReady] Missing name for GDX file.");
            return false;
        }
        if (!QFile::exists(file)) {
            logMessage("Debug-Server: [gdxReady] File not found: " + file);
            return false;
        }
        emit signalGdxReady(file);
        break;
    case finished:
        break;
    default:
        logMessage("Debug-Server: Unknown GAMS request: " + reList.join(", "));
        return false;
    }

    return true;
}

QString Server::toBpString(const QString &file, QSet<int> lines)
{
    if (lines.isEmpty()) return QString();
    QString res = file;
    auto iter = lines.constBegin();
    while (iter != lines.constEnd())
        res += ':' + QString::number(*iter);
    return res;
}

void Server::addBreakpoint(const QString &filename, int line)
{
    callProcedure(addBP, {toBpString(filename, {line})});
}

void Server::addBreakpoints(const QHash<QString, QSet<int> > &breakpoints)
{
    QStringList args;
    auto iter = breakpoints.constBegin();
    while (iter != breakpoints.constEnd()) {
        if (!iter.value().isEmpty())
            args << toBpString(iter.key(), iter.value());
        ++iter;
    }
    if (!args.isEmpty())
        callProcedure(addBP, args);
}

void Server::delBreakpoint(const QString &filename, int line)
{
    callProcedure(delBP, {toBpString(filename, {line})});
}

void Server::clearBreakpoints(const QString file)
{
    callProcedure(delBP, {file});
}

void Server::sendRun()
{
    callProcedure(run);
}

void Server::sendStepLine()
{
    callProcedure(stepLine);
}

void Server::sendInterrupt()
{
    callProcedure(interrupt);
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
