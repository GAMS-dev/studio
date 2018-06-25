#include "systemlogedit.h"
#include "studiosettings.h"

#include <QString>

namespace gams {
namespace studio {

SystemLogEdit::SystemLogEdit(StudioSettings *settings, QWidget *parent)
    : AbstractEdit(settings, parent)
{
    setTextInteractionFlags(textInteractionFlags() | Qt::TextSelectableByKeyboard);
    setLineWrapMode(AbstractEdit::WidgetWidth);
    setFont(QFont(mSettings->fontFamily(), mSettings->fontSize()));
    setReadOnly(true);
}

void SystemLogEdit::appendLog(const QString &msg, LogMsgType type)
{
    if (msg.isEmpty()) return;

    QString logMsg;
    switch (type) {
    case LogMsgType::Info:
        logMsg.append("<span style='color:blue;font-weight: bold;'>Info:</span> ");
        break;
    case LogMsgType::Warning:
        logMsg.append("<span style='color:orange;font-weight: bold;'>Warning:</span> ");
        break;
    case LogMsgType::Error:
        logMsg.append("<span style='color:red;font-weight: bold;'>Error:</span> ");
        break;
    }

    logMsg.append(msg + "<br/>");

    moveCursor(QTextCursor::End);
    appendHtml(logMsg);
    moveCursor(QTextCursor::End);
}

AbstractEdit::EditorType SystemLogEdit::type()
{
    return EditorType::SystemLog;
}

}
}
