#include "systemlogeditor.h"

#include <QString>


namespace gams {
namespace studio {

SystemLogEditor::SystemLogEditor(StudioSettings *settings, QWidget *parent)
    : AbstractEditor(settings, parent)
{
    setTextInteractionFlags(textInteractionFlags() |  Qt::TextSelectableByKeyboard);
    setLineWrapMode(AbstractEditor::WidgetWidth);
    setReadOnly(true);
}

void SystemLogEditor::appendLog(const QString &msg)
{
    if (msg.isEmpty()) return;

    appendPlainText(msg);

//    QPlainTextEdit *outWin = ui->systemLogView;
//    if (!text.isNull()) {
//        outWin->moveCursor(QTextCursor::End);
//        outWin->insertPlainText(text + "\n");
//        outWin->moveCursor(QTextCursor::End);
//        outWin->document()->setModified(false);
    //    }
}

AbstractEditor::EditorType SystemLogEditor::type()
{
    return EditorType::SystemLogEditor;
}

}
}
