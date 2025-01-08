/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#ifndef SYSTEMLOGHIGHLIGHTER_H
#define SYSTEMLOGHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include "theme.h"

namespace gams {
namespace studio {

class HighlightingData
{
private:
    HighlightingData() {}

public:
    static const QString ErrorKeyword;
    static const QString InfoKeyword;
    static const QString WarningKeyword;
};

class HighlightingRule
{
public:
    QRegularExpression pattern() const
    {
        return mPattern;
    }

    QTextCharFormat format() const
    {
        return mFormat;
    }

protected:
    QRegularExpression mPattern;
    QTextCharFormat mFormat;
    static const QString timestampRegex;
};

class LinkHighlightingRule
    : public HighlightingRule
{
public:
    LinkHighlightingRule()
    {
        mPattern = QRegularExpression("http[s]?://(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\\(\\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))+");
        mFormat.setForeground(Theme::color(Theme::Normal_Blue));
        mFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        mFormat.setAnchor(true);
    }
};

class ErrorHighlightingRule
    : public HighlightingRule
{
public:
    ErrorHighlightingRule()
    {
        mPattern = QRegularExpression(HighlightingData::ErrorKeyword + timestampRegex);
        mFormat.setForeground(Theme::color(Theme::Normal_Red));
        mFormat.setFontWeight(QFont::Bold);
    }
};

class InfoHighlightingRule
    : public HighlightingRule
{
public:
    InfoHighlightingRule()
    {
        mPattern = QRegularExpression(HighlightingData::InfoKeyword + timestampRegex);
        mFormat.setForeground(Theme::color(Theme::Normal_Green));
        mFormat.setFontWeight(QFont::Bold);
    }
};

class WarningHighlightingRule
    : public HighlightingRule
{
public:
    WarningHighlightingRule()
    {
        mPattern = QRegularExpression(HighlightingData::WarningKeyword + timestampRegex);
        mFormat.setForeground(Qt::darkYellow);
        mFormat.setFontWeight(QFont::Bold);
    }
};


class SystemLogHighlighter
    : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SystemLogHighlighter(QObject *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    QVector<HighlightingRule> mHighlightingRules;
};

}
}

#endif // SYSTEMLOGHIGHLIGHTER_H
