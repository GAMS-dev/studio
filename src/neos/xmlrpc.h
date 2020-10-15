#ifndef GAMS_STUDIO_NEOS_XMLRPC_H
#define GAMS_STUDIO_NEOS_XMLRPC_H

#include <QString>
#include <QVariantList>
#include <QIODevice>

namespace gams {
namespace studio {
namespace neos {

class XmlRpc
{
public:
    static QByteArray prepareCall(const QString &method, const QVariantList &params = QVariantList());
    static QVariantList parseParams(QIODevice *device, QString &method);

private:
    XmlRpc();
};

} // namespace neos
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NEOS_XMLRPC_H
