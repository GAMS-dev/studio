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
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "processlogedit.h"
#include <QMenu>
#include <QScrollBar>
#include <QDebug>

namespace gams {
namespace studio {

ProcessLogEdit::ProcessLogEdit(QWidget *parent)
    : AbstractEdit(parent)
{
    setTextInteractionFlags(Qt::TextSelectableByMouse);
    connect(this, &ProcessLogEdit::textChanged, this, &ProcessLogEdit::updateExtraSelections);
    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &ProcessLogEdit::updateExtraSelections);
}

void ProcessLogEdit::mouseDoubleClickEvent(QMouseEvent *event)
{
    jumpToLst(event->pos(), false);
}

void ProcessLogEdit::mouseReleaseEvent(QMouseEvent *event)
{
    AbstractEdit::mouseReleaseEvent(event);
    if (event->modifiers()==Qt::ControlModifier)
        jumpToLst(event->pos(), true);
}

void ProcessLogEdit::jumpToLst(QPoint pos, bool fuzzy)
{
    QTextCursor cursor = cursorForPosition(pos);
    if (marks()->values(cursor.blockNumber()).isEmpty() || fuzzy) {

        int line = cursor.blockNumber();
        TextMark* linkMark = nullptr;
        for (TextMark *mark: *marks()) {
            if (mark->type() == TextMark::link && mark->refFileKind() == FileKind::Lst) {
                if (mark->line() < line)
                    linkMark = mark;
                else if (!linkMark)
                    linkMark = mark;
                else if (line+1 < mark->line()+mark->spread())
                    break;
                else if (qAbs(linkMark->line()-line) < qAbs(line-mark->line()))
                    break;
                else {
                    linkMark = mark;
                    break;
                }
            }
        }
        if (linkMark)
            linkMark->jumpToRefMark(true);
    }
}

void ProcessLogEdit::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *menu = createStandardContextMenu();
    QAction act("Clear Log", this);
    connect(&act, &QAction::triggered, this, &ProcessLogEdit::clear);
    menu->insertAction(menu->actions().at(1), &act);

    menu->exec(e->globalPos());
    delete menu;
}

void ProcessLogEdit::extraSelCurrentLine(QList<QTextEdit::ExtraSelection> &selections)
{
    Q_UNUSED(selections);
    return;
}

AbstractEdit::EditorType ProcessLogEdit::type()
{
    return EditorType::ProcessLog;
}

}
}
