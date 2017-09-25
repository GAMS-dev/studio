/*
 * This file is part of the GAMS IDE project.
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
#ifndef FILEGROUPCONTEXT_H
#define FILEGROUPCONTEXT_H

#include "filesystemcontext.h"

namespace gams {
namespace ide {

class FileGroupContext : public FileSystemContext
{
    Q_OBJECT
public:
    ~FileGroupContext();
    void setFlag(ContextFlag flag, bool value = true);
    void unsetFlag(ContextFlag flag);

    int childCount();
    int indexOf(FileSystemContext *child);
    FileSystemContext* childEntry(int index);

signals:
    void contentChanged(int id, QDir fileInfo);

public slots:
    void directoryChanged(const QString &path);

protected:
    friend class FileRepository;
    friend class FileSystemContext;

    FileGroupContext(FileGroupContext *parent, int id, QString name, QString location, bool isGist);
    int peekIndex(const QString &name, bool* exactMatch = nullptr);
    void insertChild(FileSystemContext *child);
    void insertChild(int pos, FileSystemContext *child);
    void removeChild(FileSystemContext *child);
    void checkFlags();

private:
    QList<FileSystemContext*> mChildList;
};

} // namespace ide
} // namespace gams

#endif // FILEGROUPCONTEXT_H
