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
            RawPlaceholder ph;
            if (colon < 0) {
                ph.order = inner.toInt();
                ph.textStart = plain.size();
                ph.textLen = 0;
            } else {
                ph.order = inner.left(colon).toInt();
                ph.textStart = plain.size();
                const QString def = inner.mid(colon + 1);
                ph.textLen = def.size();
                plain += def;
            }
            raw << ph;
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
    for (const RawPlaceholder &ph : std::as_const(raw)) {
        Placeholder p;
        p.order  = ph.order;
        p.cursor = QTextCursor(mEdit->document());
        p.cursor.setPosition(insertPos + ph.textStart);
        if (ph.textLen > 0)
            p.cursor.setPosition(insertPos + ph.textStart + ph.textLen, QTextCursor::KeepAnchor);
        const int key = (ph.order == 0) ? INT_MAX : ph.order;
        ordered.insert(key, p);
    }
    mPlaceholders = ordered.values();
}

void SnippetControl::activateSnippet(QPlainTextEdit *edit, const QString &body, int insertPos)
{
    mEdit = edit;
    mCurrentIndex = 0;
    parseAndInsert(body, insertPos);
    if (mPlaceholders.isEmpty()) {
        deactivate();
        return;
    }
    selectPlaceholder(0);
}

void SnippetControl::selectPlaceholder(int index)
{
    if (!mEdit || index < 0 || index >= mPlaceholders.size()) {
        deactivate();
        return;
    }
    mCurrentIndex = index;
    const Placeholder &ph = mPlaceholders.at(index);

    QTextCursor cur = mEdit->textCursor();
    if (ph.cursor.hasSelection()) {
        mEdit->setTextCursor(ph.cursor);
    } else {
        cur.setPosition(ph.cursor.position());
        mEdit->setTextCursor(cur);
    }

    // deactivate after placing cursor on exit marker
    if (ph.order == 0)
        deactivate();
}

bool SnippetControl::handleKey(QKeyEvent *e)
{
    if (!isActive()) return false;

    switch (e->key()) {
    case Qt::Key_Tab:
        if (e->modifiers() & Qt::ShiftModifier) {
            if (mCurrentIndex > 0)
                selectPlaceholder(mCurrentIndex - 1);
        } else {
            if (mCurrentIndex + 1 < mPlaceholders.size())
                selectPlaceholder(mCurrentIndex + 1);
            else
                deactivate();
        }
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
        deactivate();
        return true;

    case Qt::Key_Escape:
        deactivate();
        return true;

    default:
        break;
    }
    return false;
}

void SnippetControl::deactivate()
{
    mEdit = nullptr;
    mPlaceholders.clear();
    mCurrentIndex = 0;
}

} // namespace studio
} // namespace gams
