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
#ifndef FILEMETAREPO_H
#define FILEMETAREPO_H

#include <QObject>
#include <QFileSystemWatcher>
#include "filemeta.h"
#include "fileevent.h"
#include "common.h"
#include "editors/codecompleter.h"

namespace gams {
namespace studio {

class TextMarkRepo;
class ProjectRepo;

class FileMetaRepo : public QObject
{
    Q_OBJECT
public:
    FileMetaRepo(QObject* parent);
    FileMeta* fileMeta(const FileId &fileId) const;
    FileMeta* fileMeta(const QString &location) const;
    FileMeta* fileMeta(QWidget * const &editor) const;
    const QList<FileMeta*> fileMetas() const;
    FileMeta* findOrCreateFileMeta(const QString &location, FileType *knownType = nullptr);
    void init(TextMarkRepo* textMarkRepo, ProjectRepo *projectRepo);
    TextMarkRepo *textMarkRepo() const;
    ProjectRepo *projectRepo() const;
    QVector<FileMeta*> openFiles() const;
    QVector<FileMeta*> modifiedFiles() const;
    QWidgetList editors() const;
    void unwatch(const FileMeta* fm);
    void unwatch(const QString &filePath);
    bool watch(const FileMeta* fm);
    void setAutoReload(const QString &location);
    void getIcon(QIcon &icon, FileMeta *file, const NodeId &projectId);

    void setDebugMode(bool debug);
    bool debugMode() const;
    static bool equals(const QFileInfo &fi1, const QFileInfo &fi2);
    void updateRenamed(FileMeta *file, const QString &oldLocation);
    void setUserGamsTypes(const QStringList &suffix);
    CodeCompleter *completer() { return &mCompleter; }

    bool askBigFileEdit() const;
    void setAskBigFileEdit(bool askBigFileEdit);

signals:
    void fileEvent(gams::studio::FileEvent &e);
    void editableFileSizeCheck(const QFile &file, bool &canOpen);
    void setGroupFontSize(gams::studio::FontGroup fontGroup, qreal fontSize, QString fontFamily = QString());
    void scrollSynchronize(QWidget *sendingEdit, int dx, int dy);
    void saveProjects();

public slots:
    void openFile(gams::studio::FileMeta* fm, const gams::studio::NodeId &groupId, bool focus = true, int codecMib = -1);
    void removeFile(gams::studio::FileMeta* fileMeta);
    void toggleBookmark(const gams::studio::FileId &fileId, int lineNr, int posInLine);
    void jumpToNextBookmark(bool back, const gams::studio::FileId &refFileId, int refLineNr);

private slots:
    void fileChanged(const QString& path);
    void reviewRemoved();
    void checkMissing();
    void fontChangeRequest(gams::studio::FileMeta *fileMeta, const QFont &f);

private:
    void addFileMeta(FileMeta* fileMeta);

private:
    FileId mNextFileId = 0;
    Settings *mSettings = nullptr;
    TextMarkRepo* mTextMarkRepo = nullptr;
    ProjectRepo* mProjectRepo = nullptr;
    QHash<FileId, FileMeta*> mFiles;
    QHash<QString, FileMeta*> mFileNames;
    QStringList mAutoReloadLater;
    QFileSystemWatcher mWatcher;
    QStringList mRemoved; // List to be checked once
    QStringList mMissList; // List to be checked periodically
    QTimer mMissCheckTimer;
    CodeCompleter mCompleter;
    bool mAskBigFileEdit = true;
    bool mDebug = false;

};

} // namespace studio
} // namespace gams

#endif // FILEMETAREPO_H
