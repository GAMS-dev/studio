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
#ifndef FILEGROUPCONTEXT_H
#define FILEGROUPCONTEXT_H

#include "filesystemcontext.h"
#include "syntax.h"

#include <memory>

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
    virtual ~FileGroupContext() override;
    void setFlag(ContextFlag flag, bool value = true) override;
    void unsetFlag(ContextFlag flag) override;

    void setLocation(const QString &location) override;

    int childCount() const override;
    int indexOf(FileSystemContext *child);
    FileSystemContext* childEntry(int index) const override;
    FileSystemContext* findContext(QString filePath);
    FileContext* findFile(QString filePath);
    QIcon icon() override;

    QString runnableGms();
    void setRunnableGms(FileContext *gmsFileContext);
    QString lstFileName();
    void setLstFileName(const QString &lstFileName);
    LogContext* logContext() const;

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
    void saveGroup();

    void dumpMarks();
    QString tooltip() override;

signals:
    void gamsProcessStateChanged(FileGroupContext* group);
    void removeNode(FileSystemContext *node);
    void requestNode(QString name, QString location, FileGroupContext* parent = nullptr);
    void findOrCreateFileContext(QString filePath, FileContext *&resultFile, FileGroupContext* fileGroup = nullptr);

protected slots:
    void onGamsProcessStateChanged(QProcess::ProcessState newState);

protected:
    friend class FileRepository;
    friend class FileSystemContext;
    friend class FileContext;
    friend class LogContext;

    FileGroupContext(FileId id, QString name, QString location, QString runInfo);
    int peekIndex(const QString &name, bool* hit = nullptr);
    void insertChild(FileSystemContext *child);
    void removeChild(FileSystemContext *child);
    void checkFlags() override;
    void setLogContext(LogContext* logContext);
    void updateRunState(const QProcess::ProcessState &state);
    void addMark(const QString &filePath, TextMark* mark);
    TextMarkList* marks(const QString &fileName);
    void removeMarks(QSet<TextMark::Type> tmTypes = QSet<TextMark::Type>());
    void removeMarks(QString fileName, QSet<TextMark::Type> tmTypes = QSet<TextMark::Type>());

private:
    QList<FileSystemContext*> mChildList;
    LogContext* mLogContext = nullptr;
    std::unique_ptr<GamsProcess> mGamsProcess;
    QString mLstFileName;
    QString mGmsFileName;
    QFileInfoList mAttachedFiles;

    QHash<int, QString> mLstErrorTexts;
    QHash<QString, TextMarkList*> mMarksForFilenames;

};

} // namespace studio
} // namespace gams

#endif // FILEGROUPCONTEXT_H
