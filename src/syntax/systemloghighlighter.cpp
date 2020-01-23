/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
 */
#include "systemloghighlighter.h"

namespace gams {
namespace studio {

const QString HighlightingData::ErrorKeyword = "Error";
const QString HighlightingData::InfoKeyword = "Info";
const QString HighlightingData::WarningKeyword = "Warning";

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
    for (const HighlightingRule &rule : mHighlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern().globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format());
        }
    }
}

}
}
