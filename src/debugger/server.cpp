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

namespace gams {
namespace studio {
namespace debugger {

const quint16 CFirstDebugPort = 3563;
const quint16 CLastDebugPort = 4563;
QSet<int> Server::mPortsInUse;

Server::Server(QObject *parent) : QObject(parent)
{
    mServer = new QTcpServer(this);
    connect(mServer, &QTcpServer::newConnection, this, &Server::newConnection, Qt::UniqueConnection);
}

Server::~Server()
{
    stop();
    mServer->deleteLater();
    mServer = nullptr;
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

void Server::addBreakpoint(const Breakpoint &breakpoint)
{
    if (mBreakpoints.contains(breakpoint))
        return;
    mBreakpoints << breakpoint;
    sortBreakpoints();
}

void Server::addBreakpoints(const QList<Breakpoint> &breakpoints)
{
    for (const Breakpoint &breakpoint : breakpoints)
        mBreakpoints << breakpoint;
    sortBreakpoints();
}

void Server::removeBreakpoint(const Breakpoint &breakpoint)
{
    mBreakpoints.removeAll(breakpoint);
}

void Server::clearBreakpoints()
{
    mBreakpoints.clear();
}

void Server::sendInterrupt()
{

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
        if (data.compare("ready", Qt::CaseInsensitive) == 0) {
            emit signalReady();
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
        } else {
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
        return b1.column - b2.column;
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

void Server::callProcedure(const QString &procedure, const QStringList &arguments)
{
    if (!mSocket->isOpen()) {
        logMessage("Debug-Server: Socket not open, can't process '" + procedure + ":" + arguments.join(":") + "'");
        return;
    }
    mSocket->write((procedure + ":" + arguments.join(":")).toUtf8());
}


} // namespace debugger
} // namespace studio
} // namespace gams
