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
#include "systemlogedit.h"
#include "syntax/systemloghighlighter.h"
#include "logger.h"

#include <QDesktopServices>
#include <QTime>

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
}

void SystemLogEdit::mouseMoveEvent(QMouseEvent *event)
{
    QTextCursor cursor = cursorForPosition(event->pos());
    int pos = cursor.positionInBlock();

    for (QTextLayout::FormatRange range : cursor.block().layout()->formats()) {
        if (range.format.isAnchor() && pos >= range.start && pos <= range.start + range.length) {
            Qt::CursorShape shape = Qt::PointingHandCursor;
            viewport()->setCursor(shape);
        } else {
            Qt::CursorShape shape = Qt::IBeamCursor;
            viewport()->setCursor(shape);
        }
    }

    AbstractEdit::mouseMoveEvent(event);
}

void SystemLogEdit::mousePressEvent(QMouseEvent *event)
{
    QTextCursor cursor = cursorForPosition(event->pos());
    int pos = cursor.positionInBlock();

    if (event->button() & Qt::LeftButton) {
        for (QTextLayout::FormatRange range : cursor.block().layout()->formats()) {
            if (range.format.isAnchor() && pos >= range.start && pos <= range.start + range.length) {
                QString link = cursor.block().text().mid(range.start, range.length);
                QDesktopServices::openUrl(link);
                event->accept();
            }
        }
    }

    AbstractEdit::mousePressEvent(event);
}

AbstractEdit::EditorType SystemLogEdit::type()
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

}
}
