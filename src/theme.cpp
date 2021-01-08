#include "theme.h"
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

Theme *Theme::mInstance = nullptr;
const QColor CUndefined(255, 0, 200);

Theme::Theme(QObject *parent) : QObject(parent)
{
    mIconSet = "solid";
    initDefault();
    initSlotTexts();
}

Theme::~Theme()
{
    for (SvgEngine *eng: mEngines)
        eng->unbind();
    mEngines.clear();
}

Theme *Theme::instance()
{
    if (!mInstance) mInstance = new Theme();
    return mInstance;
}

void Theme::initSlotTexts()
{
    mSlotText.clear();
    mSlotText.insert(Edit_currentLineBg,        "Current line");
    mSlotText.insert(Edit_errorBg,              "Error");
    mSlotText.insert(Edit_currentWordBg,        "Current word");
    mSlotText.insert(Edit_matchesBg,            "Matches");
    mSlotText.insert(Edit_foldLineBg,           "Fold line");
    mSlotText.insert(Edit_parenthesesValidFg,   "Matching parentheses");
    mSlotText.insert(Edit_parenthesesInvalidFg, "Invalid parentheses");
    mSlotText.insert(Edit_linenrAreaFg,         "Line numbers");
    mSlotText.insert(Edit_linenrAreaMarkFg,     "Current line numbers");
    mSlotText.insert(Edit_foldLineFg,           "Folded line marker");

    mSlotText.insert(Icon_Gray,                 "Icon pen");
    mSlotText.insert(Icon_Back,                 "Icon brush");
    mSlotText.insert(Icon_Paper,                "Icon paper");

    mSlotText.insert(Syntax_assign,             "Assignment");
    mSlotText.insert(Syntax_comment,            "Comment");
    mSlotText.insert(Syntax_directive,          "Dollar-Control");
    mSlotText.insert(Syntax_directiveBody,      "Dollar-Control body");
    mSlotText.insert(Syntax_title,              "Title");
    mSlotText.insert(Syntax_keyword,            "Keyword");
    mSlotText.insert(Syntax_declaration,        "Declaration Keyword");
    mSlotText.insert(Syntax_identifier,         "Identifier");
    mSlotText.insert(Syntax_description,        "Description");
    mSlotText.insert(Syntax_identifierAssign,   "Identifier Declaration");
    mSlotText.insert(Syntax_assignLabel,        "Declaration Label");
    mSlotText.insert(Syntax_assignValue,        "Declaration Value");
    mSlotText.insert(Syntax_tableHeader,        "Table header");
    mSlotText.insert(Syntax_embedded,           "Embedded code");
}

void Theme::initDefault()
{
    mColorThemes.clear();
    mThemeNames.clear();
    mThemeBases.clear();

    // default to first color theme
    mTheme = 0;

    // Add first color theme
    int sNr = 0;
    mColorThemes << ColorTheme();
    mThemeNames << "Light";
    mThemeBases << 0;

    mColorThemes[sNr].clear();
    mColorThemes[sNr].insert(invalid,                        CUndefined);
    mColorThemes[sNr].insert(Edit_text,                      QColor(Qt::black));
    mColorThemes[sNr].insert(Syntax_neutral,                 QColor(Qt::black));
    mColorThemes[sNr].insert(Edit_background,                QColor(Qt::white));
    mColorThemes[sNr].insert(Edit_currentLineBg,             QColor(255, 250, 170));
    mColorThemes[sNr].insert(Edit_errorBg,                   QColor(255, 220, 200));
    mColorThemes[sNr].insert(Edit_currentWordBg,             QColor(210,200,200));
    mColorThemes[sNr].insert(Edit_matchesBg,                 QColor(22,164,25));
    mColorThemes[sNr].insert(Edit_foldLineBg,                QColor(200,200,200));
    mColorThemes[sNr].insert(Edit_foldLineFg,                QColor(255,255,255));
    mColorThemes[sNr].insert(Edit_parenthesesValidFg,        QColor(Qt::red));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidFg,      QColor(Qt::black));
    mColorThemes[sNr].insert(Edit_parenthesesValidBg,        QColor(Qt::green).lighter(170));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidBg,      QColor(Qt::red).lighter(150));
    mColorThemes[sNr].insert(Edit_parenthesesValidBgBlink,   QColor(Qt::green).lighter(130));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidBgBlink, QColor(Qt::red).lighter(115));
    mColorThemes[sNr].insert(Edit_linenrAreaBg,              QColor(245,245,245));
    mColorThemes[sNr].insert(Edit_linenrAreaMarkBg,          QColor(225,255,235));
    mColorThemes[sNr].insert(Edit_linenrAreaFoldBg,          QColor(135,195,255));
    mColorThemes[sNr].insert(Edit_linenrAreaFg,              QColor(Qt::gray));
    mColorThemes[sNr].insert(Edit_linenrAreaMarkFg,          QColor(Qt::black));

    mColorThemes[sNr].insert(Mark_errorFg,                   QColor(Qt::darkRed));
    mColorThemes[sNr].insert(Mark_listingFg,                 QColor(Qt::blue));
    mColorThemes[sNr].insert(Mark_fileFg,                    QColor(Qt::darkGreen));

    mColorThemes[sNr].insert(Icon_Gray,                      QColor(170,150,130));
    mColorThemes[sNr].insert(Icon_Back,                      QColor(51,68,85));
    mColorThemes[sNr].insert(Icon_Paper,                     QColor(Qt::white));
    mColorThemes[sNr].insert(Disable_Gray,                   QColor(180,180,175));
    mColorThemes[sNr].insert(Disable_Back,                   QColor(170,170,170));
    mColorThemes[sNr].insert(Active_Gray,                    QColor(54,122,195));
    mColorThemes[sNr].insert(Active_Back,                    QColor(34,102,170));
    mColorThemes[sNr].insert(Select_Gray,                    QColor(54,122,195));
    mColorThemes[sNr].insert(Select_Back,                    QColor(34,102,170));
    mColorThemes[sNr].insert(Normal_Red,                     QColor(187,0,0));
    mColorThemes[sNr].insert(Normal_Green,                   QColor(52,134,25));
    mColorThemes[sNr].insert(Normal_Blue,                    QColor(34,102,170));
    mColorThemes[sNr].insert(Normal_Yellow,                  QColor(Qt::darkYellow));

    mColorThemes[sNr].insert(Syntax_undefined,               CUndefined);
    mColorThemes[sNr].insert(Syntax_directive,               Color(QColor(Qt::darkMagenta).darker(120)));
    mColorThemes[sNr].insert(Syntax_assign,                  Color());
    mColorThemes[sNr].insert(Syntax_directiveBody,           Color(QColor(Qt::darkBlue).lighter(170), fItalic));
    mColorThemes[sNr].insert(Syntax_comment,                 Color(QColor(120, 120, 120), fItalic)); //QColor(150, 120, 65)
    mColorThemes[sNr].insert(Syntax_title,                   Color(QColor(Qt::darkBlue).lighter(140), fBold));
    mColorThemes[sNr].insert(Syntax_keyword,                 Color(QColor(Qt::darkBlue).lighter(140), fBold));
    mColorThemes[sNr].insert(Syntax_declaration,             Color(QColor(Qt::darkBlue).lighter(140), fBold));
    mColorThemes[sNr].insert(Syntax_identifier,              Color(QColor(Qt::black)));
    mColorThemes[sNr].insert(Syntax_description,             Color(QColor(Qt::darkBlue).lighter(170)));
    mColorThemes[sNr].insert(Syntax_identifierAssign,        Color(QColor(Qt::darkGreen).darker(140)));
    mColorThemes[sNr].insert(Syntax_assignLabel,             Color(QColor(Qt::darkGreen).darker(110)));
    mColorThemes[sNr].insert(Syntax_assignValue,             Color(QColor(0, 80, 120)));
    mColorThemes[sNr].insert(Syntax_tableHeader,             Color(QColor(Qt::darkGreen).darker(140), fBold));
    mColorThemes[sNr].insert(Syntax_embedded,                Color(QColor(200, 70, 0)));

    // Add and switch to second color theme
    mColorThemes << mColorThemes.at(sNr++);
    mThemeNames << "Dark";
    mThemeBases << 1;

    // Dark Colors
    QColor dark_highlight(243,150,25);      //QColor(243,150,25);
    QColor dark_id(153,240,255);            //QColor(153,240,255);
    QColor dark_assignment(144,226,149);    //QColor(144,226,149);
    QColor dark_unobstrusive(125,125,125);  //QColor(125,125,125)
    QColor dark_neutral(223,224,223);       //QColor(223,224,223);

    mColorThemes[sNr].insert(Edit_text,                      Color(dark_neutral));
    mColorThemes[sNr].insert(Syntax_neutral,                 Color(dark_neutral));
    mColorThemes[sNr].insert(Edit_background,                QColor(30,30,30));
    mColorThemes[sNr].insert(Edit_currentLineBg,             QColor(0,73,61));
    mColorThemes[sNr].insert(Edit_errorBg,                   QColor(187,34,51));
    mColorThemes[sNr].insert(Edit_matchesBg,                 QColor(51,102,51));
    mColorThemes[sNr].insert(Edit_foldLineBg,                QColor(80,80,80));
    mColorThemes[sNr].insert(Edit_foldLineFg,                QColor(0,0,0));
    mColorThemes[sNr].insert(Edit_linenrAreaBg,              QColor(16,16,16));
    mColorThemes[sNr].insert(Edit_linenrAreaMarkBg,          QColor(40,40,40));
    mColorThemes[sNr].insert(Edit_linenrAreaFoldBg,          QColor(15,75,115));
    mColorThemes[sNr].insert(Edit_linenrAreaMarkFg,          QColor(Qt::white));
    mColorThemes[sNr].insert(Edit_currentWordBg,             QColor(60,60,60));
    mColorThemes[sNr].insert(Edit_parenthesesValidFg,        QColor(Qt::yellow).lighter(170));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidFg,      QColor(Qt::black));
    mColorThemes[sNr].insert(Edit_parenthesesValidBg,        QColor(40,70,30));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidBg,      QColor(Qt::red).darker(170));
    mColorThemes[sNr].insert(Edit_parenthesesValidBgBlink,   QColor(60,90,50));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidBgBlink, QColor(Qt::red).darker(115));
    mColorThemes[sNr].insert(Syntax_title,             Color(dark_highlight, fBold));
    mColorThemes[sNr].insert(Syntax_directive,         QColor(200,60,90));
    mColorThemes[sNr].insert(Syntax_keyword,           Color(dark_highlight, fBold));
    mColorThemes[sNr].insert(Syntax_declaration,       Color(dark_highlight, fBold));
    mColorThemes[sNr].insert(Syntax_description,       Color(dark_unobstrusive));
    mColorThemes[sNr].insert(Syntax_comment,           Color(dark_unobstrusive, fItalic));
    mColorThemes[sNr].insert(Syntax_identifier,        Color(dark_id));
    mColorThemes[sNr].insert(Syntax_assign,            Color(dark_id));
    mColorThemes[sNr].insert(Syntax_identifierAssign,  Color(dark_assignment));
    mColorThemes[sNr].insert(Syntax_assignLabel,       Color(dark_assignment));
    mColorThemes[sNr].insert(Syntax_tableHeader,       Color(dark_assignment, fBold));
    mColorThemes[sNr].insert(Syntax_assignValue,       Color(dark_assignment.lighter()));
    mColorThemes[sNr].insert(Syntax_directiveBody,     Color(dark_highlight, fItalic));

    mColorThemes[sNr].insert(Icon_Gray,                QColor(65,55,50));
    mColorThemes[sNr].insert(Icon_Back,                QColor(220,220,220));
    mColorThemes[sNr].insert(Active_Back,              QColor(Qt::white));
    mColorThemes[sNr].insert(Disable_Back,             QColor(96,99,96));
    mColorThemes[sNr].insert(Normal_Red,               QColor(187,34,51));
    mColorThemes[sNr].insert(Normal_Green,             QColor(102,170,102));
    mColorThemes[sNr].insert(Normal_Blue,              QColor(68,153,238));

    invalidate();
}

QStringList Theme::themes()
{
    return mThemeNames;
}

int Theme::setActiveTheme(QString themeName)
{
    int theme = mThemeNames.indexOf(themeName);
    return setActiveTheme(theme);
}

int Theme::setActiveTheme(int theme)
{
    if (theme < 0 || theme >= mThemeNames.size()) return -1;
    mTheme = theme;
    invalidate();
    return theme;
}

QString Theme::renameActiveTheme(const QString &name)
{
    if (mTheme < 2) return mThemeNames.at(mTheme);
    if (name.compare(mThemeNames.at(mTheme)) == 0) return name;
    QString uniqueName = findUniqueName(name);
    mThemeNames.replace(mTheme, uniqueName);
    if (mTheme < mThemeNames.count()-1) {
        mThemeNames.move(mTheme, mThemeNames.count()-1);
        mThemeBases.move(mTheme, mThemeNames.count()-1);
        mColorThemes.move(mTheme, mThemeNames.count()-1);
        mTheme = mThemeNames.count()-1;
    }

    // restore sort order
    int i = mThemeNames.count() - 1;
    while (i > 2 && mThemeNames.at(i-1).compare(name, Qt::CaseInsensitive) > 0) --i;

    int last = mThemeNames.count() - 1;
    if (i < last) {
        mThemeNames.move(last, i);
        mThemeBases.move(last, i);
        mColorThemes.move(last, i);
        mTheme = i;
    }

    return uniqueName;
}

int Theme::activeTheme() const
{
    return mTheme;
}

QString Theme::activeThemeName()
{
    return mThemeNames.at(mTheme);
}

int Theme::baseTheme(int theme) const
{
    if (theme < 0 || theme >= mThemeBases.size()) return -1;
    return mThemeBases.at(theme);
}

QString Theme::name(Theme::ColorSlot slot)
{
    return QMetaEnum::fromType<ColorSlot>().valueToKey(slot);
}

QString Theme::text(Theme::ColorSlot slot)
{
    return instance()->mSlotText.value(slot);
}

bool Theme::hasFontProps(Theme::ColorSlot slot)
{
    return slot >= Syntax_undefined;
}

Theme::ColorSlot Theme::slot(QString name)
{
    bool ok;
    int value = QMetaEnum::fromType<ColorSlot>().keyToValue(name.toLatin1().data(), &ok);
    if (!ok) return invalid;
    return ColorSlot(value);
}

QHash<QString, QStringList> Theme::iconCodes() const
{
    QHash<QString, QStringList> set;
    const ColorTheme &theme = mColorThemes.at(mTheme);
    for (ColorSlot &slot: theme.keys()) {
        QString slotName = name(slot);
        if (slotName.startsWith("Icon_")) {
            QString key = slotName.mid(5, slotName.length()-5);
            set.insert(key, QStringList());
            for (int i = 0 ; i < 4 ; ++i)
                set[key] << theme.value(slot).color.name();
            set[key] << theme.value(Normal_Red).color.name();
            set[key] << theme.value(Normal_Green).color.name();
            set[key] << theme.value(Normal_Blue).color.name();
        }
    }
    for (ColorSlot &slot: theme.keys()) {
        QString slotName = name(slot);
        if (slotName.startsWith("Disable_")) {
            QString key = slotName.mid(8, slotName.length()-8);
            if (set.contains(key))
                set[key].replace(1, theme.value(slot).color.name());
        }
        if (slotName.startsWith("Active_")) {
            QString key = slotName.mid(7, slotName.length()-7);
            if (set.contains(key))
                set[key].replace(2, theme.value(slot).color.name());
        }
        if (slotName.startsWith("Select_")) {
            QString key = slotName.mid(7, slotName.length()-7);
            if (set.contains(key))
                set[key].replace(3, theme.value(slot).color.name());
        }
    }
    return set;
}

QByteArray Theme::colorizedContent(QString name, QIcon::Mode mode)
{
    QFile file(name);
    if (!file.open(QFile::ReadOnly)) return QByteArray();
    QByteArray data = file.readAll();
    file.close();

    int end = data.indexOf("</style");
    if (end < 0) return data;
    int iMode = int(mode);


    QHash<QString, QStringList> iconCode = mIconCodes;
    QHash<QString, QStringList>::const_iterator it = iconCode.constBegin(); // Icon_Gray + Icon_Back
    for ( ; it != iconCode.constEnd() ; ++it) {
        int start = data.indexOf("<style");
        while (start >= 0 && start < end) {
            QString key = QString(".%1").arg(it.key());
            int from = data.indexOf('.'+it.key().toUtf8(), start+1);
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

QString Theme::findUniqueName(const QString &name)
{
    if (!mThemeNames.contains(name)) return name;
    QString uniqueName = name;
    QString base = name;
    int nr = 0;
    while (!base.isEmpty() && base.at(base.length()-1).isDigit())
        base = base.left(base.length()-1);
    if (base.isEmpty()) base = name;
    if (base.length() != name.length()) nr = name.right(name.length()-base.length()).toInt();
    while (mThemeNames.contains(uniqueName))
        uniqueName = base + QString::number(++nr);
    return uniqueName;
}

QColor merge(QColor c1, QColor c2, qreal weight = 0.5)
{
    return QColor::fromRgbF((c1.redF()*weight + c2.redF()*(1-weight)),
                            (c1.greenF()*weight + c2.redF()*(1-weight)),
                            (c1.blueF()*weight + c2.blueF()*(1-weight)));
}

void Theme::invalidate()
{
    mIconCodes = iconCodes();
    mIconCache.clear();
    mDataCache.clear();

    emit changed();
}

void Theme::unbind(SvgEngine *engine)
{
    mEngines.removeAll(engine);
}

int Theme::copyTheme(int index, const QString &destName)
{
    QString name = findUniqueName(destName);
    mColorThemes << mColorThemes.at(index);
    mThemeNames << name;
    mThemeBases << mThemeBases.at(index);

    // restore sort order
    int i = mThemeNames.count() - 1;
    while (i > 2 && mThemeNames.at(i-1).compare(name, Qt::CaseInsensitive) > 0) --i;

    int last = mThemeNames.count() - 1;
    if (i < last) {
        mThemeNames.move(last, i);
        mThemeBases.move(last, i);
        mColorThemes.move(last, i);
    }

    return i;
}

int Theme::removeTheme(int index)
{
    if (index < 2 || index >= mThemeNames.count()) return mTheme;
    if (index <= mTheme) --mTheme;
    QString name = mThemeNames.at(index);
    mColorThemes.removeAt(index);
    mThemeNames.removeAt(index);
    mThemeBases.removeAt(index);
    return mTheme;
}

QColor Theme::color(Theme::ColorSlot slot)
{
    int theme = instance()->mTheme;
    return instance()->mColorThemes.at(theme).value(slot, CUndefined).color;
}

void Theme::setColor(Theme::ColorSlot slot, QColor color)
{
    int theme = instance()->mTheme;
    Color dat = instance()->mColorThemes.at(theme).value(slot);
    dat.color = color;
    instance()->mColorThemes[theme].insert(slot, dat);
}

QIcon Theme::icon(QString name, bool forceSquare, QString disabledName)
{
    if (name.contains("%")) name = name.arg(instance()->mIconSet);
    if (!instance()->mIconCache.contains(name)) {
        SvgEngine *eng = (disabledName.isEmpty() ? new SvgEngine(name) : new SvgEngine(name, disabledName));
        if (forceSquare) eng->forceSquare(true);
        instance()->mEngines << eng;
        instance()->mIconCache.insert(name, QIcon(eng));
    }
    return instance()->mIconCache.value(name);
}

QByteArray &Theme::data(QString name, QIcon::Mode mode)
{
    QStringList ext {"_N","_D","_A","_S"};
    QString nameKey = name + ext.at(int(mode));
    if (!instance()->mDataCache.contains(nameKey)) {
        QByteArray data(instance()->colorizedContent(name, mode));
        instance()->mDataCache.insert(nameKey, data);
    }
    return instance()->mDataCache[nameKey];
}

bool Theme::hasFlag(Theme::ColorSlot slot, Theme::FontFlag flag)
{
    int theme = instance()->mTheme;
    Color cl = instance()->mColorThemes.at(theme).value(slot);
    if (flag == fNormal) return (cl.fontFlag == fNormal);
    return (FontFlag(flag & cl.fontFlag) == flag);
}

void Theme::setFlags(Theme::ColorSlot slot, Theme::FontFlag flag)
{
    int theme = instance()->mTheme;
    Color dat = instance()->mColorThemes.at(theme).value(slot);
    dat.fontFlag = flag;
    instance()->mColorThemes[theme].insert(slot, dat);
}

QVariantList Theme::writeUserThemes() const
{
    QVariantList res;
    // starts with index i=2 (only export user settings, light and dark should be fixed)
    for (int i = 2; i < mColorThemes.length(); ++i) {
        QVariantMap resData;
        const QHash<ColorSlot, Color> &theme = mColorThemes.at(i);
        for (ColorSlot key = invalid; key < ColorSlotCount; key = static_cast<ColorSlot>(key+1)) {
            resData.insert(name(key), theme.value(key).color.name() + "," + QString::number(theme.value(key).fontFlag));
        }
        QVariantMap resTheme;
        resTheme.insert("name", mThemeNames.at(i));
        resTheme.insert("base", mThemeBases.at(i));
        resTheme.insert("theme", resData);
        res << resTheme;
    }
    return res;
}

void Theme::readUserThemes(const QVariantList &sourceThemes)
{
    // remove user defined themes
    while (mThemeBases.size() > 2) mThemeBases.removeLast();
    while (mThemeNames.size() > 2) mThemeNames.removeLast();
    while (mColorThemes.size() > 2) mColorThemes.removeLast();

    // add new user defined themes
    for (QVariant vSource: sourceThemes) {
        QVariantMap tSource = vSource.toMap();
        if (tSource.isEmpty() || !tSource.contains("name") || !tSource.contains("theme")) continue;
        QString name = tSource.value("name").toString();
        int base = tSource.value("base").toInt();

        // clone first theme as base
        int newInd = copyTheme(base, name);
        ColorTheme currentTheme = mColorThemes.at(newInd);

        QVariantMap sourceData = tSource.value("theme").toMap();
        for (auto it = sourceData.constBegin() ; it != sourceData.constEnd() ; ++it) {
            ColorSlot cSlot = slot(it.key());
            if (cSlot == invalid) continue;
            QStringList dat = it.value().toString().split(',');
            if (!dat.size()) continue;
            bool ok = true;
            int iFlag = dat.size() < 2 ? 0 : dat.at(1).toInt(&ok);
            Color color = Color(QColor(dat.at(0)), FontFlag(iFlag));
            currentTheme.insert(cSlot, color);
        }
        mColorThemes.replace(newInd, currentTheme);
    }
}

} // namespace studio
} // namespace gams
