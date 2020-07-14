#include "neosxmlrpc.h"
#include "logger.h"

#include <QString>

namespace gams {
namespace studio {
namespace neos {

QString s1 =
R"s1(
<document>
<category>lp</category>
<solver>BDMLP</solver>
<priority>%priority%</priority>
<inputType>GAMS</inputType>
<model><![CDATA[]]></model>
<options><![CDATA[]]></options>
<parameters><![CDATA[fw=1%parameters%]]></parameters>
<restart><base64>:restartb64:</base64></restart>
<wantlog><![CDATA[yes]]></wantlog>
<wantlst><![CDATA[yes]]></wantlst>
<wantgdx><![CDATA[%wantgdx%]]></wantgdx>
</document>
)s1";

NeosXmlRpc::NeosXmlRpc(QUrl url, QObject* parent) : QObject(parent), mUrl(url)
{

}

void NeosXmlRpc::parseResponse(QString response, QNetworkReply *reply)
{
    QDomDocument doc;
    QVariant arg;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if (!doc.setContent(response, &errorMsg, &errorLine, &errorColumn)) {
        DEB() << "XML not well formatted";
        return;
    }
    if (doc.documentElement().firstChild().toElement().tagName().toLower() == "params") {
        QDomNode paramNode = doc.documentElement().firstChild().firstChild();
        if(!paramNode.isNull())
            arg = toVariant(paramNode.firstChild().toElement());
        emit received(arg, reply);
    } else if (doc.documentElement().firstChild().toElement().tagName().toLower() == "fault") {
        const QVariantMap errDat = toVariant(doc.documentElement().firstChild().firstChild().toElement()).toMap();
        DEB() << "Error " << errDat.value("faultCode").toInt() << ": " << errDat.value("faultString").toString();
    } else {
        DEB() << "Error in xmlrpc";
    }
    return;
}

QVariant NeosXmlRpc::toVariant(QDomElement node)
{
    if (node.tagName().toLower() != "value") return QVariant();
    if (!node.firstChild().isElement()) return QVariant(node.text());

    const QDomElement elem = node.firstChild().toElement();
    const QString type = elem.tagName().toLower();

    if (type == "string") return QVariant(elem.text());
    else if (type == "i4" || type == "int") return QVariant(elem.text().toInt());
    else if (type == "double") return QVariant(elem.text().toDouble());
    else if (type == "boolean") return (elem.text().toLower() == "true" || elem.text() == "1");
    else if (type == "base64") return QVariant(QByteArray::fromBase64( elem.text().toLatin1()));
    else if (type == "datetime" || type == "datetime.iso8601")
        return QVariant(QDateTime::fromString(elem.text(), "yyyyMMddThh:mm:ss"));
    else if (type == "array") {
        QList<QVariant> values;
        QDomNode node = elem.firstChild().firstChild();
        while (!node.isNull()) {
            values << toVariant(node.toElement());
            node = node.nextSibling();
        }
        return QVariant(values);
    }
    else if (type == "struct") {
        QMap<QString, QVariant> map;
        QDomNode memberNode = elem.firstChild();
        while (!memberNode.isNull())	{
            const QString key = memberNode.toElement().elementsByTagName("name").item(0).toElement().text();
            map[key] = toVariant(memberNode.toElement().elementsByTagName("value").item(0).toElement());
            memberNode = memberNode.nextSibling();
        }
        return QVariant(map);
    } else {
        DEB() << "XML error: Unknown type " << elem.tagName().toLower();
    }
    return QVariant();
}



} // namespace neos
} // namespace studio
} // namespace gams
