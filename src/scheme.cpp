#include "scheme.h"
#include <QHash>
#include "logger.h"
#include <QMetaEnum>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

namespace gams {
namespace studio {

Scheme *Scheme::mInstance = nullptr;
const QColor CUndefined(255,0,200);

Scheme::Scheme(QObject *parent) : QObject(parent)
{}

Scheme *Scheme::instance()
{
    if (!mInstance) mInstance = new Scheme();
    return mInstance;
}

void Scheme::initDefault()
{
    mColorSchemes.clear();
    mSchemeNames.clear();

    mColorSchemes << ColorScheme();
    mSchemeNames << "Standard";
    mColorSchemes[0].clear();
    mColorSchemes[0].insert(invalid,                        CUndefined);
    mColorSchemes[0].insert(Edit_currentLineBg,             QColor(255, 250, 170));
    mColorSchemes[0].insert(Edit_errorBg,                   QColor(Qt::lightGray));
    mColorSchemes[0].insert(Edit_currentWordBg,             QColor(210,200,200));
    mColorSchemes[0].insert(Edit_matchesBg,                 QColor(Qt::green).lighter(160));
    mColorSchemes[0].insert(Edit_parenthesesValidFg,        QColor(Qt::red));
    mColorSchemes[0].insert(Edit_parenthesesInvalidFg,      QColor(Qt::black));
    mColorSchemes[0].insert(Edit_parenthesesValidBg,        QColor(Qt::green).lighter(170));
    mColorSchemes[0].insert(Edit_parenthesesInvalidBg,      QColor(Qt::red).lighter(150));
    mColorSchemes[0].insert(Edit_parenthesesValidBgBlink,   QColor(Qt::green).lighter(130));
    mColorSchemes[0].insert(Edit_parenthesesInvalidBgBlink, QColor(Qt::red).lighter(115));
    mColorSchemes[0].insert(Edit_linenrAreaBg,              QColor(245,245,245));
    mColorSchemes[0].insert(Edit_linenrAreaMarkBg,          QColor(225,255,235));
    mColorSchemes[0].insert(Edit_linenrAreaFg,              QColor(Qt::black));
    mColorSchemes[0].insert(Edit_linenrAreaMarkFg,          QColor(Qt::gray));
    mColorSchemes[0].insert(Edit_blockSelectBg,             QColor(Qt::cyan).lighter(150));

    mColorSchemes[0].insert(Mark_errorFg,                   QColor(Qt::darkRed));
    mColorSchemes[0].insert(Mark_listingFg,                 QColor(Qt::blue));
    mColorSchemes[0].insert(Mark_fileFg,                    QColor(Qt::darkGreen));

    mColorSchemes[0].insert(Icon_Main,                      QColor(Qt::white));
    mColorSchemes[0].insert(Icon_Invers,                    QColor(Qt::black));

    mColorSchemes[0].insert(Syntax_undefined,               CUndefined);
    mColorSchemes[0].insert(Syntax_neutral,                 Color());
    mColorSchemes[0].insert(Syntax_directive,               Color(QColor(Qt::darkMagenta).darker(120)));
    mColorSchemes[0].insert(Syntax_assign,                  Color());
    mColorSchemes[0].insert(Syntax_directiveBody,           Color(QColor(Qt::darkBlue).lighter(170), fItalic));
    mColorSchemes[0].insert(Syntax_comment,                 Color(QColor(80, 145, 75), fItalic));
    mColorSchemes[0].insert(Syntax_title,                   Color(QColor(Qt::darkBlue).lighter(140), fBoldItalic));
    mColorSchemes[0].insert(Syntax_keyword,                 Color(QColor(Qt::darkBlue).lighter(140), fBold));
    mColorSchemes[0].insert(Syntax_declaration,             Color(QColor(Qt::darkBlue).lighter(140), fBold));
    mColorSchemes[0].insert(Syntax_identifier,              Color(QColor(Qt::black)));
    mColorSchemes[0].insert(Syntax_description,             Color(QColor(Qt::darkBlue).lighter(170)));
    mColorSchemes[0].insert(Syntax_identifierAssign,        Color(QColor(Qt::darkGreen).darker(140)));
    mColorSchemes[0].insert(Syntax_assignLabel,             Color(QColor(Qt::darkGreen).darker(110)));
    mColorSchemes[0].insert(Syntax_assignValue,             Color(QColor(0, 80, 120)));
    mColorSchemes[0].insert(Syntax_tableHeader,             Color(QColor(Qt::darkGreen).darker(140), fBold));
    mColorSchemes[0].insert(Syntax_embedded,                Color(QColor(200, 70, 0)));

//    mColorSchemes << ColorScheme();
//    mSchemeNames << "Dark";

}

QStringList Scheme::schemes()
{
    return mSchemeNames;
}

int Scheme::setActiveScheme(QString schemeName)
{
    int scheme = mSchemeNames.indexOf(schemeName);
    if (scheme < 0 || scheme >= mSchemeNames.size()) return -1;
    mActiveScheme = scheme;
    emit colorsChanged();
    return mActiveScheme;
}

QString Scheme::setActiveScheme(int scheme)
{
    if (scheme < 0 || scheme >= mSchemeNames.size()) return QString();
    mActiveScheme = scheme;
    emit colorsChanged();
    return mSchemeNames.at(scheme);
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

QColor Scheme::color(Scheme::ColorSlot slot)
{
    return instance()->mColorSchemes.at(instance()->mActiveScheme).value(slot, CUndefined).color;
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
}

} // namespace studio
} // namespace gams
