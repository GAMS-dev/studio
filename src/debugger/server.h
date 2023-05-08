#ifndef GAMS_STUDIO_DEBUGGER_SERVER_H
#define GAMS_STUDIO_DEBUGGER_SERVER_H

#include <QObject>

class QTcpServer;
class QTcpSocket;

namespace gams {
namespace studio {
namespace debugger {

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    bool isListening();
    quint16 port();
    bool start(quint16 port);
    void stop();

public slots:
    void newConnection();

signals:
    void addProcessData(const QByteArray &data);

private:
    void logMessage(const QString &message);

    QTcpServer *mServer = nullptr;
    QList<QTcpSocket*> mSockets;
};

} // namespace debugger
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_DEBUGGER_SERVER_H
