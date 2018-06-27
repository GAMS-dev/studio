#include "systemlogedit.h"
#include "studiosettings.h"

#include <QDesktopServices>

namespace gams {
namespace studio {

SystemLogEdit::SystemLogEdit(StudioSettings *settings, QWidget *parent)
    : AbstractEdit(settings, parent),
      mHighlighter(new SystemLogHighlighter(this))
{
    setTextInteractionFlags(Qt::TextSelectableByMouse);
    setLineWrapMode(AbstractEdit::WidgetWidth);
    setFont(QFont(mSettings->fontFamily(), mSettings->fontSize()));
    setMouseTracking(true);
    mHighlighter->setDocument(document());
}

void SystemLogEdit::appendLog(const QString &msg, LogMsgType type)
{
    if (msg.isEmpty()) return;
    QString logLevel = level(type);
    appendPlainText(logLevel + msg);
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

    for (QTextLayout::FormatRange range : cursor.block().layout()->formats()) {
        if (range.format.isAnchor() && pos >= range.start && pos <= range.start + range.length) {
            QString link = cursor.block().text().mid(range.start, range.length);
            QDesktopServices::openUrl(link);
            event->accept();
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
        return "Info: ";
    case LogMsgType::Warning:
        return "Warning: ";
    case LogMsgType::Error:
        return "Error: ";
    default:
        return QString();
    }
}

}
}
