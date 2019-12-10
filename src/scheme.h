#ifndef GAMS_STUDIO_SCHEME_H
#define GAMS_STUDIO_SCHEME_H

#include <QObject>
#include <QColor>
#include <QBrush>
#include <QIcon>

namespace gams {
namespace studio {

class SvgEngine;

class Scheme : public QObject
{
    Q_OBJECT
public:
    enum ColorSlot {
        invalid,

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
        Edit_blockSelectBg,

        Mark_errorFg,
        Mark_listingFg,
        Mark_fileFg,

        Icon_Main,
        Icon_Invers,

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
    int setActiveScheme(QString schemeName);
    int setActiveScheme(int scheme);
    int activeScheme() const;
    ColorSlot slot(QString name);
    void invalidate();
    void unbind(SvgEngine *engine);
    static void next();

    QByteArray exportJsonColorSchemes();
    void importJsonColorSchemes(const QByteArray &jsonData);

    static QString name(ColorSlot slot);
    static QColor color(ColorSlot slot);
    static QIcon icon(QString name);
    static QByteArray &data(QString name);
    static bool hasFlag(ColorSlot slot, FontFlag flag);

signals:
    void changed();

private:
    explicit Scheme(QObject *parent = nullptr);
    QHash<QString, QString> iconCodes() const;
    QByteArray colorizedContent(QString name);

private:
    static Scheme *mInstance;
    typedef QHash<ColorSlot, Color> ColorScheme;
    QList<ColorScheme> mColorSchemes;
    QStringList mSchemeNames;
    QHash<QString, QString> mIconCode;
    QHash<QString, QIcon> mIconCache;
    QHash<QString, QByteArray> mDataCache;
    int mActiveScheme = 0;
    QVector<SvgEngine*> mEngines;
};

inline QColor toColor(Scheme::ColorSlot code) { return Scheme::color(code); }
inline QString name(Scheme::ColorSlot col) { return Scheme::instance()->name(col); }

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_SCHEME_H
