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
            qDebug() << "ERROR: Can't connect to port" + QString::number(port)
                        + ". The server already listens to port" + QString::number(mServer->serverPort());
            return false;
        }
        qDebug() << "WARNING: The server already listens to port" + mServer->serverPort();
        return true;
    }

    if(!mServer->listen(QHostAddress::LocalHost, port)) {
        qDebug() << "Server could not start!";
        return false;
    }
    qDebug() << "Server started!";
    return true;
}

void Server::stop()
{
    for (QTcpSocket *socket : mSockets) {
        socket->close();
    }

    if (isListening())
        mServer->close();
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
        qDebug() << "Socket disconnected";
        mSockets.removeAll(socket);
    });
    connect(socket, &QTcpSocket::readyRead, this, [socket]() {
        QByteArray data = socket->readAll();
        qDebug() << "Request: " << data;
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


} // namespace debugger
} // namespace studio
} // namespace gams
