#ifndef GAMS_STUDIO_THEME_H
#define GAMS_STUDIO_THEME_H

#include <QObject>
#include <QColor>
#include <QBrush>
#include <QIcon>
#include <QPalette>

namespace gams {
namespace studio {

class SvgEngine;

class Theme : public QObject
{
    Q_OBJECT
public:
    enum ColorSlot {
        invalid = 0,

        Edit_text,
        Edit_background,
        Edit_currentLineBg,
        Edit_errorBg,
        Edit_currentWordBg,
        Edit_matchesBg,
        Edit_foldLineBg,
        Edit_foldLineFg,
        Edit_parenthesesValidFg,
        Edit_parenthesesInvalidFg,
        Edit_parenthesesValidBg,
        Edit_parenthesesInvalidBg,
        Edit_parenthesesValidBgBlink,
        Edit_parenthesesInvalidBgBlink,
        Edit_linenrAreaBg,
        Edit_linenrAreaMarkBg,
        Edit_linenrAreaFoldBg,
        Edit_linenrAreaFg,
        Edit_linenrAreaMarkFg,

        Mark_errorFg,
        Mark_listingFg,
        Mark_fileFg,

        Icon_Back,
        Icon_Gray,
        Icon_Paper,
        Disable_Gray,
        Disable_Back,
        Active_Gray,
        Active_Back,
        Select_Gray,
        Select_Back,
        Normal_Red,
        Normal_Green,
        Normal_Blue,
        Normal_Yellow,

        Syntax_undefined,
        Syntax_neutral,
        Syntax_directive,
        Syntax_assign,
        Syntax_directiveBody,
        Syntax_comment,
        Syntax_title,
        Syntax_keyword,
        Syntax_declaration,
        Syntax_identifier,
        Syntax_description,
        Syntax_identifierAssign,
        Syntax_assignLabel,
        Syntax_assignValue,
        Syntax_tableHeader,
        Syntax_embedded,

        ColorSlotCount
    };
    Q_ENUM(ColorSlot)

    enum FontFlag {fNormal, fBold, fItalic, fBoldItalic};

    enum IconSet {ThinIcons, SolidIcons};

    enum Scope {StudioScope, EditorScope};
    Q_ENUM(Scope)

private:
    struct Color {
        Color(QColor _color = QColor(), FontFlag _fontFlag = fNormal) : color(_color), fontFlag(_fontFlag) {}
        QColor color;
        FontFlag fontFlag;
    };

public:
    ~Theme();
    static Theme *instance();
    void initDefault();
    int themeCount() { return mThemeNames.size(); }
    QStringList themes();
    int setActiveTheme(QString themeName, Scope scope = Theme::EditorScope);
    int setActiveTheme(int theme, Scope scope = Theme::EditorScope);
    int activeTheme(Scope scope) const;
    ColorSlot slot(QString name);
    void invalidate();
    void unbind(SvgEngine *engine);
    bool isValidScope(int scopeValue);
    int copyTheme(int index);

    QVariantList exportThemes();
    void importThemes(const QVariantList &syntaxThemes);

    static QList<Scope> scopes();
    static QString name(ColorSlot slot);
    static QString text(ColorSlot slot);
    static bool hasFontProps(ColorSlot slot);
    static QColor color(ColorSlot slot, Theme::Scope scope = Theme::EditorScope);
    static void setColor(ColorSlot slot, gams::studio::Theme::Scope scope, QColor color);
    static QIcon icon(QString name, Scope scope, bool forceSquare = false, QString disabledName = QString());
    static QIcon icon(QString name, bool forceSquare = false, QString disabledName = QString());
    static QByteArray &data(QString name, Scope scope, QIcon::Mode mode);
    static bool hasFlag(ColorSlot slot, FontFlag flag, Theme::Scope scope = Theme::EditorScope);
    static void setFlags(ColorSlot slot, FontFlag flag, Theme::Scope scope = Theme::EditorScope);


signals:
    void changed();

private:
    explicit Theme(QObject *parent = nullptr);
    void initSlotTexts();
    QList<QHash<QString, QStringList>> iconCodes() const;
    QByteArray colorizedContent(QString name, Scope scope, QIcon::Mode mode = QIcon::Normal);

private:
    static Theme *mInstance;
    typedef QHash<ColorSlot, Color> ColorTheme;
    QList<ColorTheme> mColorThemes;
    QHash<ColorSlot, QString> mSlotText;
    QStringList mThemeNames;
    QString mIconSet;
    QHash<Scope, int> mScopeTheme;
    QList<QHash<QString, QStringList>> mIconCodes;
    QHash<QString, QIcon> mIconCache;
    QHash<QString, QByteArray> mDataCache;
    QVector<SvgEngine*> mEngines;
};

inline QColor toColor(Theme::ColorSlot code, Theme::Scope scope = Theme::EditorScope) {
    return Theme::color(code, scope); }
inline QString name(Theme::ColorSlot col) { return Theme::instance()->name(col); }

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_THEME_H
