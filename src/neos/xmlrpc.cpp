#include "xmlrpc.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include <QVariantMap>
#include <QDateTime>
#include "iostream"

namespace gams {
namespace studio {
namespace neos {

QStringList tokens {
    "NoToken",
    "Invalid",
    "StartDocument",
    "EndDocument",
    "StartElement",
    "EndElement",
    "Characters",
    "Comment",
    "DTD",
    "EntityReference",
    "ProcessingInstruction"
};


XmlRpc::XmlRpc()
{

}

bool putVariant(QXmlStreamWriter &xml, QVariant var, QByteArray &errorText)
{
    xml.writeStartElement("value");
    switch (var.type()) {
    case QVariant::List: {        // <array><data>…</data></array>
        xml.writeStartElement("array");
        xml.writeStartElement("data");
        QVariantList list = var.toList();
        for (QVariant v : list) {
            if (!putVariant(xml, v, errorText)) return false;
        }
        xml.writeEndElement();
        xml.writeEndElement();
    }   break;
    case QVariant::ByteArray:   // <base64>..==</base64>
        xml.writeTextElement("base64", var.toByteArray().toBase64());
        break;
    case QVariant::Bool:        // <boolean>1</boolean>
        xml.writeTextElement("boolean", var.toString());
        break;
    case QVariant::DateTime:    // <dateTime.iso8601>20200414T16:23:55</dateTime.iso8601>
        xml.writeTextElement("dateTime.iso8601", var.toDateTime().toString(Qt::ISODate));
        break;
    case QVariant::Double:      // <double>-0.32653</double>
        xml.writeTextElement("double", var.toString());
        break;
    case QVariant::UInt:
    case QVariant::Int:         // <int>32</int> or <i4>32</i4>
        xml.writeTextElement("int", var.toString());
        break;
    case QVariant::String:      // <string>..</string>
        xml.writeTextElement("string", var.toString());
        break;
    case QVariant::Hash: {      // <struct><member><name>…</name><value>..</value></member></struct>
        xml.writeStartElement("struct");
        QVariantHash map = var.toHash();
        QVariantHash::const_iterator it;
        for (it = map.constBegin(); it != map.constEnd(); ++it) {
            xml.writeStartElement("member");
            xml.writeTextElement("name", it.key());
            if (!putVariant(xml, it.value(), errorText)) return false;
            xml.writeEndElement();
        }
        xml.writeEndElement();
    }   break;
    default:
        errorText = "XML-Error: Unsupported QVariant of type ";
        errorText.append(QByteArray::number(var.type()));
        qDebug() << errorText;
        return false;
    }
    xml.writeEndElement();
    return true;
}

QByteArray XmlRpc::prepareCall(const QString &method, const QVariantList &params)
{
    QByteArray res;
    QByteArray errorText;
    QXmlStreamWriter xml(&res);
    xml.writeStartDocument();
    xml.writeStartElement("methodCall");
    xml.writeTextElement("methodName", method);
    if (!params.isEmpty()) {
        xml.writeStartElement("params");
        for (QVariant var : params) {
            xml.writeStartElement("param");
            if (!putVariant(xml, var, errorText)) return errorText;
            xml.writeEndElement();
        }
    }
    xml.writeEndElement();
    xml.writeEndDocument();
    return res;

}

const QStringList typeTag {"array","base64","boolean","dateTime.iso8601","double","int","i4","string","struct"};
int typeTagId(QStringRef ref) { // workaround: currently indexOf(QStringRef) doesn't work on all platforms
    for (int i = 0; i < typeTag.size(); ++i)
        if (typeTag.at(i).compare(ref, Qt::CaseInsensitive) == 0) return i;
    return -1;
}

QVariantList getVariantList(QXmlStreamReader &xml);
QVariantHash getVariantHash(QXmlStreamReader &xml);

QVariant getVariant(QXmlStreamReader &xml)
{
    if (xml.name().compare(QLatin1String("value")) != 0) return QVariant();
    if (!xml.readNextStartElement()) return QVariant();
    QVariant res;
    int id = typeTagId(xml.name());
    if (id<0) return QVariant();
    bool ok = true;
    switch (id) {
    case 0: {       // array
        res = getVariantList(xml);
    }   break;
    case 1:         // base64
        res = QByteArray::fromBase64(xml.readElementText().toLatin1());
        break;
    case 2: {       // boolean
        QString s = xml.readElementText().toLower();
        res = (s == "true" || s == "1");
    }   break;
    case 3:         // dateTime.iso8601
        res = QDateTime::fromString(xml.readElementText(), Qt::ISODate);
        break;
    case 4:         // double
        res = xml.readElementText().toDouble(&ok);
        break;
    case 5:         // int
    case 6:         // i4
        res = xml.readElementText().toInt(&ok);
        break;
    case 7:         // string
        res = xml.readElementText();
        break;
    case 8: {       // struct
        res = getVariantHash(xml);
    }   break;
    default:
        break;
    }
    while (xml.tokenType() != QXmlStreamReader::EndElement || xml.name().compare(QLatin1String("value")) != 0)
        xml.skipCurrentElement();
    return res;
}

QVariantList getVariantList(QXmlStreamReader &xml)
{   // <array><data><value>..</value><value>..</value></data></array>
    QVariantList list;
    if (xml.name().compare(QLatin1String("array")) != 0) return QVariantList();
    if (!xml.readNextStartElement()) return QVariantList();
    if (xml.name().compare(QLatin1String("data")) != 0) return QVariantList();
    while (xml.readNextStartElement())
        list << getVariant(xml);
    return list;
}

QVariantHash getVariantHash(QXmlStreamReader &xml)
{   // <struct><member><name>..</name><value>..</value></member><member>..</member></struct>
    QVariantHash hash;
    if (xml.name().compare(QLatin1String("struct")) != 0) return QVariantHash();
    while (xml.readNextStartElement()) {
        if (xml.name().compare(QLatin1String("member")) != 0) return hash;
        if (!xml.readNextStartElement()) return hash;
        if (xml.name().compare(QLatin1String("name")) != 0) return QVariantHash();
        QString name = xml.readElementText();
        if (!xml.readNextStartElement()) return hash;
        hash.insert(name, getVariant(xml));
        xml.skipCurrentElement();
    }
    return hash;
}

QVariantList XmlRpc::parseParams(QIODevice *device, QString &method)
{
    QVariantList res;
    QXmlStreamReader xml(device);
    method = QString();
    if (xml.readNextStartElement()) {
        // it's a call, read method name
        if (xml.name().compare(QLatin1String("methodCall")) == 0) {
            if (xml.readNextStartElement() && xml.name().compare(QLatin1String("methodName")) == 0) {
                method = xml.readElementText();
                xml.readNextStartElement();
            } else return res;
        }

        // check for abort states
        if (xml.name().compare(QLatin1String("methodResponse")) == 0) {
            if (!xml.readNextStartElement()) return res; // missing params
        } else if (method.isEmpty()) return res; // neither call nor response
        if (xml.name().compare(QLatin1String("params")) != 0) {
            return res;
        }

        // read values
        while (xml.readNextStartElement()) {
            if (xml.name().compare(QLatin1String("param")) != 0) continue;
            if (!xml.readNextStartElement()) break;
            QVariant var = getVariant(xml);
            if (var.isNull())
                return res;
            res << var;
            xml.skipCurrentElement();
        }
    }
    return res;
}

} // namespace neos
} // namespace studio
} // namespace gams
