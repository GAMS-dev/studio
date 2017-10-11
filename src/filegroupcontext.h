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
#ifndef FILEGROUPCONTEXT_H
#define FILEGROUPCONTEXT_H

#include "filesystemcontext.h"

namespace gams {
namespace studio {

class FileGroupContext : public FileSystemContext
{
    Q_OBJECT

public:
    ~FileGroupContext();
    void setFlag(ContextFlag flag, bool value = true);
    void unsetFlag(ContextFlag flag);

    void setLocation(const QString &location);

    int childCount();
    int indexOf(FileSystemContext *child);
    FileSystemContext* childEntry(int index);
    FileSystemContext* findFile(QString filePath);
    QIcon icon();

    bool isWatched();
    void setWatched(bool watch = true);
    QString runableGms();

signals:
    void contentChanged(int id, QDir fileInfo);

public slots:
    void directoryChanged(const QString &path);

protected:
    friend class FileRepository;
    friend class FileSystemContext;

    FileGroupContext(FileGroupContext *parent, int id, QString name, QString location, QString runInfo);
    int peekIndex(const QString &name, bool* hit = nullptr);
    void insertChild(FileSystemContext *child);
    void removeChild(FileSystemContext *child);
    void checkFlags();

private:
    QList<FileSystemContext*> mChildList;
    QFileSystemWatcher *mDirWatcher = nullptr;
    QString mRunInfo;
};

} // namespace studio
} // namespace gams

#endif // FILEGROUPCONTEXT_H
