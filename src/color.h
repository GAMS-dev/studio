#ifndef GAMS_STUDIO_COLOR_H
#define GAMS_STUDIO_COLOR_H

#include <QObject>
#include <QColor>

namespace gams {
namespace studio {

class Color : public QObject
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

public:
    static Color *instance();
    void initDefault();
    QStringList schemes();
    int setActiveScheme(QString schemeName);
    QString setActiveScheme(int scheme);

    QColor get(ColorSlot slot);
    QString name(ColorSlot slot);
    ColorSlot slot(QString name);

    QByteArray exportJsonColorSchemes();
    void importJsonColorSchemes(const QByteArray &jsonData);

signals:
    void colorsChanged();

private:
    explicit Color(QObject *parent = nullptr);

private:
    static Color *mInstance;
    typedef QHash<ColorSlot, QColor> ColorSet;
    QList<ColorSet> mColorSchemes;
    QStringList mSchemeNames;
    int mActiveScheme = 0;

};

inline QColor color(Color::ColorSlot code) { return Color::instance()->get(code); }
inline QString name(Color::ColorSlot col) { return Color::instance()->name(col); }


} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_COLOR_H
