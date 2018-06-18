#ifndef FILEEVENT_H
#define FILEEVENT_H

#include <QEvent>
#include "common.h"

namespace gams {
namespace studio {


class FileEvent : public QEvent
{
public:

    enum class Kind {
        changed,
        closed,
        created,
        changedExtern,
        renamedExtern,
        removedExtern,  // removed-event is delayed a bit to improve recognition of moved- or changed-events
    };

    FileEvent(FileId fileId, FileEvent::Kind kind);
    FileId fileId() const;
    FileEvent::Kind kind() const;
    static QEvent::Type type();

private:
    FileId mFileId;
    FileEvent::Kind mKind;
    static QEvent::Type mType;

};

} // namespace studio
} // namespace gams

#endif // FILEEVENT_H
