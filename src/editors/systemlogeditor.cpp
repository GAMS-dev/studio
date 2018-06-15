#include "systemlogeditor.h"

#include <QString>


namespace gams {
namespace studio {

SystemLogEditor::SystemLogEditor(StudioSettings *settings, QWidget *parent)
    : AbstractEditor(settings, parent)
{
    setTextInteractionFlags(textInteractionFlags() |  Qt::TextSelectableByKeyboard);
    setLineWrapMode(AbstractEditor::WidgetWidth);
    setFont(QFont(mSettings->fontFamily(), mSettings->fontSize()));
    setReadOnly(true);
}

void SystemLogEditor::appendLog(const QString &msg, LogMsgType type)
{
    if (msg.isEmpty()) return;

    QString logMsg("");

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
    default:
        break;
    }

    logMsg.append(msg + "<br/>");

    moveCursor(QTextCursor::End);
    appendHtml(logMsg);
    moveCursor(QTextCursor::End);
}

AbstractEditor::EditorType SystemLogEditor::type()
{
    return EditorType::SystemLogEditor;
}

}
}
