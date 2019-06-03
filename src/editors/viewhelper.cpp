#include "viewhelper.h"
#include "abstractedit.h"
#include "file/filemeta.h"

#include <QVariant>

namespace gams {
namespace studio {

ViewHelper::ViewHelper()
{ }

FileId ViewHelper::fileId(QWidget *widget)
{
    bool ok;
    FileId res = widget->property("fileId").toInt(&ok);
    return ok ? res : FileId();
}

void ViewHelper::setFileId(QWidget *widget, FileId id)
{
    widget->setProperty("fileId", id.isValid() ? QVariant(id) : -1);
    if (AbstractEdit *ed = toAbstractEdit(widget)) {
        // if there is an inner edit: set the property additionally
        if (ed != widget)
            ed->setProperty("fileId", id.isValid() ? QVariant(id) : -1);
    }
    if (TextView *tv = toTextView(widget)) {
        // if there is an inner edit: set the property additionally
        tv->edit()->setProperty("fileId", id.isValid() ? QVariant(id) : -1);
    }
}

NodeId ViewHelper::groupId(QWidget *widget)
{
    bool ok;
    NodeId res = widget->property("groupId").toInt(&ok);
    return ok ? res : NodeId();
}

void ViewHelper::setGroupId(QWidget *widget, NodeId id)
{
    widget->setProperty("groupId", id.isValid() ? QVariant(id) : -1);
    if (AbstractEdit *ed = toAbstractEdit(widget)) {
        // if there is an inner edit: set the property additionally
        if (ed != widget)
            ed->setProperty("groupId", id.isValid() ? QVariant(id) : -1);
    }
    if (TextView *tv = toTextView(widget)) {
        // if there is an inner edit: set the property additionally
        tv->edit()->setProperty("groupId", id.isValid() ? QVariant(id) : -1);
    }
}

QString ViewHelper::location(QWidget *widget)
{
    return widget->property("location").toString();
}

void ViewHelper::setLocation(QWidget *widget, QString location)
{
    widget->setProperty("location", location);
    // if there is an inner edit: set the property additionally
    if (AbstractEdit *ed = toAbstractEdit(widget)) {
        if (ed != widget) ed->setProperty("location", location);
    } else if (TextView* tv = toTextView(widget)) {
        if (tv != widget) tv->edit()->setProperty("location", location);
    }
}

} // namespace studio
} // namespace gams
