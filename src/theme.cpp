/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "theme.h"
#include "svgengine.h"

#include <QHash>
#include <QMetaEnum>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QGuiApplication>
#include <QWidget>
#include <QApplication>
#include <QStyleHints>

namespace gams {
namespace studio {

Theme *Theme::mInstance = nullptr;
const QColor CUndefined(255, 0, 200);
const QColor Theme::CAutoBackground(0, 0, 1);

Theme::Theme(QObject *parent) : QObject(parent)
{
    mIconSet = "solid";
    initDefault();
    initSlotTexts();
}

Theme::~Theme()
{
    for (SvgEngine *eng: std::as_const(mEngines))
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
    mSlotText.insert(Edit_text,                 "Default text");
    mSlotText.insert(Edit_currentLineBg,        "Current line");
    mSlotText.insert(Edit_errorBg,              "Error");
    mSlotText.insert(Edit_currentWordBg,        "Current word");
    mSlotText.insert(Edit_findFg,               "Find matches");
    mSlotText.insert(Edit_searchFg,             "Search result");
    mSlotText.insert(Edit_foldLineBg,           "Fold lines marker");
    mSlotText.insert(Edit_parenthesesValidFg,   "Matching parentheses");
    mSlotText.insert(Edit_parenthesesInvalidFg, "Invalid parentheses");
    mSlotText.insert(Edit_linenrAreaFg,         "Line number");
    mSlotText.insert(Edit_linenrAreaMarkFg,     "Current line number");
    mSlotText.insert(Edit_profilingBg1,          "Profiling tone low");
    mSlotText.insert(Edit_profilingBg2,          "Profiling tone medium");
    mSlotText.insert(Edit_profilingBg3,          "Profiling tone high");
    mSlotText.insert(Edit_foldLineFg,           "Folded lines marker");
    mSlotText.insert(Edit_linenrAreaFoldBg,     "Fold marker");
    mSlotText.insert(Mark_errorFg,              "Error marker");
    mSlotText.insert(Mark_listingFg,            "Listing marker");
    mSlotText.insert(Mark_fileFg,               "File marker");

    mSlotText.insert(Icon_Gray,                 "Icon Pen");
    mSlotText.insert(Icon_Back,                 "Icon Base");
    mSlotText.insert(Icon_Paper,                "Icon Paper");
    mSlotText.insert(Disable_Gray,              "Disabled Icon Pen");
    mSlotText.insert(Disable_Back,              "Disabled Icon Base");

    mSlotText.insert(Syntax_formula,            "Formula");
    mSlotText.insert(Syntax_comment,            "Comment");
    mSlotText.insert(Syntax_comment_bg,         "Comment background");
    mSlotText.insert(Syntax_dco,                "Dollar Control Option");
    mSlotText.insert(Syntax_dco_bg,             "Dollar Control Option background");
    mSlotText.insert(Syntax_dcoBody,            "Dollar Control Argument");
    mSlotText.insert(Syntax_dcoBody_bg,         "Dollar Control Argument background");
    mSlotText.insert(Syntax_title,              "Title");
    mSlotText.insert(Syntax_keyword,            "Keyword");
    mSlotText.insert(Syntax_keyword_bg,         "Keyword background");
    mSlotText.insert(Syntax_declaration,        "Declaration Keyword");
    mSlotText.insert(Syntax_declaration_bg,     "Declaration Keyword background");
    mSlotText.insert(Syntax_identifier,         "Identifier");
    mSlotText.insert(Syntax_identifier_bg,      "Identifier background");
    mSlotText.insert(Syntax_description,        "Symbol text");
    mSlotText.insert(Syntax_description_bg,     "Symbol text background");
    mSlotText.insert(Syntax_assignLabel,        "Element");
    mSlotText.insert(Syntax_assignLabel_bg,     "Element background");
    mSlotText.insert(Syntax_assignValue,        "Element text/value");
    mSlotText.insert(Syntax_assignValue_bg,     "Element text/value background");
    mSlotText.insert(Syntax_tableHeader,        "Table header");
    mSlotText.insert(Syntax_tableHeader_bg,     "Table header background");
    mSlotText.insert(Syntax_embedded,           "Embedded code");
    mSlotText.insert(Syntax_embedded_bg,        "Embedded code background");

    mSlotText.insert(Window_window,             "Window background");
    mSlotText.insert(Window_windowText,         "Window");
    mSlotText.insert(Window_base,               "Text background");
    mSlotText.insert(Window_alternateBase,      "Alternate background");
    mSlotText.insert(Window_tooltipBase,        "Tooltip background");
    mSlotText.insert(Window_tooltipText,        "Tooltip");
    mSlotText.insert(Window_labelHighlight,     "Label background");
    mSlotText.insert(Window_labelHighlightText, "Label");
    mSlotText.insert(Window_text,               "Text");
    mSlotText.insert(Window_button,             "Button background");
    mSlotText.insert(Window_buttonText,         "Button Text");
    mSlotText.insert(Window_placeHolderText,    "Input");
    mSlotText.insert(Window_highlight,          "Selection background");
    mSlotText.insert(Window_highlightedText,    "Selection");
    mSlotText.insert(Window_link,               "Hyperlink");

    mSlotText.insert(Window_disable_window,        "Disabled Window");
    mSlotText.insert(Window_disable_base,          "Disabled Text");
    mSlotText.insert(Window_disable_button,        "Disabled Button");
    mSlotText.insert(Window_disable_alternateBase, "Disabled AlternateBase");

    mSlotText.insert(Window_disable_windowText,    "Disabled Window");
    mSlotText.insert(Window_disable_text,          "Disabled Text");
    mSlotText.insert(Window_disable_buttonText,    "Disabled Button Text");

    mSlotText.insert(Window_disable_highlight,      "Disabled Selection");
    mSlotText.insert(Window_disable_hightlightText, "Disabled Selection");

    mSlotText.insert(Window_disable_placeholderText,"Disabled Input");
    mSlotText.insert(Window_disable_link,           "Disabled hyperlink");
}

void Theme::initDefault()
{
    mColorThemes.clear();
    mThemeNames.clear();
    mThemeBases.clear();

    // default to first color theme - Light
    mActiveTheme = 0;

    // Add first color theme
    int sNr = 0;
    mColorThemes << ColorTheme();
    mThemeNames  << "Light";
    mThemeBases  << 0;

    QColor light_white        (Qt::white);
    QColor light_darkWhite    (245, 245, 245);
    QColor light_grayWhite    (240, 240, 240);
    QColor light_darkerWhite  (230, 230, 230);
    QColor light_inactiveWhite(200, 200, 200);

    QColor light_black        (Qt::black);
//    QColor light_lightBlack   (50, 50, 50);
//    QColor light_darkGray     (170, 170, 170);
    QColor light_gray         (140, 140, 140);
    QColor light_lightGray    (128, 128, 128);

    QColor light_highlightblue(160, 210, 238);
    QColor light_selectBlue   (0, 120, 215);
    QColor light_linkBlue     (0, 102, 204);

    QColor profile_green      (55,235, 20);
    QColor profile_yellow     (255,205,  0);
    QColor profile_red        (255, 55, 20);

    QColor light_red          (187,0,0);
    QColor light_green        (52,134,25);
    QColor light_blue         (34,102,170);
    QColor light_yellow       (236,140,20);
    QColor light_lightYellow  (255, 250, 170);

    QColor gams_orange        (243,150,25);
    QColor buttery_yellow     (255, 240, 200);

    mColorThemes[sNr].clear();
    mColorThemes[sNr].insert(invalid,                        CUndefined);
    mColorThemes[sNr].insert(Edit_text,                      light_black);
    mColorThemes[sNr].insert(Syntax_neutral,                 light_black);
    mColorThemes[sNr].insert(Edit_background,                light_white);
    mColorThemes[sNr].insert(Edit_currentLineBg,             light_lightYellow);
    mColorThemes[sNr].insert(Edit_errorBg,                   QColor(255, 220, 200));
    mColorThemes[sNr].insert(Edit_currentWordBg,             QColor(210,200,200));
    mColorThemes[sNr].insert(Edit_findBg,                    QColor(92,194,75));
    mColorThemes[sNr].insert(Edit_searchBg,                  QColor(102,164,0));
    mColorThemes[sNr].insert(Edit_findFg,                    light_white);
    mColorThemes[sNr].insert(Edit_searchFg,                  light_white);
    mColorThemes[sNr].insert(Edit_foldLineBg,                QColor(135,195,255));
    mColorThemes[sNr].insert(Edit_foldLineFg,                QColor(255,255,255));
    mColorThemes[sNr].insert(Edit_parenthesesValidFg,        QColor(Qt::red));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidFg,      QColor(Qt::black));
    mColorThemes[sNr].insert(Edit_parenthesesValidBg,        QColor(Qt::green).lighter(170));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidBg,      QColor(Qt::red).lighter(150));
    mColorThemes[sNr].insert(Edit_parenthesesValidBgBlink,   QColor(Qt::green).lighter(130));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidBgBlink, QColor(Qt::red).lighter(115));
    mColorThemes[sNr].insert(Edit_linenrAreaBg,              light_darkWhite);
    mColorThemes[sNr].insert(Edit_linenrAreaMarkBg,          QColor(225,255,235));
    mColorThemes[sNr].insert(Edit_linenrAreaFoldBg,          QColor(135,195,255));
    mColorThemes[sNr].insert(Edit_profilingBg1,              profile_green);
    mColorThemes[sNr].insert(Edit_profilingBg2,              profile_yellow);
    mColorThemes[sNr].insert(Edit_profilingBg3,              profile_red);
    mColorThemes[sNr].insert(Edit_linenrAreaFg,              light_gray);
    mColorThemes[sNr].insert(Edit_linenrAreaMarkFg,          light_black);
    mColorThemes[sNr].insert(Edit_logRemoteBk,               light_lightYellow);

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
    mColorThemes[sNr].insert(Normal_Red,                     light_red);
    mColorThemes[sNr].insert(Normal_Green,                   light_green);
    mColorThemes[sNr].insert(Normal_Blue,                    light_blue);
    mColorThemes[sNr].insert(Normal_Yellow,                  light_yellow);

    mColorThemes[sNr].insert(Syntax_undefined,               CUndefined);
    mColorThemes[sNr].insert(Syntax_dco,                     Color(QColor(Qt::darkMagenta).darker(120)));
    mColorThemes[sNr].insert(Syntax_dco_bg,                  CAutoBackground);
    mColorThemes[sNr].insert(Syntax_formula,                 Color(Qt::black));
    mColorThemes[sNr].insert(Syntax_dcoBody,                 Color(QColor(Qt::darkBlue).lighter(170), fItalic));
    mColorThemes[sNr].insert(Syntax_dcoBody_bg,              CAutoBackground);
    mColorThemes[sNr].insert(Syntax_comment,                 Color(QColor(120, 120, 120), fItalic));
    mColorThemes[sNr].insert(Syntax_comment_bg,              CAutoBackground);
    mColorThemes[sNr].insert(Syntax_title,                   Color(QColor(Qt::darkBlue).lighter(140), fBold));
    mColorThemes[sNr].insert(Syntax_keyword,                 Color(QColor(Qt::darkBlue).lighter(140), fBold));
    mColorThemes[sNr].insert(Syntax_keyword_bg,              CAutoBackground);
    mColorThemes[sNr].insert(Syntax_declaration,             Color(QColor(Qt::darkBlue).lighter(140), fBold));
    mColorThemes[sNr].insert(Syntax_declaration_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_identifier,              Color(QColor(Qt::black)));
    mColorThemes[sNr].insert(Syntax_identifier_bg,           CAutoBackground);
    mColorThemes[sNr].insert(Syntax_description,             Color(QColor(Qt::darkBlue).lighter(170)));
    mColorThemes[sNr].insert(Syntax_description_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_assignLabel,             Color(QColor(Qt::darkGreen).darker(110)));
    mColorThemes[sNr].insert(Syntax_assignLabel_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_assignValue,             Color(QColor(0, 80, 120)));
    mColorThemes[sNr].insert(Syntax_assignValue_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_tableHeader,             Color(QColor(Qt::darkGreen).darker(140), fBold));
    mColorThemes[sNr].insert(Syntax_tableHeader_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_embedded,                Color(QColor(200, 70, 0)));
    mColorThemes[sNr].insert(Syntax_embedded_bg,             CAutoBackground);

    mColorThemes[sNr].insert(Window_window,                  light_darkWhite);
    mColorThemes[sNr].insert(Window_windowText,              light_black);
    mColorThemes[sNr].insert(Window_base,                    light_white);
    mColorThemes[sNr].insert(Window_alternateBase,           light_darkerWhite);
    mColorThemes[sNr].insert(Window_text,                    light_black);
    mColorThemes[sNr].insert(Window_button,                  light_darkWhite);
    mColorThemes[sNr].insert(Window_buttonText,              light_black);
    mColorThemes[sNr].insert(Window_placeHolderText,         light_lightGray);
    mColorThemes[sNr].insert(Window_highlight,               light_highlightblue);
    mColorThemes[sNr].insert(Window_highlightedText,         light_black);
    mColorThemes[sNr].insert(Window_link,                    light_selectBlue);

    mColorThemes[sNr].insert(Window_tooltipBase,             buttery_yellow);
    mColorThemes[sNr].insert(Window_tooltipText,             light_black);
    mColorThemes[sNr].insert(Window_labelHighlight,          gams_orange);
    mColorThemes[sNr].insert(Window_labelHighlightText,      light_black);

    mColorThemes[sNr].insert(Window_disable_window,          light_white);
    mColorThemes[sNr].insert(Window_disable_base,            light_white);
    mColorThemes[sNr].insert(Window_disable_button,          light_white);
    mColorThemes[sNr].insert(Window_disable_alternateBase,   light_white);

    mColorThemes[sNr].insert(Window_disable_windowText,      light_inactiveWhite);
    mColorThemes[sNr].insert(Window_disable_text,            light_inactiveWhite);
    mColorThemes[sNr].insert(Window_disable_buttonText,      light_inactiveWhite);

    mColorThemes[sNr].insert(Window_disable_hightlightText,  light_lightGray);
    mColorThemes[sNr].insert(Window_disable_highlight,       light_highlightblue);

    mColorThemes[sNr].insert(Window_disable_placeholderText, light_gray);
    mColorThemes[sNr].insert(Window_disable_link,            light_gray);

    // Add and switch to second color theme - Dark
    sNr++;
    mColorThemes << mColorThemes.at(0);
    mThemeNames  << "Dark";
    mThemeBases  << sNr;

    // Dark Colors
    QColor dark_selection   (214, 135, 31);
    QColor dark_lightBlack  (30, 30, 30);
    QColor dark_base        (45,45,45);
    QColor dark_disabled    (65, 65, 65);
    QColor dark_id          (153,240,255);
    QColor dark_assignment  (144,226,149);
    QColor dark_unobstrusive(125,125,125);
    QColor dark_neutral     (223,224,223);
    QColor dark_background  (Qt::black);

    QColor dark_red         (187,34,51);
    QColor dark_green       (102,170,102);
    QColor dark_blue        (68,153,238);
    QColor dark_yellow      (236,140,20);

    QColor dark_linebg      (0,73,61);

    mColorThemes[sNr].insert(Edit_text,                      dark_neutral);
    mColorThemes[sNr].insert(Syntax_neutral,                 dark_neutral);
    mColorThemes[sNr].insert(Edit_background,                dark_base);
    mColorThemes[sNr].insert(Edit_currentLineBg,             dark_linebg);
    mColorThemes[sNr].insert(Edit_errorBg,                   QColor(187,34,51));
    mColorThemes[sNr].insert(Edit_findBg,                    QColor(51,102,51));
    mColorThemes[sNr].insert(Edit_searchBg,                  QColor(102,164,0));
    mColorThemes[sNr].insert(Edit_findFg,                    light_white);
    mColorThemes[sNr].insert(Edit_searchFg,                  light_white);
    mColorThemes[sNr].insert(Edit_foldLineBg,                dark_selection.darker(150));
    mColorThemes[sNr].insert(Edit_foldLineFg,                QColor(0,0,0));
    mColorThemes[sNr].insert(Edit_linenrAreaBg,              dark_lightBlack);
    mColorThemes[sNr].insert(Edit_linenrAreaMarkBg,          QColor(40,40,40));
    mColorThemes[sNr].insert(Edit_linenrAreaFoldBg,          QColor(15,75,115));
    mColorThemes[sNr].insert(Edit_linenrAreaMarkFg,          QColor(Qt::white));
    mColorThemes[sNr].insert(Edit_currentWordBg,             dark_lightBlack);
    mColorThemes[sNr].insert(Edit_parenthesesValidFg,        QColor(Qt::yellow).lighter(170));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidFg,      QColor(Qt::black));
    mColorThemes[sNr].insert(Edit_parenthesesValidBg,        QColor(40,70,30));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidBg,      QColor(Qt::red).darker(170));
    mColorThemes[sNr].insert(Edit_parenthesesValidBgBlink,   QColor(60,90,50));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidBgBlink, QColor(Qt::red).darker(115));
    mColorThemes[sNr].insert(Edit_profilingBg1,              profile_green);
    mColorThemes[sNr].insert(Edit_profilingBg2,              profile_yellow);
    mColorThemes[sNr].insert(Edit_profilingBg3,              profile_red);
    mColorThemes[sNr].insert(Edit_logRemoteBk,               dark_linebg);

    mColorThemes[sNr].insert(Mark_errorFg,                   QColor(180,60,60));
    mColorThemes[sNr].insert(Mark_listingFg,                 QColor(60,90,250));
    mColorThemes[sNr].insert(Mark_fileFg,                    QColor(Qt::darkGreen));

    mColorThemes[sNr].insert(Icon_Gray,                      QColor(65, 55, 50));
    mColorThemes[sNr].insert(Icon_Back,                      QColor(220,220,220));
    mColorThemes[sNr].insert(Icon_Paper,                     dark_neutral);
    mColorThemes[sNr].insert(Active_Gray,                    dark_unobstrusive);
    mColorThemes[sNr].insert(Active_Back,                    QColor(Qt::white));
    mColorThemes[sNr].insert(Disable_Gray,                   dark_unobstrusive);
    mColorThemes[sNr].insert(Disable_Back,                   dark_unobstrusive.lighter(75));
    mColorThemes[sNr].insert(Select_Gray,                    gams_orange);
    mColorThemes[sNr].insert(Select_Back,                    dark_base);
    mColorThemes[sNr].insert(Normal_Red,                     dark_red);
    mColorThemes[sNr].insert(Normal_Green,                   dark_green);
    mColorThemes[sNr].insert(Normal_Blue,                    dark_blue);
    mColorThemes[sNr].insert(Normal_Yellow,                  dark_yellow);

    mColorThemes[sNr].insert(Syntax_title,                   Color(gams_orange, fBold));
    mColorThemes[sNr].insert(Syntax_dco,                     QColor(200,60,90));
    mColorThemes[sNr].insert(Syntax_dco_bg,                  CAutoBackground);
    mColorThemes[sNr].insert(Syntax_formula,                 Color(dark_neutral));
    mColorThemes[sNr].insert(Syntax_dcoBody,                 Color(gams_orange, fItalic));
    mColorThemes[sNr].insert(Syntax_dcoBody_bg,              CAutoBackground);
    mColorThemes[sNr].insert(Syntax_comment,                 Color(dark_unobstrusive, fItalic));
    mColorThemes[sNr].insert(Syntax_comment_bg,              CAutoBackground);
    mColorThemes[sNr].insert(Syntax_keyword,                 Color(gams_orange, fBold));
    mColorThemes[sNr].insert(Syntax_keyword_bg,              CAutoBackground);
    mColorThemes[sNr].insert(Syntax_declaration,             Color(gams_orange, fBold));
    mColorThemes[sNr].insert(Syntax_declaration_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_identifier,              Color(dark_id));
    mColorThemes[sNr].insert(Syntax_identifier_bg,           CAutoBackground);
    mColorThemes[sNr].insert(Syntax_description,             Color(dark_unobstrusive));
    mColorThemes[sNr].insert(Syntax_description_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_assignLabel,             dark_assignment);
    mColorThemes[sNr].insert(Syntax_assignLabel_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_assignValue,             Color(QColor(9, 207, 187)));
    mColorThemes[sNr].insert(Syntax_assignValue_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_tableHeader,             Color(dark_assignment, fBold));
    mColorThemes[sNr].insert(Syntax_tableHeader_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_assignValue,             Color(dark_assignment.lighter()));
    mColorThemes[sNr].insert(Syntax_embedded_bg,             CAutoBackground);

    mColorThemes[sNr].insert(Window_window,                  dark_lightBlack);
    mColorThemes[sNr].insert(Window_windowText,              dark_neutral);
    mColorThemes[sNr].insert(Window_base,                    dark_base);
    mColorThemes[sNr].insert(Window_alternateBase,           dark_disabled);
    mColorThemes[sNr].insert(Window_text,                    dark_neutral);
    mColorThemes[sNr].insert(Window_button,                  dark_base);
    mColorThemes[sNr].insert(Window_buttonText,              dark_neutral);
    mColorThemes[sNr].insert(Window_placeHolderText,         dark_unobstrusive);
    mColorThemes[sNr].insert(Window_highlight,               dark_selection);
    mColorThemes[sNr].insert(Window_highlightedText,         dark_base);
    mColorThemes[sNr].insert(Window_link,                    light_selectBlue);

    mColorThemes[sNr].insert(Window_tooltipBase,             dark_base);
    mColorThemes[sNr].insert(Window_tooltipText,             dark_neutral);
    mColorThemes[sNr].insert(Window_labelHighlight,          gams_orange);
    mColorThemes[sNr].insert(Window_labelHighlightText,      dark_neutral);

    mColorThemes[sNr].insert(Window_disable_window,          dark_base);
    mColorThemes[sNr].insert(Window_disable_base,            dark_base);
    mColorThemes[sNr].insert(Window_disable_button,          dark_base);
    mColorThemes[sNr].insert(Window_disable_alternateBase,   dark_base);

    mColorThemes[sNr].insert(Window_disable_windowText,      dark_unobstrusive);
    mColorThemes[sNr].insert(Window_disable_text,            dark_unobstrusive);
    mColorThemes[sNr].insert(Window_disable_buttonText,      dark_unobstrusive);

    mColorThemes[sNr].insert(Window_disable_hightlightText,  light_lightGray);
    mColorThemes[sNr].insert(Window_disable_highlight,       dark_base);

    mColorThemes[sNr].insert(Window_disable_placeholderText, light_gray);
    mColorThemes[sNr].insert(Window_disable_link,            light_gray);

    // Third theme is based on Okabe-Ito Palette, recommened colorblind-safe color pallette
    sNr++;
    mColorThemes << mColorThemes.at(0);
    mThemeNames  << "Distinct - light";
    mThemeBases  << sNr;

    QColor distinct_orange     (230, 159, 0);
//    QColor distinct_skyblue    (86, 180, 233);
    QColor distinct_bluishgreen(0, 158, 115);
    QColor distinct_yellow     (240, 228, 66);
    QColor distinct_blue       (0, 114, 178);
    QColor distinct_red        (213, 94, 0);
    QColor distinct_purple     (204, 121, 167);
    QColor distinct_black      (0, 0, 0);
    QColor distinct_gray       (65, 74, 84);

    mColorThemes[sNr].clear();
    mColorThemes[sNr].insert(invalid,                        CUndefined);
    mColorThemes[sNr].insert(Edit_text,                      distinct_black);
    mColorThemes[sNr].insert(Syntax_neutral,                 distinct_black);
    mColorThemes[sNr].insert(Edit_background,                light_white);
    mColorThemes[sNr].insert(Edit_currentLineBg,             light_lightYellow);
    mColorThemes[sNr].insert(Edit_errorBg,                   QColor(255, 220, 200));
    mColorThemes[sNr].insert(Edit_currentWordBg,             QColor(210,200,200));
    mColorThemes[sNr].insert(Edit_findBg,                    distinct_red);
    mColorThemes[sNr].insert(Edit_searchBg,                  distinct_orange);
    mColorThemes[sNr].insert(Edit_findFg,                    light_white);
    mColorThemes[sNr].insert(Edit_searchFg,                  light_white);
    mColorThemes[sNr].insert(Edit_foldLineBg,                distinct_yellow.darker(100));
    mColorThemes[sNr].insert(Edit_foldLineFg,                distinct_black.lighter(120));
    mColorThemes[sNr].insert(Edit_parenthesesValidFg,        distinct_red);
    mColorThemes[sNr].insert(Edit_parenthesesInvalidFg,      distinct_black);
    mColorThemes[sNr].insert(Edit_parenthesesValidBg,        distinct_bluishgreen.lighter(130));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidBg,      distinct_red.lighter(150));
    mColorThemes[sNr].insert(Edit_parenthesesValidBgBlink,   distinct_bluishgreen.lighter(130));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidBgBlink, distinct_red.lighter(115));
    mColorThemes[sNr].insert(Edit_linenrAreaBg,              light_grayWhite);
    mColorThemes[sNr].insert(Edit_linenrAreaMarkBg,          light_inactiveWhite.lighter(105));
    mColorThemes[sNr].insert(Edit_linenrAreaFoldBg,          distinct_yellow.darker(100));
    mColorThemes[sNr].insert(Edit_profilingBg1,              profile_green);
    mColorThemes[sNr].insert(Edit_profilingBg2,              profile_yellow);
    mColorThemes[sNr].insert(Edit_profilingBg3,              profile_red);
    mColorThemes[sNr].insert(Edit_linenrAreaFg,              light_lightGray);
    mColorThemes[sNr].insert(Edit_linenrAreaMarkFg,          QColor(Qt::black));
    mColorThemes[sNr].insert(Edit_logRemoteBk,               light_lightYellow);

    mColorThemes[sNr].insert(Mark_errorFg,                   distinct_red);
    mColorThemes[sNr].insert(Mark_listingFg,                 distinct_blue);
    mColorThemes[sNr].insert(Mark_fileFg,                    distinct_bluishgreen);

    mColorThemes[sNr].insert(Icon_Gray,                      QColor(170,150,130));
    mColorThemes[sNr].insert(Icon_Back,                      QColor(51,68,85));
    mColorThemes[sNr].insert(Icon_Paper,                     QColor(Qt::white));
    mColorThemes[sNr].insert(Active_Gray,                    QColor(54,122,195));
    mColorThemes[sNr].insert(Active_Back,                    distinct_black);
    mColorThemes[sNr].insert(Disable_Gray,                   QColor(180,180,180));
    mColorThemes[sNr].insert(Disable_Back,                   QColor(201,201,201));
    mColorThemes[sNr].insert(Select_Gray,                    distinct_yellow);
    mColorThemes[sNr].insert(Select_Back,                    light_white);
    mColorThemes[sNr].insert(Normal_Red,                     light_red);
    mColorThemes[sNr].insert(Normal_Green,                   light_green);
    mColorThemes[sNr].insert(Normal_Blue,                    light_blue);
    mColorThemes[sNr].insert(Normal_Yellow,                  light_yellow);

    mColorThemes[sNr].insert(Syntax_undefined,               CUndefined);
    mColorThemes[sNr].insert(Syntax_dco,                     Color(distinct_red, fBold));
    mColorThemes[sNr].insert(Syntax_dco_bg,                  CAutoBackground);
    mColorThemes[sNr].insert(Syntax_formula,                 distinct_black);
    mColorThemes[sNr].insert(Syntax_dcoBody,                 Color(distinct_blue, fItalic));
    mColorThemes[sNr].insert(Syntax_dcoBody_bg,              CAutoBackground);
    mColorThemes[sNr].insert(Syntax_comment,                 Color(distinct_gray, fItalic));
    mColorThemes[sNr].insert(Syntax_comment_bg,              CAutoBackground);
    mColorThemes[sNr].insert(Syntax_title,                   Color(distinct_blue, fBold));
    mColorThemes[sNr].insert(Syntax_keyword,                 Color(distinct_red, fBold));
    mColorThemes[sNr].insert(Syntax_keyword_bg,              CAutoBackground);
    mColorThemes[sNr].insert(Syntax_declaration,             Color(distinct_red, fBold));
    mColorThemes[sNr].insert(Syntax_declaration_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_identifier,              distinct_blue);
    mColorThemes[sNr].insert(Syntax_identifier_bg,           CAutoBackground);
    mColorThemes[sNr].insert(Syntax_description,             distinct_gray);
    mColorThemes[sNr].insert(Syntax_description_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_assignLabel,             distinct_bluishgreen);
    mColorThemes[sNr].insert(Syntax_assignLabel_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_assignValue,             distinct_purple);
    mColorThemes[sNr].insert(Syntax_assignValue_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_tableHeader,             Color(distinct_bluishgreen, fBold));
    mColorThemes[sNr].insert(Syntax_tableHeader_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_embedded,                distinct_orange);
    mColorThemes[sNr].insert(Syntax_embedded_bg,             CAutoBackground);

    mColorThemes[sNr].insert(Window_window,                  light_grayWhite);
    mColorThemes[sNr].insert(Window_windowText,              distinct_black);
    mColorThemes[sNr].insert(Window_base,                    light_white);
    mColorThemes[sNr].insert(Window_alternateBase,           light_inactiveWhite.lighter(105));
    mColorThemes[sNr].insert(Window_text,                    distinct_black);
    mColorThemes[sNr].insert(Window_button,                  light_darkerWhite);
    mColorThemes[sNr].insert(Window_buttonText,              distinct_black);
    mColorThemes[sNr].insert(Window_placeHolderText,         distinct_blue);
    mColorThemes[sNr].insert(Window_highlight,               distinct_yellow);
    mColorThemes[sNr].insert(Window_highlightedText,         distinct_black);
    mColorThemes[sNr].insert(Window_link,                    light_linkBlue);

    mColorThemes[sNr].insert(Window_tooltipBase,             buttery_yellow);
    mColorThemes[sNr].insert(Window_tooltipText,             light_black);
    mColorThemes[sNr].insert(Window_labelHighlight,          distinct_yellow);
    mColorThemes[sNr].insert(Window_labelHighlightText,      light_black);

    mColorThemes[sNr].insert(Window_disable_window,          light_white);
    mColorThemes[sNr].insert(Window_disable_base,            light_white);
    mColorThemes[sNr].insert(Window_disable_button,          light_white);
    mColorThemes[sNr].insert(Window_disable_alternateBase,   light_white);

    mColorThemes[sNr].insert(Window_disable_windowText,      light_inactiveWhite);
    mColorThemes[sNr].insert(Window_disable_text,            light_inactiveWhite);
    mColorThemes[sNr].insert(Window_disable_buttonText,      light_inactiveWhite);

    mColorThemes[sNr].insert(Window_disable_hightlightText,  light_lightGray);
    mColorThemes[sNr].insert(Window_disable_highlight,       light_darkerWhite);

    mColorThemes[sNr].insert(Window_disable_placeholderText, light_gray);
    mColorThemes[sNr].insert(Window_disable_link,            light_gray);

    // Dracula Color Theme
    sNr++;
    mColorThemes << mColorThemes.at(1);
    mThemeNames  << "Dracula - dark";
    mThemeBases  << sNr;

    QColor drac_background (40, 42, 54);
//    QColor drac_currentLine(98, 114, 164);
    QColor drac_selection  (68, 71, 90);
    QColor drac_foreground (248, 248, 242);
    QColor drac_comment    (98, 114, 164);
    QColor drac_cyan       (139, 233, 253);
    QColor drac_green      (80, 250, 123);
    QColor drac_orange     (255, 184, 108);
    QColor drac_pink       (255, 121, 198);
    QColor drac_purple     (189, 147, 249);
    QColor drac_red        (255, 85, 85);
    QColor drac_yellow     (241, 250, 140);

    QColor drac_bgLighter  (66, 68, 80);
    QColor drac_bgLight    (52, 55, 70);
//    QColor drac_bgDark     (33, 34, 44);
    QColor drac_bgDarker   (25, 26, 33);

    mColorThemes[sNr].insert(Edit_text,                      drac_foreground);
    mColorThemes[sNr].insert(Syntax_neutral,                 drac_foreground);
    mColorThemes[sNr].insert(Edit_background,                drac_background);
    mColorThemes[sNr].insert(Edit_currentLineBg,             dark_linebg);
    mColorThemes[sNr].insert(Edit_errorBg,                   drac_red);
    mColorThemes[sNr].insert(Edit_findBg,                    drac_pink.darker(190));
    mColorThemes[sNr].insert(Edit_searchBg,                  drac_pink);
    mColorThemes[sNr].insert(Edit_findFg,                    drac_foreground);
    mColorThemes[sNr].insert(Edit_searchFg,                  drac_foreground);
    mColorThemes[sNr].insert(Edit_foldLineBg,                drac_purple.darker(90));
    mColorThemes[sNr].insert(Edit_foldLineFg,                drac_bgLighter);
    mColorThemes[sNr].insert(Edit_linenrAreaBg,              drac_bgDarker);
    mColorThemes[sNr].insert(Edit_linenrAreaMarkBg,          drac_bgLighter);
    mColorThemes[sNr].insert(Edit_linenrAreaFoldBg,          drac_bgLight);
    mColorThemes[sNr].insert(Edit_linenrAreaMarkFg,          drac_purple);
    mColorThemes[sNr].insert(Edit_currentWordBg,             dark_background);
    mColorThemes[sNr].insert(Edit_parenthesesValidFg,        drac_foreground);
    mColorThemes[sNr].insert(Edit_parenthesesInvalidFg,      drac_background);
    mColorThemes[sNr].insert(Edit_parenthesesValidBg,        drac_comment.lighter(90));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidBg,      drac_cyan);
    mColorThemes[sNr].insert(Edit_parenthesesValidBgBlink,   drac_comment.lighter(120));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidBgBlink, drac_cyan.darker(115));
    mColorThemes[sNr].insert(Edit_logRemoteBk,               dark_linebg);

    mColorThemes[sNr].insert(Mark_errorFg,                   drac_red);
    mColorThemes[sNr].insert(Mark_listingFg,                 drac_cyan);
    mColorThemes[sNr].insert(Mark_fileFg,                    drac_green);

    mColorThemes[sNr].insert(Icon_Gray,                      QColor(65, 55, 50));
    mColorThemes[sNr].insert(Icon_Back,                      QColor(220,220,220));
    mColorThemes[sNr].insert(Icon_Paper,                     dark_neutral);
    mColorThemes[sNr].insert(Active_Gray,                    dark_unobstrusive);
    mColorThemes[sNr].insert(Active_Back,                    drac_foreground);
    mColorThemes[sNr].insert(Disable_Gray,                   dark_unobstrusive);
    mColorThemes[sNr].insert(Disable_Back,                   dark_unobstrusive.lighter(75));
    mColorThemes[sNr].insert(Select_Gray,                    drac_purple);
    mColorThemes[sNr].insert(Select_Back,                    dark_base);
    mColorThemes[sNr].insert(Normal_Red,                     dark_red);
    mColorThemes[sNr].insert(Normal_Green,                   dark_green);
    mColorThemes[sNr].insert(Normal_Blue,                    dark_blue);
    mColorThemes[sNr].insert(Normal_Yellow,                  dark_yellow);

    mColorThemes[sNr].insert(Syntax_undefined,               CUndefined);
    mColorThemes[sNr].insert(Syntax_dco,                     drac_red);
    mColorThemes[sNr].insert(Syntax_dco_bg,                  CAutoBackground);
    mColorThemes[sNr].insert(Syntax_formula,                 drac_yellow);
    mColorThemes[sNr].insert(Syntax_dcoBody,                 Color(drac_yellow, fItalic));
    mColorThemes[sNr].insert(Syntax_dcoBody_bg,              CAutoBackground);
    mColorThemes[sNr].insert(Syntax_comment,                 Color(drac_comment, fItalic));
    mColorThemes[sNr].insert(Syntax_comment_bg,              CAutoBackground);
    mColorThemes[sNr].insert(Syntax_title,                   Color(drac_yellow, fBold));
    mColorThemes[sNr].insert(Syntax_keyword,                 Color(drac_pink,   fBold));
    mColorThemes[sNr].insert(Syntax_keyword_bg,              CAutoBackground);
    mColorThemes[sNr].insert(Syntax_declaration,             Color(drac_cyan, fBold));
    mColorThemes[sNr].insert(Syntax_declaration_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_identifier,              drac_foreground);
    mColorThemes[sNr].insert(Syntax_identifier_bg,           CAutoBackground);
    mColorThemes[sNr].insert(Syntax_description,             drac_comment);
    mColorThemes[sNr].insert(Syntax_description_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_assignLabel,             drac_purple);
    mColorThemes[sNr].insert(Syntax_assignLabel_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_assignValue,             Color(drac_purple.lighter()));
    mColorThemes[sNr].insert(Syntax_assignValue_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_tableHeader,             drac_green);
    mColorThemes[sNr].insert(Syntax_tableHeader_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_embedded,                drac_orange);
    mColorThemes[sNr].insert(Syntax_embedded_bg,             CAutoBackground);

    mColorThemes[sNr].insert(Window_window,                  drac_bgDarker);
    mColorThemes[sNr].insert(Window_windowText,              drac_foreground);
    mColorThemes[sNr].insert(Window_base,                    drac_background);
    mColorThemes[sNr].insert(Window_alternateBase,           drac_selection);
    mColorThemes[sNr].insert(Window_text,                    drac_foreground);
    mColorThemes[sNr].insert(Window_button,                  drac_background);
    mColorThemes[sNr].insert(Window_buttonText,              dark_neutral);
    mColorThemes[sNr].insert(Window_placeHolderText,         drac_comment);
    mColorThemes[sNr].insert(Window_highlight,               drac_purple);
    mColorThemes[sNr].insert(Window_highlightedText,         drac_background);
    mColorThemes[sNr].insert(Window_link,                    drac_cyan);

    mColorThemes[sNr].insert(Window_tooltipBase,             dark_base);
    mColorThemes[sNr].insert(Window_tooltipText,             dark_neutral);
    mColorThemes[sNr].insert(Window_labelHighlight,          drac_purple);
    mColorThemes[sNr].insert(Window_labelHighlightText,      dark_neutral);

    mColorThemes[sNr].insert(Window_disable_window,          dark_base);
    mColorThemes[sNr].insert(Window_disable_base,            dark_background);
    mColorThemes[sNr].insert(Window_disable_button,          dark_base);
    mColorThemes[sNr].insert(Window_disable_alternateBase,   dark_base);

    mColorThemes[sNr].insert(Window_disable_windowText,      dark_unobstrusive);
    mColorThemes[sNr].insert(Window_disable_text,            dark_unobstrusive);
    mColorThemes[sNr].insert(Window_disable_buttonText,      dark_unobstrusive);

    mColorThemes[sNr].insert(Window_disable_hightlightText,  light_lightGray);
    mColorThemes[sNr].insert(Window_disable_highlight,       drac_background);

    mColorThemes[sNr].insert(Window_disable_placeholderText, light_gray);
    mColorThemes[sNr].insert(Window_disable_link,            light_gray);

    // Solarized light Theme
    sNr++;
    mColorThemes << mColorThemes.at(0);
    mThemeNames  << "Solarized - light";
    mThemeBases  << sNr;

    QColor solarized_base0  (131, 148, 150);  // text
    QColor solarized_base1  (147, 161, 161);  // light text
    QColor solarized_base2  (238, 232, 213);  // input field/list background
    QColor solarized_base3  (253, 246, 227);  // very light
    QColor solarized_base00 (101, 123, 131);  // body text
    QColor solarized_base01 (88, 110, 117);   // dark text/disable text
    QColor solarized_base02 (7, 54, 66);      // dark
    QColor solarized_base03 (0, 43, 54);      // very dark
    QColor solarized_yellow (181, 137, 0);
    QColor solarized_orange (203, 75, 22);    // brred
    QColor solarized_red    (220, 50, 47);
    QColor solarized_magenta(211, 54, 130);
    QColor solarized_violet (108, 113, 196);  // brmagenta
    QColor solarized_blue   (38, 139, 210);
    QColor solarized_cyan   (42, 161, 152);
    QColor solarized_green  (133, 153, 0);

    QColor solarized_highlight (140, 197, 183);
    QColor solarized_lightgreen(200, 228, 212);
    QColor solarized_linebg    (210, 205, 190);

    Q_UNUSED(solarized_base00)

    mColorThemes[sNr].insert(invalid,                        CUndefined);
    mColorThemes[sNr].insert(Edit_text,                      solarized_base02);
    mColorThemes[sNr].insert(Syntax_neutral,                 solarized_base02);
    mColorThemes[sNr].insert(Edit_background,                solarized_base3);
    mColorThemes[sNr].insert(Edit_currentLineBg,             solarized_linebg);
    mColorThemes[sNr].insert(Edit_errorBg,                   solarized_red.lighter(130));
    mColorThemes[sNr].insert(Edit_currentWordBg,             solarized_lightgreen);
    mColorThemes[sNr].insert(Edit_findBg,                    QColor(145, 149, 211));
    mColorThemes[sNr].insert(Edit_searchBg,                  QColor(147, 197, 233));
    mColorThemes[sNr].insert(Edit_findFg,                    solarized_base3);
    mColorThemes[sNr].insert(Edit_searchFg,                  solarized_base3);
    mColorThemes[sNr].insert(Edit_foldLineBg,                solarized_yellow);
    mColorThemes[sNr].insert(Edit_foldLineFg,                solarized_base2);

    mColorThemes[sNr].insert(Edit_parenthesesValidFg,        solarized_base02);
    mColorThemes[sNr].insert(Edit_parenthesesInvalidFg,      solarized_red);
    mColorThemes[sNr].insert(Edit_parenthesesValidBg,        solarized_orange.lighter(120));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidBg,      solarized_base01.lighter(125));
    mColorThemes[sNr].insert(Edit_parenthesesValidBgBlink,   solarized_orange.lighter(150));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidBgBlink, solarized_base1.lighter(125));
    mColorThemes[sNr].insert(Edit_linenrAreaBg,              solarized_base2);
    mColorThemes[sNr].insert(Edit_linenrAreaMarkBg,          solarized_base1.lighter(125));
    mColorThemes[sNr].insert(Edit_linenrAreaFoldBg,          solarized_yellow);
    mColorThemes[sNr].insert(Edit_profilingBg1,              solarized_green);
    mColorThemes[sNr].insert(Edit_profilingBg2,              solarized_yellow);
    mColorThemes[sNr].insert(Edit_profilingBg3,              solarized_red);
    mColorThemes[sNr].insert(Edit_linenrAreaFg,              solarized_base1);
    mColorThemes[sNr].insert(Edit_linenrAreaMarkFg,          solarized_base01);
    mColorThemes[sNr].insert(Edit_logRemoteBk,               solarized_linebg);

    mColorThemes[sNr].insert(Mark_errorFg,                   solarized_red.lighter(130));
    mColorThemes[sNr].insert(Mark_listingFg,                 solarized_cyan);
    mColorThemes[sNr].insert(Mark_fileFg,                    solarized_green);

    mColorThemes[sNr].insert(Icon_Gray,                      solarized_base1);
    mColorThemes[sNr].insert(Icon_Back,                      solarized_base0);
    mColorThemes[sNr].insert(Icon_Paper,                     solarized_base3);
    mColorThemes[sNr].insert(Active_Gray,                    solarized_base3);
    mColorThemes[sNr].insert(Active_Back,                    solarized_base01.lighter(125));
    mColorThemes[sNr].insert(Disable_Gray,                   solarized_base01.lighter(175));
    mColorThemes[sNr].insert(Disable_Back,                   solarized_base1.lighter(125));
    mColorThemes[sNr].insert(Select_Gray,                    solarized_blue);
    mColorThemes[sNr].insert(Select_Back,                    solarized_blue);
    mColorThemes[sNr].insert(Normal_Red,                     light_red);
    mColorThemes[sNr].insert(Normal_Green,                   light_green);
    mColorThemes[sNr].insert(Normal_Blue,                    light_blue);
    mColorThemes[sNr].insert(Normal_Yellow,                  light_yellow);

    mColorThemes[sNr].insert(Syntax_undefined,               CUndefined);
    mColorThemes[sNr].insert(Syntax_dco,                     Color(solarized_red, fBold));
    mColorThemes[sNr].insert(Syntax_dco_bg,                  CAutoBackground);
    mColorThemes[sNr].insert(Syntax_formula,                 Color(solarized_base01));
    mColorThemes[sNr].insert(Syntax_dcoBody,                 Color(solarized_violet, fItalic));
    mColorThemes[sNr].insert(Syntax_dcoBody_bg,              CAutoBackground);
    mColorThemes[sNr].insert(Syntax_comment,                 Color(solarized_base00, fItalic));
    mColorThemes[sNr].insert(Syntax_comment_bg,              CAutoBackground);
    mColorThemes[sNr].insert(Syntax_title,                   Color(solarized_yellow, fBold));
    mColorThemes[sNr].insert(Syntax_keyword,                 Color(solarized_violet, fBold));
    mColorThemes[sNr].insert(Syntax_keyword_bg,              CAutoBackground);
    mColorThemes[sNr].insert(Syntax_declaration,             Color(solarized_magenta, fBold));
    mColorThemes[sNr].insert(Syntax_declaration_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_identifier,              solarized_green);
    mColorThemes[sNr].insert(Syntax_identifier_bg,           CAutoBackground);
    mColorThemes[sNr].insert(Syntax_description,             solarized_blue);
    mColorThemes[sNr].insert(Syntax_description_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_assignLabel,             solarized_yellow);
    mColorThemes[sNr].insert(Syntax_assignLabel_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_assignValue,             solarized_cyan);
    mColorThemes[sNr].insert(Syntax_assignValue_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_tableHeader,             solarized_yellow);
    mColorThemes[sNr].insert(Syntax_tableHeader_bg,          CAutoBackground);
    mColorThemes[sNr].insert(Syntax_embedded,                solarized_orange);
    mColorThemes[sNr].insert(Syntax_embedded_bg,             CAutoBackground);

    mColorThemes[sNr].insert(Window_window,                  solarized_base2);
    mColorThemes[sNr].insert(Window_windowText,              solarized_base02);
    mColorThemes[sNr].insert(Window_base,                    solarized_base3);
    mColorThemes[sNr].insert(Window_alternateBase,           solarized_base2);
    mColorThemes[sNr].insert(Window_text,                    solarized_base02);
    mColorThemes[sNr].insert(Window_button,                  solarized_base2);
    mColorThemes[sNr].insert(Window_buttonText,              solarized_base02);
    mColorThemes[sNr].insert(Window_placeHolderText,         solarized_base01);
    mColorThemes[sNr].insert(Window_highlight,               solarized_highlight);
    mColorThemes[sNr].insert(Window_highlightedText,         solarized_base02);
    mColorThemes[sNr].insert(Window_link,                    solarized_blue);

    mColorThemes[sNr].insert(Window_tooltipBase,             buttery_yellow);
    mColorThemes[sNr].insert(Window_tooltipText,             light_black);
    mColorThemes[sNr].insert(Window_labelHighlight,          solarized_highlight);
    mColorThemes[sNr].insert(Window_labelHighlightText,      light_black);

    mColorThemes[sNr].insert(Window_disable_window,          solarized_base2.darker(90));
    mColorThemes[sNr].insert(Window_disable_base,            solarized_base2);
    mColorThemes[sNr].insert(Window_disable_button,          solarized_base2.darker(90));
    mColorThemes[sNr].insert(Window_disable_alternateBase,   solarized_base2.darker(90));

    mColorThemes[sNr].insert(Window_disable_windowText,      light_inactiveWhite);
    mColorThemes[sNr].insert(Window_disable_text,            light_inactiveWhite);
    mColorThemes[sNr].insert(Window_disable_buttonText,      light_inactiveWhite);

    mColorThemes[sNr].insert(Window_disable_hightlightText,  light_lightGray);
    mColorThemes[sNr].insert(Window_disable_highlight,       light_darkerWhite);

    mColorThemes[sNr].insert(Window_disable_placeholderText, light_gray);
    mColorThemes[sNr].insert(Window_disable_link,            light_gray);

    // Solarized dark Theme
    mColorThemes << mColorThemes.at(sNr++);
    mThemeNames  << "Solarized - dark";
    mThemeBases  << sNr;
    mColorThemes[sNr].insert(invalid,                        CUndefined);
    mColorThemes[sNr].insert(Edit_text,                      solarized_base1.lighter(120));
    mColorThemes[sNr].insert(Syntax_neutral,                 solarized_base1);
    mColorThemes[sNr].insert(Edit_background,                solarized_base02);
    mColorThemes[sNr].insert(Edit_currentLineBg,             solarized_cyan.darker(190));
    mColorThemes[sNr].insert(Edit_errorBg,                   solarized_red.lighter(50));
    mColorThemes[sNr].insert(Edit_currentWordBg,             dark_background);
    mColorThemes[sNr].insert(Edit_findBg,                    QColor(145, 149, 211));
    mColorThemes[sNr].insert(Edit_searchBg,                  solarized_blue.lighter(120));
    mColorThemes[sNr].insert(Edit_findFg,                    solarized_base2);
    mColorThemes[sNr].insert(Edit_searchFg,                  solarized_base2);
    mColorThemes[sNr].insert(Edit_foldLineBg,                solarized_green.darker(90));
    mColorThemes[sNr].insert(Edit_foldLineFg,                solarized_base02);
    mColorThemes[sNr].insert(Edit_parenthesesValidFg,        solarized_base02);
    mColorThemes[sNr].insert(Edit_parenthesesInvalidFg,      solarized_base02);
    mColorThemes[sNr].insert(Edit_parenthesesValidBg,        solarized_cyan.darker(90));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidBg,      solarized_orange.darker(90));
    mColorThemes[sNr].insert(Edit_parenthesesValidBgBlink,   solarized_cyan.darker(90));
    mColorThemes[sNr].insert(Edit_parenthesesInvalidBgBlink, solarized_orange.darker(120));
    mColorThemes[sNr].insert(Edit_linenrAreaBg,              solarized_base03);
    mColorThemes[sNr].insert(Edit_linenrAreaMarkBg,          solarized_cyan.darker(150));
    mColorThemes[sNr].insert(Edit_linenrAreaFoldBg,          solarized_green.darker(90));
    mColorThemes[sNr].insert(Edit_profilingBg1,              solarized_green);
    mColorThemes[sNr].insert(Edit_profilingBg2,              solarized_yellow);
    mColorThemes[sNr].insert(Edit_profilingBg3,              solarized_red);
    mColorThemes[sNr].insert(Edit_linenrAreaFg,              solarized_base1);
    mColorThemes[sNr].insert(Edit_linenrAreaMarkFg,          solarized_base03);
    mColorThemes[sNr].insert(Edit_logRemoteBk,               solarized_cyan.darker(190));

    mColorThemes[sNr].insert(Icon_Gray,                      solarized_base01);
    mColorThemes[sNr].insert(Icon_Back,                      solarized_base0);
    mColorThemes[sNr].insert(Icon_Paper,                     solarized_base3);
    mColorThemes[sNr].insert(Active_Gray,                    dark_unobstrusive.darker(150));
    mColorThemes[sNr].insert(Active_Back,                    solarized_base0);
    mColorThemes[sNr].insert(Disable_Gray,                   solarized_base0.darker(190));
    mColorThemes[sNr].insert(Disable_Back,                   dark_unobstrusive.darker(150));
    mColorThemes[sNr].insert(Select_Gray,                    solarized_blue);
    mColorThemes[sNr].insert(Select_Back,                    solarized_blue);
    mColorThemes[sNr].insert(Normal_Red,                     dark_red);
    mColorThemes[sNr].insert(Normal_Green,                   dark_green);
    mColorThemes[sNr].insert(Normal_Blue,                    dark_blue);
    mColorThemes[sNr].insert(Normal_Yellow,                  dark_yellow);

    mColorThemes[sNr].insert(Window_window,                  solarized_base03);
    mColorThemes[sNr].insert(Window_windowText,              solarized_base0);
    mColorThemes[sNr].insert(Window_base,                    solarized_base02);
    mColorThemes[sNr].insert(Window_alternateBase,           QColor(66, 83, 88));
    mColorThemes[sNr].insert(Window_text,                    solarized_base0);
    mColorThemes[sNr].insert(Window_button,                  solarized_base02);
    mColorThemes[sNr].insert(Window_buttonText,              solarized_base1);
    mColorThemes[sNr].insert(Window_placeHolderText,         solarized_base01);
    mColorThemes[sNr].insert(Window_highlight,               solarized_cyan);
    mColorThemes[sNr].insert(Window_highlightedText,         solarized_base02);
    mColorThemes[sNr].insert(Window_link,                    solarized_blue);

    mColorThemes[sNr].insert(Window_tooltipBase,             dark_base);
    mColorThemes[sNr].insert(Window_tooltipText,             dark_neutral);
    mColorThemes[sNr].insert(Window_labelHighlight,          solarized_cyan);
    mColorThemes[sNr].insert(Window_labelHighlightText,      solarized_base1.lighter(120));

    mColorThemes[sNr].insert(Window_disable_window,          solarized_base03.darker(120));
    mColorThemes[sNr].insert(Window_disable_base,            solarized_base03.darker(120));
    mColorThemes[sNr].insert(Window_disable_button,          solarized_base03.darker(120));
    mColorThemes[sNr].insert(Window_disable_alternateBase,   solarized_base03.darker(120));

    mColorThemes[sNr].insert(Window_disable_windowText,      solarized_base01.darker(120));
    mColorThemes[sNr].insert(Window_disable_text,            solarized_base01.darker(120));
    mColorThemes[sNr].insert(Window_disable_buttonText,      solarized_base01.darker(120));

    mColorThemes[sNr].insert(Window_disable_hightlightText,  light_lightGray);
    mColorThemes[sNr].insert(Window_disable_highlight,       light_gray.darker(190));

    mColorThemes[sNr].insert(Window_disable_placeholderText, light_gray);
    mColorThemes[sNr].insert(Window_disable_link,            light_gray);

    mFixedThemeCount = mColorThemes.size();

    invalidate();
}

QStringList Theme::themes()
{
    return mThemeNames;
}

int Theme::setActiveTheme(const QString &themeName)
{
    int theme = mThemeNames.indexOf(themeName);
    return setActiveTheme(theme);
}

int Theme::setActiveTheme(int theme)
{
    if (theme < 0 || theme >= mThemeNames.size()) return -1;
    bool changed = mActiveTheme != theme;
    mActiveTheme = theme;
    if (changed)
        invalidate();
    return theme;
}

QString Theme::renameActiveTheme(const QString &name)
{
    QString currentName = mThemeNames.at(mActiveTheme);
    if (mActiveTheme < mFixedThemeCount) return currentName;
    if (name.compare(currentName) == 0) return name;
    QString uniqueName = findUniqueName(name, currentName);
    mThemeNames.replace(mActiveTheme, uniqueName);
    if (mActiveTheme < mThemeNames.count()-1) {
        mThemeNames.move(mActiveTheme, mThemeNames.count()-1);
        mThemeBases.move(mActiveTheme, mThemeNames.count()-1);
        mColorThemes.move(mActiveTheme, mThemeNames.count()-1);
        mActiveTheme = mThemeNames.count()-1;
    }

    // restore sort order
    int i = mThemeNames.count() - 1;
    while (i > mFixedThemeCount && mThemeNames.at(i-1).compare(name, Qt::CaseInsensitive) > 0) --i;

    int last = mThemeNames.count() - 1;
    if (i < last) {
        mThemeNames.move(last, i);
        mThemeBases.move(last, i);
        mColorThemes.move(last, i);
        mActiveTheme = i;
    }

    return uniqueName;
}

int Theme::activeTheme() const
{
    return mActiveTheme;
}

QString Theme::activeThemeName()
{
    return mThemeNames.at(mActiveTheme);
}

int Theme::baseTheme(int theme) const
{
    if (theme < 0 || theme >= mThemeBases.size()) return -1;
    return mThemeBases.at(theme);
}

void Theme::fillThemeColorPalette(QPalette &palette, bool useBaseBackground, bool highlighTransparent)
{
    palette.setColor(QPalette::Window,          useBaseBackground ? Theme::color(Theme::Edit_background)
                                                                  : Theme::color(Theme::Window_window));
    palette.setColor(QPalette::WindowText,      Theme::color(Theme::Window_text));
    palette.setColor(QPalette::Base,            Theme::color(Theme::Window_base));
    palette.setColor(QPalette::AlternateBase,   Theme::color(Theme::Window_alternateBase));
    palette.setColor(QPalette::Text,            Theme::color(Theme::Window_text));
    palette.setColor(QPalette::Button,          Theme::color(Theme::Window_button));
    palette.setColor(QPalette::ButtonText,      Theme::color(Theme::Window_buttonText));
    palette.setColor(QPalette::PlaceholderText, Theme::color(Theme::Window_placeHolderText));
    palette.setColor(QPalette::Highlight,       highlighTransparent ? Qt::transparent
                                                                    : Theme::color(Theme::Window_highlight));
    palette.setColor(QPalette::HighlightedText, Theme::color(Theme::Window_highlightedText));
    palette.setColor(QPalette::Link,            Theme::color(Theme::Window_link));

    // Muted text to signal un-clickable layout
    palette.setColor(QPalette::Disabled, QPalette::Window,          Theme::color(Window_disable_window));
    palette.setColor(QPalette::Disabled, QPalette::Base,            Theme::color(Window_disable_base));
    palette.setColor(QPalette::Disabled, QPalette::Button,          Theme::color(Window_disable_button));
    palette.setColor(QPalette::Disabled, QPalette::AlternateBase,   Theme::color(Window_disable_alternateBase));

    palette.setColor(QPalette::Disabled, QPalette::WindowText,      Theme::color(Window_disable_windowText));
    palette.setColor(QPalette::Disabled, QPalette::Text,            Theme::color(Window_disable_text));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText,      Theme::color(Window_disable_buttonText));

    palette.setColor(QPalette::Disabled, QPalette::Highlight,       Theme::color(Window_disable_highlight));
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, Theme::color(Window_disable_hightlightText));

    palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, Theme::color(Window_disable_placeholderText));
    palette.setColor(QPalette::Disabled, QPalette::Link,            Theme::color(Window_disable_link));
}

void Theme::setThemeColorPalette(QWidget *widget,bool useBaseBackground, bool highlighTransparent)
{
    if (!widget)
        return;

    QPalette palette = widget->palette();
    Theme::fillThemeColorPalette(palette, useBaseBackground, highlighTransparent);
    widget->setPalette(palette);
}

bool Theme::isDark()
{
    return isDark(instance()->mActiveTheme);
}

bool Theme::isDark(int theme)
{
    return isDark(instance()->mColorThemes.at(theme).value(Window_windowText).color,
                  instance()->mColorThemes.at(theme).value(Window_window).color);
}

bool Theme::isDark(QColor foreground, QColor background)
{
    return foreground.lightness() > background.lightness() ;
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
    return slot >= Syntax_undefined && slot < Window_window;
}

Theme::ColorSlot Theme::slot(const QString &name)
{
    bool ok;
    int value = QMetaEnum::fromType<ColorSlot>().keyToValue(name.toLatin1().data(), &ok);
    if (!ok) return invalid;
    return ColorSlot(value);
}

QHash<QString, QStringList> Theme::iconCodes() const
{
    QHash<QString, QStringList> set;
    const ColorTheme &theme = mColorThemes.at(mActiveTheme);
    for (auto it = theme.constBegin() ; it != theme.constEnd() ; ++it) {
        QString slotName = name(it.key());
        if (slotName.startsWith("Icon_")) {
            QString key = slotName.mid(5, slotName.length()-5);
            set.insert(key, QStringList());
            for (int i = 0 ; i < 4 ; ++i)
                set[key] << theme.value(it.key()).color.name();
            set[key] << theme.value(Normal_Red).color.name();
            set[key] << theme.value(Normal_Green).color.name();
            set[key] << theme.value(Normal_Blue).color.name();
            set[key] << theme.value(Normal_Yellow).color.name();
        }
    }
    for (auto it = theme.constBegin() ; it != theme.constEnd() ; ++it) {
        QString slotName = name(it.key());
        if (slotName.startsWith("Disable_")) {
            QString key = slotName.mid(8, slotName.length()-8);
            if (set.contains(key))
                set[key].replace(1, theme.value(it.key()).color.name());
        }
        if (slotName.startsWith("Active_")) {
            QString key = slotName.mid(7, slotName.length()-7);
            if (set.contains(key))
                set[key].replace(2, theme.value(it.key()).color.name());
        }
        if (slotName.startsWith("Select_")) {
            QString key = slotName.mid(7, slotName.length()-7);
            if (set.contains(key))
                set[key].replace(3, theme.value(it.key()).color.name());
        }
    }
    return set;
}

QByteArray Theme::colorizedContent(const QString &name, QIcon::Mode mode, int alpha)
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
                if (data.at(from) == 'Y') colorCode = it.value().at(7);
            }
            while (data.length() > from && data.at(from) != '{') ++from;
            if (data.indexOf("fill:", from) != from+1) continue;
            from += 6;
            int len = data.indexOf(";", from) - from;
            data.replace(from, len, colorCode.toLatin1());
        }
        if (alpha<100 && alpha>=0) {
            int start = data.indexOf("<style");
            QByteArray key("fill-opacity:");
            while (start < end) {
                start = data.indexOf(key, start);
                if (start < 0 || start > end) break;
                start += key.length();
                int semi = data.indexOf(";", start);
                if (semi < 0 || semi > start+3) continue;
                QByteArray sAlpha("0."+QString::number(alpha).toUtf8());
                data.replace(start, semi-start, sAlpha);
            }
        }
    }
    return data;
}

QString Theme::findUniqueName(const QString &name, const QString &ignore)
{
    if (!mThemeNames.contains(name)) return name;
    QString uniqueName = name;
    QString base = name;
    int nr = 0;
    while (!base.isEmpty() && base.at(base.length()-1).isDigit())
        base = base.left(base.length()-1);
    if (base.isEmpty()) base = name;
    if (base.length() != name.length()) nr = name.right(name.length()-base.length()).toInt();
    while (mThemeNames.contains(uniqueName) && uniqueName != ignore)
        uniqueName = base + QString::number(++nr);
    return uniqueName;
}

QString Theme::getNameWithMode(const QString &name, QIcon::Mode mode)
{
    QStringList ext {"_N","_D","_A","_S"};
    return name + ext.at(int(mode));
}

void Theme::invalidate()
{
    mIconCodes = iconCodes();
    mIconCache.clear();
    mDataCache.clear();

    emit changed(false);
}

void Theme::unbind(SvgEngine *engine)
{
    mEngines.removeAll(engine);
}

int Theme::copyTheme(int index, const QString &destName)
{
    QString name = findUniqueName(destName);
    mColorThemes << mColorThemes.at(index);
    mThemeNames  << name;
    mThemeBases  << mThemeBases.at(index);

    // restore sort order
    int i = mThemeNames.count() - 1;
    while (i > mFixedThemeCount && mThemeNames.at(i-1).compare(name, Qt::CaseInsensitive) > 0) --i;

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
    if (index < mFixedThemeCount || index >= mThemeNames.count()) return mActiveTheme;
    if (index <= mActiveTheme) --mActiveTheme;
    mColorThemes.removeAt(index);
    mThemeNames.removeAt(index);
    mThemeBases.removeAt(index);
    return mActiveTheme;
}

QColor Theme::color(Theme::ColorSlot slot)
{
    int theme = instance()->mActiveTheme;
    return instance()->mColorThemes.at(theme).value(slot, CUndefined).color;
}

void Theme::setColor(Theme::ColorSlot slot, QColor color)
{
    int theme = instance()->mActiveTheme;
    Color dat = instance()->mColorThemes.at(theme).value(slot);
    dat.color = color;
    instance()->mColorThemes[theme].insert(slot, dat);
}

QIcon Theme::icon(const QString &name, QIcon::Mode mode, int alpha)
{
    QString nameKey = getNameWithMode(name, mode);
    return icon(name, nameKey, true, QString(), alpha);
}

QIcon Theme::icon(const QString &name, bool forceSquare, const QString &disabledName, int alpha)
{
    return icon(name, name, forceSquare, disabledName, alpha);
}

QIcon Theme::icon(QString name, QString nameKey, bool forceSquare, const QString &disabledName, int alpha)
{
    if (name.contains("%")) name = name.arg(instance()->mIconSet);
    if (nameKey.contains("%")) nameKey = nameKey.arg(instance()->mIconSet);
    nameKey = nameKey + (alpha<100 && alpha>=0 ? QString::number(alpha) : "");
    if (!instance()->mIconCache.contains(nameKey)) {
        SvgEngine *eng = disabledName.isEmpty() ? new SvgEngine(name, alpha)
                                                : new SvgEngine(name, disabledName, alpha);
        if (forceSquare) eng->forceSquare(true);
        instance()->mEngines << eng;
        instance()->mIconCache.insert(nameKey, QIcon(eng));
    }
    return instance()->mIconCache.value(nameKey);
}

QByteArray &Theme::data(const QString &name, QIcon::Mode mode, int alpha)
{
    QStringList ext {"_N","_D","_A","_S"};
    QString nameKey = name + ext.at(int(mode)) + (alpha<100 && alpha>=0 ? QString::number(alpha) : "");
    if (!instance()->mDataCache.contains(nameKey)) {
        QByteArray data(instance()->colorizedContent(name, mode, alpha));
        instance()->mDataCache.insert(nameKey, data);
    }
    return instance()->mDataCache[nameKey];
}

bool Theme::hasFlag(Theme::ColorSlot slot, Theme::FontFlag flag)
{
    int theme = instance()->mActiveTheme;
    Color cl = instance()->mColorThemes.at(theme).value(slot);
    if (flag == fNormal) return (cl.fontFlag == fNormal);
    return (FontFlag(flag & cl.fontFlag) == flag);
}

void Theme::setFlags(Theme::ColorSlot slot, Theme::FontFlag flag)
{
    int theme = instance()->mActiveTheme;
    Color dat = instance()->mColorThemes.at(theme).value(slot);
    dat.fontFlag = flag;
    instance()->mColorThemes[theme].insert(slot, dat);
}

///
/// \brief Theme::mixColor Mixes the color from mixSlot into the one from baseSlot by the ammount of alpha
/// \param baseSlot
/// \param mixSlot
/// \param alpha A value in the interval of [0..1]
/// \return
///
QColor Theme::mixColor(ColorSlot baseSlot, ColorSlot mixSlot, qreal alpha)
{
    QColor res = color(baseSlot);
    return mixColor(res, mixSlot, alpha);
}

///
/// \brief Theme::mixColor Mixes the color from mixSlot into the one from baseSlot by the ammount of alpha
/// \param baseColor
/// \param mixSlot
/// \param alpha A value in the interval of [0..1]
/// \return
///
QColor Theme::mixColor(QColor baseColor, ColorSlot mixSlot, qreal alpha)
{
    QColor res = baseColor;
    QColor mix = color(mixSlot);
    res.setRed  (qRound(alpha * (mix.red()   - res.red()  ) + res.red()));
    res.setGreen(qRound(alpha * (mix.green() - res.green()) + res.green()));
    res.setBlue (qRound(alpha * (mix.blue()  - res.blue() ) + res.blue()));
    return res;
}

qreal CProfileMinAlpha = 0.05;
qreal CProfileMaxAlpha = 0.75;
qreal CProfileLow = 0.05;
qreal CProfileMid = 0.25;

QColor Theme::profileColor(ColorSlot baseSlot, qreal alpha)
{
    ColorSlot mixSlot = alpha < CProfileLow ? Edit_profilingBg1 : alpha < CProfileMid ? Edit_profilingBg2 : Edit_profilingBg3;
    qreal minAlpha = alpha < CProfileLow ? CProfileMinAlpha : CProfileMinAlpha + 0.2;
    qreal shade = alpha < CProfileLow ? alpha / CProfileLow
                                      : alpha < CProfileMid ? (alpha - CProfileLow) / (CProfileMid - CProfileLow)
                                        : (alpha - CProfileMid) / (1. - CProfileMid);
    shade = minAlpha + shade * (CProfileMaxAlpha - minAlpha);
    return mixColor(baseSlot, mixSlot, shade);
}

QVariantList Theme::writeUserThemes() const
{
    QVariantList res;
    res.reserve(mColorThemes.size());
    // starts with index i=mBaseThemeCount (only export user settings, all studio based themes should be fixed)
    for (int i = mFixedThemeCount; i < mColorThemes.size(); ++i) {
        QVariantMap resData;
        const QHash<ColorSlot, Color> &theme = mColorThemes.at(i);
        for (ColorSlot key = invalid; key < ColorSlotCount; key = static_cast<ColorSlot>(key+1)) {
            resData.insert(name(key), theme.value(key).color.name() + "," + QString::number(theme.value(key).fontFlag));
        }
        QVariantMap resTheme;
        resTheme.insert("name",  mThemeNames.at(i));
        resTheme.insert("base",  Theme::isDark(i) ? 1 : 0);
        resTheme.insert("theme", resData);
        res << resTheme;
    }
    return res;
}

void Theme::readUserThemes(const QVariantList &sourceThemes)
{
    // remove user defined themes
    while (mThemeBases.size()  > mFixedThemeCount) mThemeBases.removeLast();
    while (mThemeNames.size()  > mFixedThemeCount) mThemeNames.removeLast();
    while (mColorThemes.size() > mFixedThemeCount) mColorThemes.removeLast();

    // add new user defined themes
    for (const QVariant &vSource: sourceThemes) {
        QVariantMap tSource = vSource.toMap();
        if (tSource.isEmpty() || !tSource.contains("name") || !tSource.contains("theme")) continue;
        QString name = tSource.value("name").toString();

        int base = tSource.value("base").toInt();
        // clone base theme to apply the changes
        int newInd = copyTheme(base, name);
        ColorTheme currentTheme = mColorThemes.at(newInd);

        QVariantMap sourceData = tSource.value("theme").toMap();
        for (auto it = sourceData.constBegin() ; it != sourceData.constEnd() ; ++it) {
            ColorSlot cSlot = slot(it.key());
            if (cSlot == invalid) continue;
            QStringList dat = it.value().toString().split(',');
            if (dat.isEmpty()) continue;
            bool ok = true;
            int iFlag = dat.size() < 2 ? 0 : dat.at(1).toInt(&ok);
            Color color = Color(QColor(dat.at(0)), FontFlag(iFlag));
            currentTheme.insert(cSlot, color);
        }
        mColorThemes.replace(newInd, currentTheme);
    }
}

int Theme::readUserTheme(const QVariantMap &tSource)
{
    if (tSource.isEmpty() || !tSource.contains("name") || !tSource.contains("theme")) return -1;
    QString name = tSource.value("name").toString();
    bool ok;
    int base = tSource.value("base").toInt(&ok);
    if (!ok) base = 0;

    // clone base theme to apply the changes
    int newInd = copyTheme(base, name);
    ColorTheme currentTheme = mColorThemes.at(newInd);

    QVariantMap sourceData = tSource.value("theme").toMap();
    for (auto it = sourceData.constBegin() ; it != sourceData.constEnd() ; ++it) {
        ColorSlot cSlot = slot(it.key());
        if (cSlot == invalid) continue;
        QStringList dat = it.value().toString().split(',');
        if (dat.isEmpty()) continue;
        bool ok = true;
        int iFlag = dat.size() < 2 ? 0 : dat.at(1).toInt(&ok);
        Color color = Color(QColor(dat.at(0)), FontFlag(iFlag));
        currentTheme.insert(cSlot, color);
    }
    mColorThemes.replace(newInd, currentTheme);
    return newInd;
}

int Theme::followOSThemeCount()
{
    return (PaletteStyleManager::currentPlatform() == PaletteStyleManager::Windows
         || PaletteStyleManager::currentPlatform() == PaletteStyleManager::MacOS   ? 1 : 0);
}

QVariantMap Theme::writeCurrentTheme()
{
    QVariantMap resData;
    const QHash<ColorSlot, Color> &theme = mColorThemes.at(mActiveTheme);
    for (ColorSlot key = invalid; key < ColorSlotCount; key = static_cast<ColorSlot>(key+1)) {
        resData.insert(name(key), theme.value(key).color.name() + "," + QString::number(theme.value(key).fontFlag));
    }
    QVariantMap resTheme;
    resTheme.insert("name",  mThemeNames.at(mActiveTheme));
    resTheme.insert("base",  Theme::isDark(mActiveTheme) ? 1 : 0);
    resTheme.insert("theme", resData);
    return resTheme;
}

} // namespace studio
} // namespace gams
