/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
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
