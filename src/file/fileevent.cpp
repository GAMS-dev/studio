/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "logger.h"

namespace gams {
namespace studio {

QEvent::Type FileEvent::mType = QEvent::None;

FileEvent::FileEvent(const FileId &_fileId, FileEventKind _kind)
    : QEvent(type()), mData(FileEventData(_fileId, _kind))
{}

FileEvent::~FileEvent()
{}

QEvent::Type FileEvent::type()
{
    if (mType == QEvent::None)
        mType = static_cast<QEvent::Type>(QEvent::registerEventType());
    return mType;
}

FileEventData FileEvent::data() const
{
    return mData;
}

FileId FileEvent::fileId() const
{
    return mData.fileId;
}

FileEventKind FileEvent::kind() const
{
    return mData.kind;
}

FileEventData::FileEventData()
    : fileId(FileId()), kind(FileEventKind::invalid), time(QTime().currentTime())
{}

FileEventData::FileEventData(const FileId &_fileId, FileEventKind _kind)
    : fileId(_fileId), kind(_kind), time(QTime().currentTime())
{}

FileEventData::FileEventData(const FileEventData &other)
{
    *this = other;
}

FileEventData &FileEventData::operator=(const FileEventData &other)
{
    fileId = other.fileId;
    kind = other.kind;
    time = other.time;
    return *this;
}

bool FileEventData::operator==(const FileEventData &other) const
{
    return (fileId == other.fileId) && (kind == other.kind);
}

bool FileEventData::operator!=(const FileEventData &other) const
{
    return !(*this == other);
}

uint qHash(const FileEventData &key) {
    return (key.fileId << 3) + int(key.kind);
}

} // namespace studio
} // namespace gams
