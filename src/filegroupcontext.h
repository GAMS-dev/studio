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
#include "syntax.h"

namespace gams {
namespace studio {

class LogContext;
class FileContext;
class GamsProcess;
//class TextMarkList;
//class TextMark;
//enum TextMark::Type;

class FileGroupContext : public FileSystemContext
{
    Q_OBJECT

public:
    virtual ~FileGroupContext();
    void setFlag(ContextFlag flag, bool value = true);
    void unsetFlag(ContextFlag flag);

    void setLocation(const QString &location);

    int childCount() const;
    int indexOf(FileSystemContext *child);
    FileSystemContext* childEntry(int index) const;
    FileSystemContext* findContext(QString filePath);
    FileContext* findFile(QString filePath);
    QIcon icon();

    QString runableGms();
    QString lstFileName();
    LogContext* logContext();

    GamsProcess* newGamsProcess();
    GamsProcess* gamsProcess();
    QProcess::ProcessState gamsProcessState() const;

    void attachFile(const QString &filepath);
    void detachFile(const QString &filepath);
    void updateChildNodes();
    void jumpToFirstError(bool focus);

    QString lstErrorText(int line);
    void setLstErrorText(int line, QString text);
    void clearLstErrorTexts();
    bool hasLstErrorText( int line = -1);

    void dumpMarks();

signals:
    void gamsProcessStateChanged(FileGroupContext* group);
    void removeNode(FileSystemContext *node);
    void requestNode(QString name, QString location, FileGroupContext* parent = nullptr);

protected slots:
    void onGamsProcessStateChanged(QProcess::ProcessState newState);
    void processDeleted();

protected:
    friend class FileRepository;
    friend class FileSystemContext;
    friend class FileContext;
    friend class LogContext;

    FileGroupContext(FileId id, QString name, QString location, QString runInfo);
    int peekIndex(const QString &name, bool* hit = nullptr);
    void insertChild(FileSystemContext *child);
    void removeChild(FileSystemContext *child);
    void checkFlags();
    void setLogContext(LogContext* logContext);
    void updateRunState(const QProcess::ProcessState &state);
    void addMark(const QString &filePath, TextMark* mark);
    TextMarkList* marks(const QString &fileName);
    void removeMarks(QSet<TextMark::Type> tmTypes = QSet<TextMark::Type>());
    void removeMarks(QString fileName, QSet<TextMark::Type> tmTypes = QSet<TextMark::Type>());

private:
    QList<FileSystemContext*> mChildList;
    QFileSystemWatcher *mDirWatcher = nullptr;
    QString mRunInfo;
    LogContext* mLogContext = nullptr;
    GamsProcess* mGamsProcess = nullptr;
    QString mLstFileName;
    QFileInfoList mAttachedFiles;
    QHash<int, QString> mLstErrorTexts;
    QHash<QString, TextMarkList*> mMarksForFilenames;
};

} // namespace studio
} // namespace gams

#endif // FILEGROUPCONTEXT_H
