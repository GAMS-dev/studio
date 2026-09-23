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
#include "snippetcontrol.h"
#include "logger.h"

#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QTextDocument>

namespace gams {
namespace studio {

struct RawPlaceholder {
    int order;
    int textStart;
    int textLen;
};

void SnippetControl::load(const QList<Snippet> &snippets)
{
    mSnippets = snippets;
}

void SnippetControl::parseAndInsert(const QString &body, int insertPos)
{
    mPlaceholders.clear();

    QList<RawPlaceholder> raw;
    QString plain;
    plain.reserve(body.size());

    int i = 0;
    while (i < body.size()) {
        if (body.at(i) == '$' && i + 1 < body.size() && body.at(i+1) == '{') {
            int close = body.indexOf('}', i + 2);
            if (close < 0) { plain += body.at(i++); continue; }
            const QString inner = body.mid(i + 2, close - i - 2);
            const int colon = inner.indexOf(':');
            RawPlaceholder rph;
            if (colon < 0) {
                rph.order = inner.toInt();
                rph.textStart = plain.size();
                rph.textLen = 0;
            } else {
                rph.order = inner.left(colon).toInt();
                rph.textStart = plain.size();
                const QString def = inner.mid(colon + 1);
                rph.textLen = def.size();
                plain += def;
            }
            raw << rph;
            i = close + 1;
        } else {
            plain += body.at(i++);
        }
    }

    QTextCursor cur(mEdit->document());
    cur.setPosition(insertPos);
    cur.insertText(plain);

    // Build placeholders sorted by order, exit marker (0) always at end
    QMap<int, Placeholder> ordered;
    for (const RawPlaceholder &rph : std::as_const(raw)) {
        Placeholder ph;
        ph.order  = rph.order;
        if (insertPos + rph.textStart > 0) {
            ph.cursor1 = QTextCursor(mEdit->document());
            ph.cursor1.setPosition(insertPos + rph.textStart - 1);
        } else
            ph.cursor1 = QTextCursor();
        ph.cursor2 = QTextCursor(mEdit->document());
        ph.cursor2.setPosition(insertPos + rph.textStart + rph.textLen);
        const int key = (rph.order == 0) ? INT_MAX : rph.order;
        ordered.insert(key, ph);
    }
    mPlaceholders = ordered.values();
}

void SnippetControl::activateSnippet(QPlainTextEdit *edit, const QString &body, int insertPos)
{
    mEdit = edit;
    mCurrentIndex = 0;
    parseAndInsert(body, insertPos);
    if (mPlaceholders.isEmpty()) {
        deactivate(false);
        return;
    }
    selectPlaceholder(0);
}

void SnippetControl::selectPlaceholder(int index)
{
    if (!mEdit) {
        deactivate(false);
        return;
    }
    if (index < 0) index = mPlaceholders.size() - 1;
    if (index >= mPlaceholders.size()) index = 0;
    mCurrentIndex = index;
    const Placeholder &ph = mPlaceholders.at(index);

    QTextCursor cur = mEdit->textCursor();
    if (ph.cursor1.isNull())
        cur.setPosition(0);
    else
        cur.setPosition(ph.cursor1.position() + 1);
    cur.setPosition(ph.cursor2.position(), QTextCursor::KeepAnchor);
    mEdit->setTextCursor(cur);
}

bool SnippetControl::handleKey(QKeyEvent *e)
{
    if (!isActive()) return false;

    switch (e->key()) {
    case Qt::Key_Backtab:
    case Qt::Key_Tab:
        selectPlaceholder(mCurrentIndex + (e->modifiers() & Qt::ShiftModifier ? -1 : +1));
        return true;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        // jump to exit marker (order==0) if present, else end of last placeholder
        for (int i = 0; i < mPlaceholders.size(); ++i) {
            if (mPlaceholders.at(i).order == 0) {
                selectPlaceholder(i);
                return true;
            }
        }
        // no exit marker — place cursor after last placeholder and deactivate
        selectPlaceholder(mPlaceholders.size() - 1);
        deactivate(false);
        return true;
    case Qt::Key_Escape:
        deactivate(true);
        return true;
    case Qt::Key_Space:
        return (e->modifiers() & Qt::ControlModifier);

    default:
        break;
    }
    return false;
}

void SnippetControl::deactivate(bool unselect)
{
    if (mEdit && unselect) {
        QTextCursor cur = mEdit->textCursor();
        if (cur.hasSelection()) {
            cur.clearSelection();
            mEdit->setTextCursor(cur);
        }
    }
    mEdit = nullptr;
    mPlaceholders.clear();
    mCurrentIndex = 0;
}

QList<QTextCursor> SnippetControl::placeholders() const
{
    QList<QTextCursor> res;
    for (const Placeholder &ph : std::as_const(mPlaceholders)) {
        QTextCursor cur = QTextCursor(mEdit->document());
        if (ph.cursor1.isNull())
            cur.setPosition(0);
        else
            cur.setPosition(ph.cursor1.position() + 1);
        cur.setPosition(ph.cursor2.position(), QTextCursor::KeepAnchor);
        res << cur;
    }
    return res;
}

} // namespace studio
} // namespace gams
