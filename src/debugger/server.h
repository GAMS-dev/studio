#ifndef GAMS_STUDIO_DEBUGGER_SERVER_H
#define GAMS_STUDIO_DEBUGGER_SERVER_H

#include <QObject>

class QTcpServer;

namespace gams {
namespace studio {
namespace debugger {

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    bool start(quint16 port);

public slots:
    void newConnection();

signals:

private:
    QTcpServer *mServer = nullptr;
    quint16 mPort;
};

} // namespace debugger
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_DEBUGGER_SERVER_H
