#ifndef SYSTEMLOGEDITOR_H
#define SYSTEMLOGEDITOR_H

#include "abstractedit.h"

namespace gams {
namespace studio {

class StudioSettings;

enum class LogMsgType { Error, Warning, Info };

class SystemLogEditor : public AbstractEdit
{
public:
    SystemLogEditor(StudioSettings *settings, QWidget *parent);
    void appendLog(const QString &msg, LogMsgType type = LogMsgType::Warning);

public:
    EditorType type() override;

};

}
}

#endif // SYSTEMLOGEDITOR_H
