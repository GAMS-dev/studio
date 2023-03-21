#include "server.h"
#include <QTcpServer>
#include <QTcpSocket>

namespace gams {
namespace studio {
namespace debugger {

Server::Server(QObject *parent) : QObject{parent}
{ }

bool Server::start(quint16 port)
{
    if (mServer) {
        if (mPort != port) {
            qDebug() << "ERROR: Can't connect to port" + port + " because the server already listens to port" + mPort;
            return false;
        }
        qDebug() << "WARNING: The server already listens to port" + mPort;
        return true;
    }
    mServer = new QTcpServer(this);
    mPort = port;
    connect(mServer, &QTcpServer::newConnection, this, &QTcpServer::newConnection, Qt::UniqueConnection);

    if(!mServer->listen(QHostAddress::Any, mPort)) {
        qDebug() << "Server could not start!";
    } else {
        qDebug() << "Server started!";
    }

}

void Server::newConnection()
{
    QTcpSocket *socket = mServer->nextPendingConnection();

    socket->write("Here's Studio\r\n");
    socket->flush();

    socket->waitForBytesWritten(3000);

    socket->close();
}


} // namespace debugger
} // namespace studio
} // namespace gams
