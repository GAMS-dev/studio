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
#include "systemlogedit.h"
#include "syntax/systemloghighlighter.h"
#include "logger.h"

#include <QDesktopServices>
#include <QTime>
#include <QMenu>

namespace gams {
namespace studio {

SystemLogEdit::SystemLogEdit(QWidget *parent)
    : AbstractEdit(parent),
      mHighlighter(new SystemLogHighlighter(this))
{
    setTextInteractionFlags(Qt::TextSelectableByMouse);
    setLineWrapMode(AbstractEdit::WidgetWidth);
    setMouseTracking(true);
    mHighlighter->setDocument(document());
}

void SystemLogEdit::append(const QString &msg, LogMsgType type)
{
    QString out = msg.trimmed();
    if (out.isEmpty()) return;
    QString logLevel = level(type);
    QString time = QTime::currentTime().toString("hh:mm:ss");
    appendPlainText(logLevel + " [" + time + "]: " + out);
    DEB() << logLevel << " [" << time << "]: " << out;
    if (type == LogMsgType::Info)
        emit newMessage(false);
    else
        emit newMessage(true);
}

void SystemLogEdit::mouseMoveEvent(QMouseEvent *event)
{
    QTextCursor cursor = cursorForPosition(event->pos());
    int pos = cursor.positionInBlock();
    Qt::CursorShape shape = Qt::IBeamCursor;
    const auto formats = cursor.block().layout()->formats();
    for (const QTextLayout::FormatRange &range : formats) {
        if (range.format.isAnchor() && pos >= range.start && pos <= range.start + range.length) {
            shape = Qt::PointingHandCursor;
        }
    }
    bool valid = (event->buttons() == Qt::NoButton || (!linkClickPos().isNull() && ((linkClickPos() - event->pos()).manhattanLength() <= 4)));
    if (shape == Qt::IBeamCursor && valid && selectEntry(event->pos(), true)) {
        shape = Qt::ArrowCursor;
    }

    AbstractEdit::mouseMoveEvent(event);
    viewport()->setCursor(shape);
}

void SystemLogEdit::mousePressEvent(QMouseEvent *event)
{
    QTextCursor cursor = cursorForPosition(event->pos());
    int pos = cursor.positionInBlock();

    if (event->button() & Qt::LeftButton) {
        const auto formats = cursor.block().layout()->formats();
        for (const QTextLayout::FormatRange &range : formats) {
            if (range.format.isAnchor() && pos >= range.start && pos <= range.start + range.length) {
                QString link = cursor.block().text().mid(range.start, range.length);
                QDesktopServices::openUrl(link);
                event->accept();
            }
        }
    }

    AbstractEdit::mousePressEvent(event);
    setLinkClickPos(event->pos());
}

void SystemLogEdit::mouseReleaseEvent(QMouseEvent *event)
{
    bool doSelect = !linkClickPos().isNull() && (linkClickPos() - event->pos()).manhattanLength() <= 4;
    AbstractEdit::mouseReleaseEvent(event);
    if (event->modifiers().testFlag(Qt::ShiftModifier)) return;
    if (doSelect) {
        selectEntry(event->pos());
    }
}

void SystemLogEdit::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *menu = createStandardContextMenu();
    for (int i = menu->actions().count()-1; i >= 0; --i) {
        QAction *act = menu->actions().at(i);
        if (act->objectName() == "edit-copy") {
            act->disconnect();
            act->setShortcut(QKeySequence("Ctrl+C"));
            connect(act, &QAction::triggered, this, &SystemLogEdit::copy);
        }
    }
    QAction act("Clear Log", this);
    connect(&act, &QAction::triggered, this, &SystemLogEdit::clear);
    menu->addAction(&act);
    menu->exec(e->globalPos());
    delete menu;
}

AbstractEdit::EditorType SystemLogEdit::type() const
{
    return EditorType::SystemLog;
}

QString SystemLogEdit::level(LogMsgType type)
{
    switch (type) {
    case LogMsgType::Info:
        return HighlightingData::InfoKeyword;
    case LogMsgType::Warning:
        return HighlightingData::WarningKeyword;
    case LogMsgType::Error:
        return HighlightingData::ErrorKeyword;
    }
    return QString();
}

bool SystemLogEdit::selectEntry(QPoint mousePos, bool checkOnly)
{
    QTextCursor cur = cursorForPosition(mousePos);
    QTextBlock block = cur.block();
    int offset = block.text().indexOf("]: ") + 3;
    if (offset > 2 && cur.positionInBlock() < offset) {
        if (checkOnly) return true;
        cur.setPosition(block.position() + offset);
        cur.setPosition(block.position() + block.length() - 1, QTextCursor::KeepAnchor);
        setTextCursor(cur);
        return true;
    }
    return false;
}

}
}
