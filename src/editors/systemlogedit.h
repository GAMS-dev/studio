#ifndef SYSTEMLOGEDIT_H
#define SYSTEMLOGEDIT_H

#include "abstractedit.h"

namespace gams {
namespace studio {

class StudioSettings;
class SystemLogHighlighter;

enum class LogMsgType { Error, Warning, Info };

class SystemLogEdit : public AbstractEdit
{
public:
    SystemLogEdit(StudioSettings *settings, QWidget *parent);
    void appendLog(const QString &msg, LogMsgType type = LogMsgType::Warning);

    EditorType type() override;

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QString level(LogMsgType type);

private:
    SystemLogHighlighter *mHighlighter;
};

}
}

#endif // SYSTEMLOGEDIT_H
