#ifndef GAMS_STUDIO_NEOS_NEOSXMLRPC_H
#define GAMS_STUDIO_NEOS_NEOSXMLRPC_H

#include <QUrl>
#include <QtNetwork>
#include <QtXml/QDomDocument>

namespace gams {
namespace studio {
namespace neos {

class NeosXmlRpc: public QObject
{
    Q_OBJECT
public:
    NeosXmlRpc(QUrl url, QObject *parent);

signals:
    void received(QVariant data, QNetworkReply *reply);

public slots:
    void parseResponse(QString response, QNetworkReply *reply);

private:
    QVariant toVariant(QDomElement node);

private:
    QUrl mUrl;

};

} // namespace neos
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NEOS_NEOSXMLRPC_H
