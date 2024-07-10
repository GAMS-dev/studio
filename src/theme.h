/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef GAMS_STUDIO_THEME_H
#define GAMS_STUDIO_THEME_H

#include <QObject>
#include <QColor>
#include <QBrush>
#include <QIcon>
#include <QPalette>
#include <QHash>

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
        Edit_logRemoteBk,

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
        Syntax_dco,
        Syntax_formula,
        Syntax_dcoBody,
        Syntax_comment,
        Syntax_title,
        Syntax_keyword,
        Syntax_declaration,
        Syntax_identifier,
        Syntax_description,
        Syntax_assignLabel,
        Syntax_assignValue,
        Syntax_tableHeader,
        Syntax_embedded,

        ColorSlotCount
    };
    Q_ENUM(ColorSlot)

    enum FontFlag {fNormal, fBold, fItalic, fBoldItalic};

    enum IconSet {ThinIcons, SolidIcons};

private:
    struct Color {
        Color(QColor _color = QColor(), FontFlag _fontFlag = fNormal) : color(_color), fontFlag(_fontFlag) {}
        QColor color;
        FontFlag fontFlag;
    };

public:
    ~Theme() override;
    static Theme *instance();
    void initDefault();
    int themeCount() { return mThemeNames.size(); }
    QStringList themes();
    int setActiveTheme(const QString &themeName);
    int setActiveTheme(int theme);
    QString renameActiveTheme(const QString &name);
    int activeTheme() const;
    QString activeThemeName();
    int baseTheme(int theme) const;
    ColorSlot slot(const QString &name);
    void invalidate();
    void unbind(SvgEngine *engine);
    int copyTheme(int index, const QString &destName);
    int removeTheme(int index);

    QVariantList writeUserThemes() const;
    QVariantMap writeCurrentTheme();
    void readUserThemes(const QVariantList &sourceThemes);
    int readUserTheme(const QVariantMap &tSource);

    static bool isDark();
    static QString name(ColorSlot slot);
    static QString text(ColorSlot slot);
    static bool hasFontProps(ColorSlot slot);
    static QColor color(ColorSlot slot);
    static void setColor(ColorSlot slot, QColor color);
    static QIcon icon(const QString &name, QIcon::Mode mode, int alpha = 100);
    static QIcon icon(const QString &name, bool forceSquare = false, const QString &disabledName = QString(), int alpha = 100);
    static QIcon icon(QString name, QString nameKey, bool forceSquare, const QString &disabledName, int alpha);
    static QByteArray &data(const QString &name, QIcon::Mode mode, int alpha = 100);
    static bool hasFlag(ColorSlot slot, FontFlag flag);
    static void setFlags(ColorSlot slot, FontFlag flag);

signals:
    void changed(bool refreshSyntax);

private:
    explicit Theme(QObject *parent = nullptr);
    void initSlotTexts();
    QHash<QString, QStringList> iconCodes() const;
    QByteArray colorizedContent(const QString &name, QIcon::Mode mode, int alpha);
    QString findUniqueName(const QString &name, const QString &ignore = QString());
    static QString getNameWithMode(const QString &name, QIcon::Mode mode);

private:
    static Theme *mInstance;
    typedef QHash<ColorSlot, Color> ColorTheme;
    QList<ColorTheme> mColorThemes;
    QHash<ColorSlot, QString> mSlotText;
    QStringList mThemeNames;
    QList<int> mThemeBases;
    QString mIconSet;
    int mTheme;
    QHash<QString, QStringList> mIconCodes;
    QHash<QString, QIcon> mIconCache;
    QHash<QString, QByteArray> mDataCache;
    QVector<SvgEngine*> mEngines;

};

inline QColor toColor(Theme::ColorSlot code) { return Theme::color(code); }
inline QString name(Theme::ColorSlot col) { return Theme::instance()->name(col); }

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_THEME_H
