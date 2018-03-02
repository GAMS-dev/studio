/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "fileactioncontext.h"
#include <QtWidgets>
#include "exception.h"

namespace gams {
namespace studio {

FileActionContext::FileActionContext(int id, QAction *action)
    : FileSystemContext(id, action->text(), action->toolTip(), FileSystemContext::FileAction), mAction(action)
{
    setFlag(cfVirtual);
}

void FileActionContext::trigger()
{
    emit mAction->trigger();
}

void FileActionContext::setLocation(const QString& location)
{
    Q_UNUSED(location);
    EXCEPT() << "The location of a FileActionContext can't be changed.";
}

QIcon FileActionContext::icon()
{
    return mAction->icon();
}



} // namespace studio
} // namespace gams
