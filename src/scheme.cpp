#include "scheme.h"
#include <QHash>
#include "logger.h"
#include "svgengine.h"
#include <QMetaEnum>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

namespace gams {
namespace studio {

Scheme *Scheme::mInstance = nullptr;
const QColor CUndefined(255, 0, 200);

Scheme::Scheme(QObject *parent) : QObject(parent)
{}

Scheme::~Scheme()
{
    for (SvgEngine *eng: mEngines)
        eng->unbind();
    mEngines.clear();
}

Scheme *Scheme::instance()
{
    if (!mInstance) mInstance = new Scheme();
    return mInstance;
}

void Scheme::initDefault()
{
    mColorSchemes.clear();
    mSchemeNames.clear();

    // Add and switch to first color scheme  ------- Standard -------
    int sNr = 0;
    mColorSchemes << ColorScheme();
    mSchemeNames << "Standard";

    mColorSchemes[sNr].clear();
    mColorSchemes[sNr].insert(invalid,                        CUndefined);
    mColorSchemes[sNr].insert(Edit_currentLineBg,             QColor(255, 250, 170));
    mColorSchemes[sNr].insert(Edit_errorBg,                   QColor(Qt::lightGray));
    mColorSchemes[sNr].insert(Edit_currentWordBg,             QColor(210,200,200));
    mColorSchemes[sNr].insert(Edit_matchesBg,                 QColor(Qt::green).lighter(160));
    mColorSchemes[sNr].insert(Edit_parenthesesValidFg,        QColor(Qt::red));
    mColorSchemes[sNr].insert(Edit_parenthesesInvalidFg,      QColor(Qt::black));
    mColorSchemes[sNr].insert(Edit_parenthesesValidBg,        QColor(Qt::green).lighter(170));
    mColorSchemes[sNr].insert(Edit_parenthesesInvalidBg,      QColor(Qt::red).lighter(150));
    mColorSchemes[sNr].insert(Edit_parenthesesValidBgBlink,   QColor(Qt::green).lighter(130));
    mColorSchemes[sNr].insert(Edit_parenthesesInvalidBgBlink, QColor(Qt::red).lighter(115));
    mColorSchemes[sNr].insert(Edit_linenrAreaBg,              QColor(245,245,245));
    mColorSchemes[sNr].insert(Edit_linenrAreaMarkBg,          QColor(225,255,235));
    mColorSchemes[sNr].insert(Edit_linenrAreaFg,              QColor(Qt::gray));
    mColorSchemes[sNr].insert(Edit_linenrAreaMarkFg,          QColor(Qt::black));
    mColorSchemes[sNr].insert(Edit_blockSelectBg,             QColor(Qt::cyan).lighter(150));

    mColorSchemes[sNr].insert(Mark_errorFg,                   QColor(Qt::darkRed));
    mColorSchemes[sNr].insert(Mark_listingFg,                 QColor(Qt::blue));
    mColorSchemes[sNr].insert(Mark_fileFg,                    QColor(Qt::darkGreen));

    mColorSchemes[sNr].insert(Icon_Main,                      QColor(Qt::white));
    mColorSchemes[sNr].insert(Icon_Invers,                    QColor(Qt::black));

    mColorSchemes[sNr].insert(Syntax_undefined,               CUndefined);
    mColorSchemes[sNr].insert(Syntax_neutral,                 Color());
    mColorSchemes[sNr].insert(Syntax_directive,               Color(QColor(Qt::darkMagenta).darker(120)));
    mColorSchemes[sNr].insert(Syntax_assign,                  Color());
    mColorSchemes[sNr].insert(Syntax_directiveBody,           Color(QColor(Qt::darkBlue).lighter(170), fItalic));
    mColorSchemes[sNr].insert(Syntax_comment,                 Color(QColor(80, 145, 75), fItalic));
    mColorSchemes[sNr].insert(Syntax_title,                   Color(QColor(Qt::darkBlue).lighter(140), fBoldItalic));
    mColorSchemes[sNr].insert(Syntax_keyword,                 Color(QColor(Qt::darkBlue).lighter(140), fBold));
    mColorSchemes[sNr].insert(Syntax_declaration,             Color(QColor(Qt::darkBlue).lighter(140), fBold));
    mColorSchemes[sNr].insert(Syntax_identifier,              Color(QColor(Qt::black)));
    mColorSchemes[sNr].insert(Syntax_description,             Color(QColor(Qt::darkBlue).lighter(170)));
    mColorSchemes[sNr].insert(Syntax_identifierAssign,        Color(QColor(Qt::darkGreen).darker(140)));
    mColorSchemes[sNr].insert(Syntax_assignLabel,             Color(QColor(Qt::darkGreen).darker(110)));
    mColorSchemes[sNr].insert(Syntax_assignValue,             Color(QColor(0, 80, 120)));
    mColorSchemes[sNr].insert(Syntax_tableHeader,             Color(QColor(Qt::darkGreen).darker(140), fBold));
    mColorSchemes[sNr].insert(Syntax_embedded,                Color(QColor(200, 70, 0)));

    // Add and switch to second color scheme  ------- Dark -------
    mColorSchemes << mColorSchemes.at(sNr++);
    mSchemeNames << "Dark";

    mColorSchemes[sNr].insert(Edit_currentLineBg,             QColor(205, 220, 255));
    mColorSchemes[sNr].insert(Edit_linenrAreaBg,              QColor(16,16,16));
    mColorSchemes[sNr].insert(Edit_linenrAreaMarkBg,          QColor(40,40,40));
    mColorSchemes[sNr].insert(Edit_linenrAreaMarkFg,          QColor(Qt::white));

    mColorSchemes[sNr].insert(Icon_Invers,                    QColor(Qt::darkRed));

    mColorSchemes[sNr].insert(Syntax_title,                   Color(QColor(Qt::darkRed).lighter(140), fBold));
    mColorSchemes[sNr].insert(Syntax_directive,               Color(QColor(Qt::darkGreen).darker(120), fBold));

}

QStringList Scheme::schemes()
{
    return mSchemeNames;
}

int Scheme::setActiveScheme(QString schemeName)
{
    int scheme = mSchemeNames.indexOf(schemeName);
    return setActiveScheme(scheme);
}

int Scheme::setActiveScheme(int scheme)
{
    DEB() << "Scheme switched to " << scheme;
    if (scheme < 0 || scheme >= mSchemeNames.size()) return -1;
    mActiveScheme = scheme;
    invalidate();
    return mActiveScheme;
}

int Scheme::activeScheme() const
{
    return mActiveScheme;
}

QString Scheme::name(Scheme::ColorSlot slot)
{
    return QMetaEnum::fromType<ColorSlot>().valueToKey(slot);
}

Scheme::ColorSlot Scheme::slot(QString name)
{
    bool ok;
    int value = QMetaEnum::fromType<ColorSlot>().keyToValue(name.toLatin1().data(), &ok);
    if (!ok) return invalid;
    return ColorSlot(value);
}

QHash<QString, QString> Scheme::iconCodes() const
{
    QHash<QString, QString> res;
    const ColorScheme &scheme = mColorSchemes.at(mActiveScheme);
    for (ColorSlot &slot: scheme.keys()) {
        QString key = name(slot);
        if (key.startsWith("Icon_")) {
            res.insert(key.mid(5, key.length()-5), scheme.value(slot).color.name());
        }
    }
    return res;
}

QByteArray Scheme::colorizedContent(QString name)
{
    QFile file(name);
    if (!file.open(QFile::ReadOnly)) return QByteArray();
    QByteArray data = file.readAll();
    file.close();
    QHash<QString, QString>::const_iterator it = mIconCode.constBegin();
    while (it != mIconCode.constEnd()) {
        QString key = QString(".%1{fill:").arg(it.key());
        int from = data.indexOf(key);
        if (from >= 0) {
            from += key.length();
            int len = data.indexOf(";", from) - from;
            data.replace(from, len, it.value().toLatin1());
        }
        ++it;
    }
    return data;
}

void Scheme::invalidate()
{
    mIconCode = iconCodes();
    mIconCache.clear();
    mDataCache.clear();
    emit changed();
}

void Scheme::unbind(SvgEngine *engine)
{
    mEngines.removeAll(engine);
}

void Scheme::next()
{
    int index = (instance()->mActiveScheme + 1) % instance()->mSchemeNames.size();
    instance()->setActiveScheme(index);
}

QColor Scheme::color(Scheme::ColorSlot slot)
{
    return instance()->mColorSchemes.at(instance()->mActiveScheme).value(slot, CUndefined).color;
}

QIcon Scheme::icon(QString name)
{
    if (!instance()->mIconCache.contains(name)) {
        SvgEngine *eng = new SvgEngine(name);
        instance()->mEngines << eng;
        instance()->mIconCache.insert(name, QIcon(eng));
    }
    return instance()->mIconCache.value(name);
}

QByteArray &Scheme::data(QString name)
{
    if (!instance()->mDataCache.contains(name)) {
        QByteArray data(instance()->colorizedContent(name));
        instance()->mDataCache.insert(name, data);
    }
    return instance()->mDataCache[name];
}

bool Scheme::hasFlag(Scheme::ColorSlot slot, Scheme::FontFlag flag)
{
    Color cl = instance()->mColorSchemes.at(instance()->mActiveScheme).value(slot);
    if (flag == fNormal) return (cl.fontFlag == fNormal);
    return (FontFlag(flag & cl.fontFlag) == flag);
}

QByteArray Scheme::exportJsonColorSchemes()
{
    QJsonArray jsonSchemes;
    Q_ASSERT(mColorSchemes.length() == mSchemeNames.length());
    for (int i = 0; i < mColorSchemes.length(); ++i) {
        const QHash<ColorSlot, Color> &scheme = mColorSchemes.at(i);
        QJsonObject jsonScheme;
        QJsonObject slotObject;
        for (ColorSlot key: scheme.keys()) {
            QJsonObject dataObject;
            dataObject["color"] = scheme.value(key).color.name();
            if (scheme.value(key).fontFlag != fNormal)
                dataObject["type"] = int(scheme.value(key).fontFlag);
            slotObject[name(key)] = dataObject;
        }
        if (mActiveScheme == i) jsonScheme["Active"] = 1;
        jsonScheme["Name"] = mSchemeNames.at(i);
        jsonScheme["Scheme"] = slotObject;
        jsonSchemes.append(jsonScheme);
    }
    QJsonDocument saveDoc = QJsonDocument(jsonSchemes);
    return saveDoc.toJson(QJsonDocument::Indented);
}

void Scheme::importJsonColorSchemes(const QByteArray &jsonData)
{
    QJsonArray jsonSchemes = QJsonDocument::fromJson(jsonData).array();
    initDefault();
    for (int i = 0; i < jsonSchemes.size(); ++i) {
        QJsonObject jsonScheme = jsonSchemes[i].toObject();
        QString schemeName = jsonScheme["Name"].toString();
        int index = mSchemeNames.indexOf(schemeName);
        if (index < 0) {
            // No scheme for this name? Create it based on a copy of first scheme (ensures that all values exist)
            index = mSchemeNames.length();
            mSchemeNames << schemeName;
            mColorSchemes << mColorSchemes.at(0);
        }
        if (jsonScheme["Active"].toInt(0))
            mActiveScheme = index;
        if (jsonScheme.contains("Scheme") && jsonScheme["Scheme"].isObject()) {
            QJsonObject slotObject = jsonScheme["Scheme"].toObject();
            for (QString key: slotObject.keys()) {
                if (!slotObject[key].isObject()) continue;
                QJsonObject dataObject = slotObject[key].toObject();
                Color data = Color(QColor(dataObject["color"].toString()), FontFlag(dataObject["type"].toInt(0)));
                mColorSchemes[index].insert(slot(key), data);
            }
        }
    }
    invalidate();
}

} // namespace studio
} // namespace gams
