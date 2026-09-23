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
#ifndef GAMS_STUDIO_SNIPPETCONTROL_H
#define GAMS_STUDIO_SNIPPETCONTROL_H

#include <QList>
#include <QTextCursor>

class QKeyEvent;
class QPlainTextEdit;

namespace gams {
namespace studio {

struct Snippet {
    QString trigger;
    QString label;
    QString description;
    QString body;
};

struct Placeholder {
    int order = 0;          // ${N:...} -> N, 0 = exit marker
    QTextCursor cursor1;    // anchor-1 or NULL when at the start of the document
    QTextCursor cursor2;    // position (end)
};

class SnippetControl
{
public:
    SnippetControl() = default;

    void load(const QList<Snippet> &snippets);

    void activateSnippet(QPlainTextEdit *edit, const QString &body, int insertPos);
    bool handleKey(QKeyEvent *e);
    bool isActive() const { return mEdit != nullptr; }
    void deactivate(bool unselect);
    QList<QTextCursor> placeholders() const;

private:
    void parseAndInsert(const QString &body, int insertPos);
    void selectPlaceholder(int index);

    QList<Snippet> mSnippets;
    QPlainTextEdit *mEdit = nullptr;
    QList<Placeholder> mPlaceholders; // sorted by tab order, exit marker last
    int mCurrentIndex = 0;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_SNIPPETCONTROL_H
