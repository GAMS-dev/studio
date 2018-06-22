#ifndef SYSTEMLOGEDIT_H
#define SYSTEMLOGEDIT_H

#include "abstractedit.h"

namespace gams {
namespace studio {

class StudioSettings;

enum class LogMsgType { Error, Warning, Info };

class SystemLogEdit : public AbstractEdit
{
public:
    SystemLogEdit(StudioSettings *settings, QWidget *parent);
    void appendLog(const QString &msg, LogMsgType type = LogMsgType::Warning);

public:
    EditorType type() override;
};

}
}

#endif // SYSTEMLOGEDIT_H
