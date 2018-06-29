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
#ifndef FILEMETAREPO_H
#define FILEMETAREPO_H

#include <QObject>
#include <QFileSystemWatcher>
#include "filemeta.h"
#include "fileevent.h"
#include "common.h"

namespace gams {
namespace studio {

class TextMarkRepo;
class ProjectRepo;

class FileMetaRepo : public QObject
{
    Q_OBJECT
public:
    FileMetaRepo(QObject* parent, StudioSettings* settings);
    FileMeta* fileMeta(const FileId &fileId) const;
    FileMeta* fileMeta(const QString &location) const;
    FileMeta* fileMeta(QWidget * const &editor) const;
    FileMeta *findOrCreateFileMeta(QString location);
    StudioSettings *settings() const;
    void init(TextMarkRepo* textMarkRepo, ProjectRepo *projectRepo);
    TextMarkRepo *textMarkRepo() const;
    ProjectRepo *projectRepo() const;
    QVector<FileMeta*> openFiles() const;
    QVector<FileMeta*> modifiedFiles() const;
    void unwatch(const QString &path);

signals:
    void fileEvent(FileEvent &e);

public slots:
    void openFile(FileMeta* fm, FileId runId, bool focus = true, int codecMib = -1);
    void removedFile(FileMeta* fileMeta);

private slots:
//    void dirChanged(const QString& path);
    void fileChanged(const QString& path);
    void reviewMissing();

private:
    void addFileMeta(FileMeta* fileMeta);

private:
    FileId mNextFileId = 0;
    StudioSettings *mSettings = nullptr;
    TextMarkRepo* mTextMarkRepo = nullptr;
    ProjectRepo* mProjectRepo = nullptr;
    QHash<FileId, FileMeta*> mFiles;
    QFileSystemWatcher mWatcher;
    QStringList mCheckExistance; // List to be checked once
    QStringList mMissList; // List to be checked periodically

};

} // namespace studio
} // namespace gams

#endif // FILEMETAREPO_H
