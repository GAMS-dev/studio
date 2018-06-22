#ifndef SYSTEMLOGEDITOR_H
#define SYSTEMLOGEDITOR_H

#include "abstracteditor.h"

namespace gams {
namespace studio {

class StudioSettings;

enum class LogMsgType { Error, Warning, Info };

class SystemLogEditor : public AbstractEditor
{

public:
    SystemLogEditor(StudioSettings *settings, QWidget *parent);
    void appendLog(const QString &msg, LogMsgType type = LogMsgType::Warning);

    // AbstractEditor interface
public:
    EditorType type() override;

};

}
}

#endif // SYSTEMLOGEDITOR_H
