/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#ifndef FILEMETRICS_H
#define FILEMETRICS_H

#include <QtCore>
#include "filetype.h"

namespace gams {
namespace studio {

///
/// The FileMetrics class stores current metrics of a file
///
class FileMetrics
{
    bool mExists;
    qint64 mSize;
    QDateTime mCreated;
    QDateTime mModified;
    FileType *mType;

public:
    enum ChangeKind {ckSkip, ckUnchanged, /* ckRenamed, */ ckNotFound, ckModified};

    FileMetrics();
    explicit FileMetrics(QFileInfo fileInfo);
    FileMetrics(const FileMetrics &other);
    FileMetrics &operator=(const FileMetrics& other);

    const FileType& fileType() const;

    ChangeKind check(QFileInfo fileInfo);

};

} // namespace studio
} // namespace gams

#endif // FILEMETRICS_H
