#include "color.h"
#include <QHash>
#include <QMetaEnum>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

namespace gams {
namespace studio {

Color *Color::mInstance = nullptr;
const QColor CUndefined(QColor(255,0,200));

Color::Color(QObject *parent) : QObject(parent)
{

}

Color *Color::instance()
{
    if (!mInstance) mInstance = new Color();
    return mInstance;
}

void Color::initDefault()
{
    mColorSchemes.clear();
    mSchemeNames.clear();
    mColorSchemes << ColorSet();
    mSchemeNames << "Light";

    mColorSchemes[0].clear();
    mColorSchemes[0].insert(Edit_currentLineBg, QColor(255, 250, 170));
    mColorSchemes[0].insert(Edit_errorBg, QColor(Qt::lightGray));
    mColorSchemes[0].insert(Edit_currentWordBg, QColor(210,200,200));
    mColorSchemes[0].insert(Edit_matchesBg, QColor(Qt::green).lighter(160));
    mColorSchemes[0].insert(Edit_parenthesesValidFg, QColor(Qt::red));
    mColorSchemes[0].insert(Edit_parenthesesInvalidFg, QColor(Qt::black));
    mColorSchemes[0].insert(Edit_parenthesesValidBg, QColor(Qt::green).lighter(170));
    mColorSchemes[0].insert(Edit_parenthesesInvalidBg, QColor(Qt::red).lighter(150));
    mColorSchemes[0].insert(Edit_parenthesesValidBgBlink, QColor(Qt::green).lighter(130));
    mColorSchemes[0].insert(Edit_parenthesesInvalidBgBlink, QColor(Qt::red).lighter(115));
    mColorSchemes[0].insert(Edit_linenrAreaBg, QColor(245,245,245));
    mColorSchemes[0].insert(Edit_linenrAreaMarkBg, QColor(225,255,235));
    mColorSchemes[0].insert(Edit_linenrAreaFg, QColor(Qt::black));
    mColorSchemes[0].insert(Edit_linenrAreaMarkFg, QColor(Qt::gray));
    mColorSchemes[0].insert(Edit_blockSelectBg, QColor(Qt::cyan).lighter(150));

    mColorSchemes[0].insert(Mark_errorFg, QColor(Qt::darkRed));
    mColorSchemes[0].insert(Mark_listingFg, QColor(Qt::blue));
    mColorSchemes[0].insert(Mark_fileFg, QColor(Qt::darkGreen));

    mColorSchemes[0].insert(Syntax_undefined, CUndefined);
    mColorSchemes[0].insert(Syntax_directive, QColor(Qt::darkMagenta).darker(120));

//    mColorSchemes << ColorSet();
//    mSchemeNames << "Dark";
}

QStringList Color::schemes()
{
    return mSchemeNames;
}

int Color::setActiveScheme(QString schemeName)
{
    int scheme = mSchemeNames.indexOf(schemeName);
    if (scheme < 0 || scheme >= mSchemeNames.size()) return -1;
    mActiveScheme = scheme;
    emit colorsChanged();
    return mActiveScheme;
}

QString Color::setActiveScheme(int scheme)
{
    if (scheme < 0 || scheme >= mSchemeNames.size()) return QString();
    mActiveScheme = scheme;
    emit colorsChanged();
    return mSchemeNames.at(scheme);
}

QColor Color::get(Color::ColorSlot slot)
{
    return mColorSchemes.at(mActiveScheme).value(slot, CUndefined);
}

QString Color::name(Color::ColorSlot slot)
{
    return QMetaEnum::fromType<ColorSlot>().valueToKey(slot);
}

Color::ColorSlot Color::slot(QString name)
{
    bool ok;
    int value = QMetaEnum::fromType<ColorSlot>().keyToValue(name.toLatin1().data(), &ok);
    if (!ok) return invalid;
    return ColorSlot(value);
}

QByteArray Color::exportJsonColorSchemes()
{
    QJsonArray jsonSchemes;
    Q_ASSERT(mColorSchemes.length() == mSchemeNames.length());
    for (int i = 0; i < mColorSchemes.length(); ++i) {
        const QHash<ColorSlot, QColor> &scheme = mColorSchemes.at(i);
        QJsonObject jsonScheme;
        QJsonObject colorObject;
        for (ColorSlot key: scheme.keys()) {
            colorObject[name(key)] = scheme.value(key).name();
        }
        if (mActiveScheme == i) jsonScheme["Active"] = 1;
        jsonScheme["Name"] = mSchemeNames.at(i);
        jsonScheme["Scheme"] = colorObject;
        jsonSchemes.append(jsonScheme);
    }
    QJsonDocument saveDoc = QJsonDocument(jsonSchemes);
    return saveDoc.toJson(QJsonDocument::Indented);
}

void Color::importJsonColorSchemes(const QByteArray &jsonData)
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
            QJsonObject colorObject = jsonScheme["Scheme"].toObject();
            for (QString key: colorObject.keys()) {
                mColorSchemes[index].insert(slot(key), QColor(colorObject[key].toString()));
            }
        }
    }
}

} // namespace studio
} // namespace gams
