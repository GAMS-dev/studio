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
#include "fileevent.h"

namespace gams {
namespace studio {

QEvent::Type FileEvent::mType = QEvent::None;

FileEvent::FileEvent(FileId fileId, Kind kind): QEvent(type()), mFileId(fileId), mKind(kind)
{

}

QEvent::Type FileEvent::type()
{
    if (mType == QEvent::None)
        mType = static_cast<QEvent::Type>(QEvent::registerEventType());
    return mType;

}

FileId FileEvent::fileId() const
{
    return mFileId;
}

FileEvent::Kind FileEvent::kind() const
{
    return mKind;
}

} // namespace studio
} // namespace gams
