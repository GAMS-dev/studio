#include "systemlogedit.h"
#include "studiosettings.h"

#include <QString>

namespace gams {
namespace studio {

SystemLogEdit::SystemLogEdit(StudioSettings *settings, QWidget *parent)
    : AbstractEdit(settings, parent),
      mHighlighter(new SystemLogHighlighter(this))
{
    setTextInteractionFlags(Qt::TextBrowserInteraction);
    setLineWrapMode(AbstractEdit::WidgetWidth);
    setFont(QFont(mSettings->fontFamily(), mSettings->fontSize()));
    mHighlighter->setDocument(document());
}

void SystemLogEdit::appendLog(const QString &msg, LogMsgType type)
{
    if (msg.isEmpty()) return;
    QString logLevel = level(type);
    appendPlainText(logLevel + msg);
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
