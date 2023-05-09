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

namespace gams {
namespace studio {
namespace debugger {

Server::Server(QObject *parent) : QObject(parent)
{ }

bool Server::start(quint16 port)
{
    if (!mServer) {
        mServer = new QTcpServer(this);
        connect(mServer, &QTcpServer::newConnection, this, &Server::newConnection, Qt::UniqueConnection);
    }

    if (mServer->isListening()) {
        if (mServer->serverPort() != port) {
            logMessage("ERROR: Can't connect to port" + QString::number(port)
                           + ". The Debug-Server already listens to port" + QString::number(mServer->serverPort()));
            return false;
        }
        logMessage("WARNING: The Debug-Server already listens to port" + mServer->serverPort());
        return true;
    }

    if(!mServer->listen(QHostAddress::LocalHost, port)) {
        logMessage("Debug-Server could not start listening to port " + QString::number(port) + "!");
        return false;
    }
    logMessage("Debug-Server started. Listening at port " + QString::number(port));
    return true;
}

void Server::stop()
{
    for (QTcpSocket *socket : mSockets) {
        socket->close();
    }

    if (isListening())
        mServer->close();
    logMessage("Debug-Server stopped.");
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
    mSockets << socket;
    connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
        logMessage("Debug-Server: Socket disconnected");
        mSockets.removeAll(socket);
    });
    connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
        QByteArray data = socket->readAll();
        logMessage("Debug-Server request: " + data);
        if (data.startsWith("exit")) {
            socket->write("bye!\r\n");
            socket->flush();
            socket->waitForBytesWritten(3000);
            socket->close();
        } else {
            socket->write("Got the message: " + data + "\r\n");
            socket->flush();
            socket->waitForBytesWritten(3000);
        }
    });
}

void Server::logMessage(const QString &message)
{
    emit addProcessData(message.toUtf8() + '\n');
}


} // namespace debugger
} // namespace studio
} // namespace gams
