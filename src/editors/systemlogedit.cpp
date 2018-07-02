#include "systemlogedit.h"
#include "syntax/systemloghighlighter.h"

#include <QDesktopServices>

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

void SystemLogEdit::appendLog(const QString &msg, LogMsgType type)
{
    if (msg.isEmpty()) return;
    QString logLevel = level(type);
    appendPlainText(logLevel + " " + msg);
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
    default:
        return QString();
    }
}

}
}
