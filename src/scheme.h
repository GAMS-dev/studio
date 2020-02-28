#ifndef GAMS_STUDIO_SCHEME_H
#define GAMS_STUDIO_SCHEME_H

#include <QObject>
#include <QColor>
#include <QBrush>
#include <QIcon>
#include <QPalette>

namespace gams {
namespace studio {

class SvgEngine;

class Scheme : public QObject
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
        Edit_parenthesesValidFg,
        Edit_parenthesesInvalidFg,
        Edit_parenthesesValidBg,
        Edit_parenthesesInvalidBg,
        Edit_parenthesesValidBgBlink,
        Edit_parenthesesInvalidBgBlink,
        Edit_linenrAreaBg,
        Edit_linenrAreaMarkBg,
        Edit_linenrAreaFg,
        Edit_linenrAreaMarkFg,

        Mark_errorFg,
        Mark_listingFg,
        Mark_fileFg,

        Icon_Back,
        Icon_Line,
        Icon_Paper,
        Disable_Line,
        Disable_Back,
        Active_Line,
        Active_Back,
        Select_Line,
        Select_Back,
        Normal_Red,
        Normal_Green,
        Normal_Blue,

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
    ~Scheme();
    static Scheme *instance();
    void initDefault();
    int schemeCount() { return mSchemeNames.size(); }
    QStringList schemes();
    int setActiveScheme(QString schemeName, Scope scope);
    int setActiveScheme(int scheme, Scope scope);
    int activeScheme(Scope scope) const;
    void setIconSet(IconSet iconSet = ThinIcons);
    ColorSlot slot(QString name);
    void invalidate();
    void unbind(SvgEngine *engine);
    bool isValidScope(int scopeValue);

    QByteArray exportJsonColorSchemes();
    void importJsonColorSchemes(const QByteArray &jsonData);

    static QString name(ColorSlot slot);
    static QString text(ColorSlot slot);
    static bool hasFontProps(ColorSlot slot);
    static QColor color(ColorSlot slot, Scheme::Scope scope = Scheme::EditorScope);
    static void setColor(ColorSlot slot, gams::studio::Scheme::Scope scope, QColor color);
    static QIcon icon(QString name, Scope scope, bool forceSquare = false);
    static QIcon icon(QString name, bool forceSquare = false);
    static QByteArray &data(QString name, Scope scope, QIcon::Mode mode);
    static bool hasFlag(ColorSlot slot, FontFlag flag, Scheme::Scope scope = Scheme::EditorScope);
    static void setFlags(ColorSlot slot, FontFlag flag, Scheme::Scope scope = Scheme::EditorScope);


signals:
    void changed();

private:
    explicit Scheme(QObject *parent = nullptr);
    void initSlotTexts();
    QHash<QString, QStringList> iconCodes() const;
    QByteArray colorizedContent(QString name, Scope scope, QIcon::Mode mode = QIcon::Normal);

private:
    static Scheme *mInstance;
    typedef QHash<ColorSlot, Color> ColorScheme;
    QList<ColorScheme> mColorSchemes;
    QHash<ColorSlot, QString> mSlotText;
    QStringList mSchemeNames;
    QString mIconSet;
    QHash<Scope, int> mScopeScheme;
    QHash<QString, QStringList> mIconCode;
    QHash<QString, QIcon> mIconCache;
    QHash<QString, QByteArray> mDataCache;
    QVector<SvgEngine*> mEngines;
};

inline QString makeKey(Scheme::Scope scope, QString text) { return QString("%1@%2").arg(scope).arg(text); }
inline QColor toColor(Scheme::ColorSlot code, Scheme::Scope scope = Scheme::EditorScope) {
    return Scheme::color(code, scope); }
inline QString name(Scheme::ColorSlot col) { return Scheme::instance()->name(col); }

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_SCHEME_H
