/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#ifndef FILEEVENT_H
#define FILEEVENT_H

#include <QEvent>
#include <QString>
#include <QTime>
#include "common.h"

namespace gams {
namespace studio {

class FileEvent;

struct FileEventData
{
    FileEventData();
    FileEventData(const FileId &_fileId, FileEventKind _kind);
    FileEventData(const FileEventData &other);
    FileEventData &operator= (const FileEventData &other);
    bool operator==(const FileEventData &other) const;
    bool operator!=(const FileEventData &other) const;
    FileId fileId;
    FileEventKind kind;
    QTime time;
};

uint qHash(const FileEventData &key);

class FileEvent : public QEvent
{
public:

    FileEvent(const FileId &fileId, FileEventKind kind);
    ~FileEvent();
    FileId fileId() const;
    FileEventKind kind() const;
    FileEventData data() const;
    static QEvent::Type type();

private:
    FileEventData mData;
    static QEvent::Type mType;

};

} // namespace studio
} // namespace gams

#endif // FILEEVENT_H
