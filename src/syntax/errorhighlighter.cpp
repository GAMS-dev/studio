/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include "errorhighlighter.h"
#include "syntaxformats.h"
#include "syntaxdeclaration.h"
#include "syntaxidentifier.h"
#include "textmark.h"
#include "logger.h"
#include "exception.h"
#include "file.h"
#include "common.h"

namespace gams {
namespace studio {

ErrorHighlighter::ErrorHighlighter(QTextDocument *doc)
    : QSyntaxHighlighter(doc), mMarks(new LineMarks())
{
}

const LineMarks *ErrorHighlighter::marks() const
{
    return mMarks;
}

void ErrorHighlighter::setMarks(const LineMarks* marks)
{
    mMarks = marks;
    rehighlight();
}

void ErrorHighlighter::syntaxState(int position, int &intState)
{
    mPositionForSyntaxState = position;
    mLastSyntaxState = 0;
    rehighlightBlock(document()->findBlock(position));
    intState = mLastSyntaxState;
    mLastSyntaxState = 0;
}

void ErrorHighlighter::highlightBlock(const QString& text)
{
    if (!marks()) {
        DEB() << "trying to highlight without marks!";
        return;
    }
    QList<TextMark*> markList = marks()->values(FileId(currentBlock().blockNumber()));
    setCombiFormat(0, text.length(), QTextCharFormat(), markList);
}

void ErrorHighlighter::setCombiFormat(int start, int len, const QTextCharFormat &charFormat, QList<TextMark*> markList)
{
    int end = start+len;
    int marksStart = end;
    int marksEnd = start;
    for (TextMark* mark: markList) {
        if (mark->blockStart() < marksStart) marksStart = qMax(start, mark->blockStart());
        if (mark->blockEnd() > marksEnd) marksEnd = qMin(end, mark->blockEnd());
    }
    if (marksStart == end) {
        setFormat(start, len, charFormat);
        return;
    }
    setFormat(start, len, charFormat);
    start = marksStart;
    end = marksEnd;

    for (TextMark* mark: markList) {
        if (mark->blockStart() >= end || mark->blockEnd() < start || true)
            continue;
        QTextCharFormat combinedFormat(charFormat);
        marksStart = qMax(mark->blockStart(), start);
        marksEnd = qMin(mark->blockEnd(), end);
        if (mark->type() == TextMark::error) {
            combinedFormat.setUnderlineColor(Qt::red);
            combinedFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
            combinedFormat.setAnchorName(QString::number(mark->line()));
            setFormat(marksStart, marksEnd-marksStart, combinedFormat);
            if (marksEnd == mark->blockEnd()) {
                combinedFormat.setBackground(QColor(225,200,255));
                combinedFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
                setFormat(marksEnd, 1, combinedFormat);
            }
        }
        if (mark->type() == TextMark::link) {
            combinedFormat.setForeground(mark->color());
            combinedFormat.setUnderlineColor(mark->color());
            combinedFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            combinedFormat.setAnchor(true);
            combinedFormat.setAnchorName(QString::number(mark->line()));
            setFormat(marksStart, marksEnd-marksStart, combinedFormat);
        }
    }
}

} // namespace studio
} // namespace gams
