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
#include "systemloghighlighter.h"

namespace gams {
namespace studio {

const QString HighlightingData::ErrorKeyword = "Error";
const QString HighlightingData::InfoKeyword = "Info";
const QString HighlightingData::WarningKeyword = "Warning";

const QString HighlightingRule::timestampRegex = " \\[\\d\\d:\\d\\d:\\d\\d\\]:";

SystemLogHighlighter::SystemLogHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
{
    mHighlightingRules.push_back(ErrorHighlightingRule());
    mHighlightingRules.push_back(InfoHighlightingRule());
    mHighlightingRules.push_back(WarningHighlightingRule());
    mHighlightingRules.push_back(LinkHighlightingRule());
}

void SystemLogHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : std::as_const(mHighlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern().globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format());
        }
    }
}

}
}
