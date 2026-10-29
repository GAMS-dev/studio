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
#include "textblankops.h"

#include <QTextDocument>
#include <algorithm>
#include <utility>
#include <vector>

namespace gams {
namespace studio {

bool isBlank(QChar c)
{
    return c.isSpace() && c != QChar::LineSeparator;
}

void replaceRange(QTextCursor &cursor, int pos, int len, const QString &text)
{
    cursor.setPosition(pos);
    cursor.setPosition(pos + len, QTextCursor::KeepAnchor);
    cursor.insertText(text);
}

TextBlankOps::TextBlankOps(const QTextCursor &cursor, int tabWidth)
    : mCursor(cursor), mTabWidth(std::max(1, tabWidth))
{
}

void TextBlankOps::apply(Flags flags)
{
    if (mCursor.isNull() || !flags) return;
    QTextCursor edit(mCursor);
    edit.beginEditBlock();
    if (flags.testFlag(TabToSpace)) tabToSpace();
    if (flags & Trim) trim(flags.testFlag(TrimLeading), flags.testFlag(TrimTrailing));
    if (flags.testFlag(EolToSpace)) eolToSpace();
    edit.endEditBlock();
}

void TextBlankOps::eolToSpace()
{
    QTextDocument *doc = mCursor.document();
    const QTextBlock first = doc->findBlock(mCursor.selectionStart());
    const QTextBlock last = mCursor.hasSelection() ? doc->findBlock(mCursor.selectionEnd()) : first.next();
    if (!last.isValid() || first == last) return;

    QTextCursor edit(mCursor);
    edit.beginEditBlock();
    for (QTextBlock b = last.previous(); b.isValid() && b.position() >= first.position(); b = b.previous()) {
        const QString t = b.text();
        const QString nt = b.next().text();
        int trail = 0, lead = 0;
        while (trail < t.size() && isBlank(t.at(t.size() - 1 - trail))) ++trail;
        while (lead < nt.size() && isBlank(nt.at(lead))) ++lead;
        replaceRange(edit, b.position() + t.size() - trail, trail + 1 + lead, QStringLiteral(" "));
    }
    edit.endEditBlock();
}

void TextBlankOps::tabToSpace()
{
    forEachLine([this](QTextCursor &edit, const QTextBlock &b) {
        const QString t = b.text();
        std::vector<std::pair<int, int>> reps;
        for (int i = 0, col = 0; i < t.size(); ++i) {
            const bool tab = t.at(i) == QLatin1Char('\t');
            const int w = tab ? mTabWidth - col % mTabWidth : 1;
            if (tab) reps.emplace_back(i, w);
            col += w;
        }
        for (auto it = reps.rbegin(); it != reps.rend(); ++it)
            replaceRange(edit, b.position() + it->first, 1, QString(it->second, QChar(' ')));
    });
}

void TextBlankOps::trim(bool leading, bool trailing)
{
    forEachLine([=](QTextCursor &edit, const QTextBlock &b) {
        const QString t = b.text();
        const int n = t.size();
        int lead = 0, trail = 0;
        while (lead < n && isBlank(t.at(lead))) ++lead;
        while (trail < n && isBlank(t.at(n - 1 - trail))) ++trail;
        lead = leading ? lead : 0;
        trail = trailing ? std::min(trail, n - lead) : 0;
        replaceRange(edit, b.position() + n - trail, trail, QString());
        replaceRange(edit, b.position(), lead, QString());
    });
}

void TextBlankOps::forEachLine(const std::function<void(QTextCursor &, const QTextBlock &)> &fn)
{
    QTextDocument *doc = mCursor.document();
    QTextBlock b = doc->findBlock(mCursor.selectionStart());
    QTextBlock last = doc->findBlock(mCursor.selectionEnd());
    if (mCursor.hasSelection() && last != b && mCursor.selectionEnd() == last.position())
        last = last.previous();

    QTextCursor edit(mCursor);
    edit.beginEditBlock();
    for (;; b = b.next()) {
        fn(edit, b);
        if (b == last) break;
    }
    edit.endEditBlock();
}

}
}
