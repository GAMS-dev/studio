#include "scheme.h"
#include <QHash>
#include "logger.h"
#include "svgengine.h"
#include <QMetaEnum>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QGuiApplication>

namespace gams {
namespace studio {

Scheme *Scheme::mInstance = nullptr;
const QColor CUndefined(255, 0, 200);

Scheme::Scheme(QObject *parent) : QObject(parent)
{
    mIconSet = "solid"; // thin, solid
    initDefault();
    initSlotTexts();
}

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

void Scheme::initSlotTexts()
{
    mSlotText.clear();
    mSlotText.insert(Edit_currentLineBg,        "Current line");
    mSlotText.insert(Edit_errorBg,              "Error");
    mSlotText.insert(Edit_currentWordBg,        "Current word");
    mSlotText.insert(Edit_matchesBg,            "matches");
    mSlotText.insert(Edit_parenthesesValidFg,   "Matching parentheses");
    mSlotText.insert(Edit_parenthesesInvalidFg, "Invalid parentheses");
    mSlotText.insert(Edit_linenrAreaFg,         "Line numbers");
    mSlotText.insert(Edit_linenrAreaMarkFg,     "Current line numbers");

    mSlotText.insert(Icon_Line,                 "Icon pen");
    mSlotText.insert(Icon_Back,                 "Icon brush");
    mSlotText.insert(Icon_Paper,                "Icon paper");

    mSlotText.insert(Syntax_assign,             "Assignment");
    mSlotText.insert(Syntax_comment,            "Comment");
    mSlotText.insert(Syntax_directive,          "Dollar-Command");
    mSlotText.insert(Syntax_directiveBody,      "Dollar-Command body");
    mSlotText.insert(Syntax_title,              "Title");
    mSlotText.insert(Syntax_keyword,            "Keyword");
    mSlotText.insert(Syntax_declaration,        "Declaration");
    mSlotText.insert(Syntax_identifier,         "Identifier");
    mSlotText.insert(Syntax_description,        "Description");
    mSlotText.insert(Syntax_identifierAssign,   "Identifier assignment");
    mSlotText.insert(Syntax_assignLabel,        "Assignment label");
    mSlotText.insert(Syntax_assignValue,        "Assignment value");
    mSlotText.insert(Syntax_tableHeader,        "Table header");
    mSlotText.insert(Syntax_embedded,           "Embedded code");
}

void Scheme::initDefault()
{
    mColorSchemes.clear();
    mSchemeNames.clear();

    // Add and switch to first color scheme
    mActiveScheme = 0;
    int sNr = 0;
    mColorSchemes << ColorScheme();
    mSchemeNames << "Light";

    mColorSchemes[sNr].clear();
    mColorSchemes[sNr].insert(invalid,                        CUndefined);
    mColorSchemes[sNr].insert(Edit_text,                      QColor(Qt::black));
    mColorSchemes[sNr].insert(Syntax_neutral,                 QColor(Qt::black));
    mColorSchemes[sNr].insert(Edit_background,                QColor(Qt::white));
    mColorSchemes[sNr].insert(Edit_currentLineBg,             QColor(255, 250, 170));
    mColorSchemes[sNr].insert(Edit_errorBg,                   QColor(255, 220, 200));
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

    mColorSchemes[sNr].insert(Mark_errorFg,                   QColor(Qt::darkRed));
    mColorSchemes[sNr].insert(Mark_listingFg,                 QColor(Qt::blue));
    mColorSchemes[sNr].insert(Mark_fileFg,                    QColor(Qt::darkGreen));

    mColorSchemes[sNr].insert(Icon_Line,                      QColor(51,68,85));
    mColorSchemes[sNr].insert(Icon_Back,                      QColor(51,68,85));
    mColorSchemes[sNr].insert(Icon_Paper,                     QColor(Qt::white));
    mColorSchemes[sNr].insert(Disable_Line,                   QColor(170,170,170));
    mColorSchemes[sNr].insert(Disable_Back,                   QColor(170,170,170));
    mColorSchemes[sNr].insert(Active_Line,                    QColor(34,102,170));
    mColorSchemes[sNr].insert(Active_Back,                    QColor(34,102,170));
    mColorSchemes[sNr].insert(Select_Line,                    QColor(34,102,170));
    mColorSchemes[sNr].insert(Select_Back,                    QColor(34,102,170));
    mColorSchemes[sNr].insert(Normal_Red,                     QColor(187,0,0));
    mColorSchemes[sNr].insert(Normal_Green,                   QColor(52,134,25));
    mColorSchemes[sNr].insert(Normal_Blue,                    QColor(34,102,170));

    mColorSchemes[sNr].insert(Syntax_undefined,               CUndefined);
    mColorSchemes[sNr].insert(Syntax_directive,               Color(QColor(Qt::darkMagenta).darker(120)));
    mColorSchemes[sNr].insert(Syntax_assign,                  Color());
    mColorSchemes[sNr].insert(Syntax_directiveBody,           Color(QColor(Qt::darkBlue).lighter(170), fItalic));
    mColorSchemes[sNr].insert(Syntax_comment,                 Color(QColor(80, 145, 75), fItalic));
    mColorSchemes[sNr].insert(Syntax_title,                   Color(QColor(Qt::darkBlue).lighter(140), fBold));
    mColorSchemes[sNr].insert(Syntax_keyword,                 Color(QColor(Qt::darkBlue).lighter(140), fBold));
    mColorSchemes[sNr].insert(Syntax_declaration,             Color(QColor(Qt::darkBlue).lighter(140), fBold));
    mColorSchemes[sNr].insert(Syntax_identifier,              Color(QColor(Qt::black)));
    mColorSchemes[sNr].insert(Syntax_description,             Color(QColor(Qt::darkBlue).lighter(170)));
    mColorSchemes[sNr].insert(Syntax_identifierAssign,        Color(QColor(Qt::darkGreen).darker(140)));
    mColorSchemes[sNr].insert(Syntax_assignLabel,             Color(QColor(Qt::darkGreen).darker(110)));
    mColorSchemes[sNr].insert(Syntax_assignValue,             Color(QColor(0, 80, 120)));
    mColorSchemes[sNr].insert(Syntax_tableHeader,             Color(QColor(Qt::darkGreen).darker(140), fBold));
    mColorSchemes[sNr].insert(Syntax_embedded,                Color(QColor(200, 70, 0)));

    // Add and switch to second color scheme
    mColorSchemes << mColorSchemes.at(sNr++);
    mSchemeNames << "Dark";

    // Dark Colors
    QColor dark_highlight(243,150,25);      //QColor(243,150,25);
    QColor dark_id(153,240,255);            //QColor(153,240,255);
    QColor dark_assignment(144,226,149);    //QColor(144,226,149);
    QColor dark_unobstrusive(191, 195, 186);//QColor(191,195,186);
    QColor dark_neutral(223,224,223);       //QColor(223,224,223);

    mColorSchemes[sNr].insert(Edit_text,                Color(dark_neutral));
    mColorSchemes[sNr].insert(Syntax_neutral,           Color(dark_neutral));
    mColorSchemes[sNr].insert(Edit_background,          QColor(30,30,30));
    mColorSchemes[sNr].insert(Edit_currentLineBg,       QColor(0,146,122));
    mColorSchemes[sNr].insert(Edit_errorBg,             QColor(187,34,51));
    mColorSchemes[sNr].insert(Edit_matchesBg,           QColor(51,102,51));
    mColorSchemes[sNr].insert(Edit_linenrAreaBg,        QColor(16,16,16));
    mColorSchemes[sNr].insert(Edit_linenrAreaMarkBg,    QColor(40,40,40));
    mColorSchemes[sNr].insert(Edit_linenrAreaMarkFg,    QColor(Qt::white));
    mColorSchemes[sNr].insert(Edit_currentWordBg,       QColor(60,60,60));

    mColorSchemes[sNr].insert(Syntax_title,             Color(dark_highlight, fBold));
    mColorSchemes[sNr].insert(Syntax_directive,         Color(dark_assignment));
    mColorSchemes[sNr].insert(Syntax_keyword,           Color(dark_neutral, fBold));
    mColorSchemes[sNr].insert(Syntax_declaration,       Color(dark_highlight, fBold));
    mColorSchemes[sNr].insert(Syntax_description,       Color(dark_unobstrusive));
    mColorSchemes[sNr].insert(Syntax_comment,           Color(dark_unobstrusive, fItalic));
    mColorSchemes[sNr].insert(Syntax_identifier,        Color(dark_id));
    mColorSchemes[sNr].insert(Syntax_assign,            Color(dark_id));
    mColorSchemes[sNr].insert(Syntax_identifierAssign,  Color(dark_assignment));
    mColorSchemes[sNr].insert(Syntax_assignLabel,       Color(dark_assignment));
    mColorSchemes[sNr].insert(Syntax_tableHeader,       Color(dark_assignment, fBold));
    mColorSchemes[sNr].insert(Syntax_assignValue,       Color(dark_assignment.lighter()));
    mColorSchemes[sNr].insert(Syntax_directiveBody,     Color(dark_highlight, fItalic));

    mColorSchemes[sNr].insert(Icon_Back,                QColor(dark_neutral));
    mColorSchemes[sNr].insert(Disable_Back,             QColor(51,68,85));
    mColorSchemes[sNr].insert(Normal_Red,               QColor(187,34,51));
    mColorSchemes[sNr].insert(Normal_Green,             QColor(102,170,102));
    mColorSchemes[sNr].insert(Normal_Blue,              QColor(68,153,238));

    invalidate();
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
    if (scheme < 0 || scheme >= mSchemeNames.size()) return -1;
    mActiveScheme = scheme;
    invalidate();
    return mActiveScheme;
}

int Scheme::activeScheme() const
{
    return mActiveScheme;
}

void Scheme::setIconSet(Scheme::IconSet iconSet)
{
    switch (iconSet) {
    case ThinIcons: mIconSet = "thin"; break;
    case SolidIcons: mIconSet = "solid"; break;
    }
    invalidate();
}

QString Scheme::name(Scheme::ColorSlot slot)
{
    return QMetaEnum::fromType<ColorSlot>().valueToKey(slot);
}

QString Scheme::text(Scheme::ColorSlot slot)
{
    return instance()->mSlotText.value(slot);
}

bool Scheme::hasFontProps(Scheme::ColorSlot slot)
{
    return slot >= Syntax_undefined;
}

Scheme::ColorSlot Scheme::slot(QString name)
{
    bool ok;
    int value = QMetaEnum::fromType<ColorSlot>().keyToValue(name.toLatin1().data(), &ok);
    if (!ok) return invalid;
    return ColorSlot(value);
}

QHash<QString, QStringList> Scheme::iconCodes() const
{
    QHash<QString, QStringList> res;
    const ColorScheme &scheme = mColorSchemes.at(mActiveScheme);
    for (ColorSlot &slot: scheme.keys()) {
        QString slotName = name(slot);
        if (slotName.startsWith("Icon_")) {
            QString key = slotName.mid(5, slotName.length()-5);
            res.insert(key, QStringList());
            for (int i = 0 ; i < 4 ; ++i)
                res[key] << scheme.value(slot).color.name();
            res[key] << scheme.value(Normal_Red).color.name();
            res[key] << scheme.value(Normal_Green).color.name();
            res[key] << scheme.value(Normal_Blue).color.name();
        }
    }
    for (ColorSlot &slot: scheme.keys()) {
        QString slotName = name(slot);
        if (slotName.startsWith("Disable_")) {
            QString key = slotName.mid(8, slotName.length()-8);
            if (res.contains(key))
                res[key].replace(1, scheme.value(slot).color.name());
        }
        if (slotName.startsWith("Active_")) {
            QString key = slotName.mid(7, slotName.length()-7);
            if (res.contains(key))
                res[key].replace(2, scheme.value(slot).color.name());
        }
        if (slotName.startsWith("Select_")) {
            QString key = slotName.mid(7, slotName.length()-7);
            if (res.contains(key))
                res[key].replace(3, scheme.value(slot).color.name());
        }
    }
    return res;
}

QByteArray Scheme::colorizedContent(QString name, QIcon::Mode mode)
{
    QFile file(name);
    if (!file.open(QFile::ReadOnly)) return QByteArray();
    QByteArray data = file.readAll();
    file.close();

    int end = data.indexOf("</style");
    if (end < 0) return data;
    int iMode = int(mode);

    QHash<QString, QStringList>::const_iterator it = mIconCode.constBegin(); // Icon_Line + Icon_Back
    for ( ; it != mIconCode.constEnd() ; ++it) {
        int start = data.indexOf("<style");
        while (start >= 0 && start < end) {
            QString key = QString(".%1").arg(it.key());
            int from = data.indexOf('.'+it.key(), start+1);
            if (from < 0 || from+10 > end) break;
            start = from;
            QString colorCode = it.value().at(iMode);
            from += key.length();
            if (mode == QIcon::Normal) {
                if (data.at(from) == 'R') colorCode = it.value().at(4);
                if (data.at(from) == 'G') colorCode = it.value().at(5);
                if (data.at(from) == 'B') colorCode = it.value().at(6);
            }
            while (data.length() > from && data.at(from) != '{') ++from;
            if (data.indexOf("fill:", from) != from+1) continue;
            from += 6;
            int len = data.indexOf(";}", from) - from;
            data.replace(from, len, colorCode.toLatin1());
//            DEB() << name << " [" << from << ", " << (end) << "] \n" << data;
        }
    }
    return data;
}

QColor merge(QColor c1, QColor c2, qreal weight = 0.5)
{
    return QColor::fromRgbF((c1.redF()*weight + c2.redF()*(1-weight)),
                            (c1.greenF()*weight + c2.redF()*(1-weight)),
                            (c1.blueF()*weight + c2.blueF()*(1-weight)));
}

void generatePalette(QPalette &pal, const QColor &line, const QColor &back)
{
    int h, s, v;
    back.getHsv(&h, &s, &v);
    // inactive and active are the same..
    const QColor buttonBrushDark = QColor(back.darker());
    const QColor buttonBrushDark150 = QColor(back.darker(150));
    const QColor buttonBrushLight150 = QColor(back.lighter(150));
    pal.setColorGroup(QPalette::Active, line, back, buttonBrushLight150,
                      buttonBrushDark, buttonBrushDark150, line, line,
                      back, back);
    pal.setColorGroup(QPalette::Inactive, line, back, buttonBrushLight150,
                      buttonBrushDark, buttonBrushDark150, line, line,
                      back, back);
    pal.setColorGroup(QPalette::Disabled, buttonBrushDark, back, buttonBrushLight150,
                      buttonBrushDark, buttonBrushDark150, buttonBrushDark, line,
                      back, back);
}

void Scheme::invalidate()
{
    mIconCode = iconCodes();
    mIconCache.clear();
    mDataCache.clear();
    mPalette = qApp->palette();
//    generatePalette(mPalette, color(Icon_Line), color(Icon_Back));
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

void Scheme::setColor(Scheme::ColorSlot slot, QColor color)
{
    Color dat = instance()->mColorSchemes.at(instance()->mActiveScheme).value(slot);
    dat.color = color;
    instance()->mColorSchemes[instance()->mActiveScheme].insert(slot, dat);
}

QIcon Scheme::icon(QString name, bool forceSquare)
{
    if (name.contains("%")) name = name.arg(instance()->mIconSet);
    if (!instance()->mIconCache.contains(name)) {
        SvgEngine *eng = new SvgEngine(name);
        if (forceSquare) eng->forceSquare(true);
        instance()->mEngines << eng;
        instance()->mIconCache.insert(name, QIcon(eng));
    }
    return instance()->mIconCache.value(name);
}

QByteArray &Scheme::data(QString name, QIcon::Mode mode)
{
    QStringList ext {"_N","_D","_A","_S"};
    QString nameKey = name + ext.at(int(mode));
    if (!instance()->mDataCache.contains(nameKey)) {
        QByteArray data(instance()->colorizedContent(name, mode));
        instance()->mDataCache.insert(nameKey, data);
    }
    return instance()->mDataCache[nameKey];
}

bool Scheme::hasFlag(Scheme::ColorSlot slot, Scheme::FontFlag flag)
{
    Color cl = instance()->mColorSchemes.at(instance()->mActiveScheme).value(slot);
    if (flag == fNormal) return (cl.fontFlag == fNormal);
    return (FontFlag(flag & cl.fontFlag) == flag);
}

void Scheme::setFlags(Scheme::ColorSlot slot, Scheme::FontFlag flag)
{
    Color dat = instance()->mColorSchemes.at(instance()->mActiveScheme).value(slot);
    dat.fontFlag = flag;
    instance()->mColorSchemes[instance()->mActiveScheme].insert(slot, dat);
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
        if (jsonScheme["Active"].toInt(0)) mActiveScheme = index;
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
