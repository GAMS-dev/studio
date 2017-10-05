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
#ifndef FILECONTEXT_H
#define FILECONTEXT_H

#include "filesystemcontext.h"

namespace gams {
namespace studio {

// TODO(JM) define extra type class that gathers all type info (enum, suffix, description, icon, ...)
enum class FileType {
    ftGsp,      ///< GAMS Studio Project file
    ftGms,      ///< GAMS source file
    ftInc,      ///< GAMS include file
    ftTxt,      ///< Plain text file
    ftLog,      ///< LOG output file
    ftLst,      ///< GAMS result file
    ftLxi,      ///< GAMS result file index
};

enum class CrudState {
    eCreate,
    eRead,
    eUpdate,
    eDelete
};

///
/// The FileMetrics class stores current metrics of a file
///
class FileMetrics
{
    bool mExists;
    qint64 mSize;
    QDateTime mCreated;
    QDateTime mModified;
public:
    enum ChangeKind {ckSkip, ckUnchanged, /* ckRenamed, */ ckNotFound, ckModified};
    FileMetrics(): mExists(false), mSize(0) {
    }
    FileMetrics(QFileInfo fileInfo) {
        mExists = fileInfo.exists();
        mSize = mExists ? fileInfo.size() : 0;
        mCreated = mExists ? fileInfo.created() : QDateTime();
        mModified = mExists ? fileInfo.lastModified() : QDateTime();
    }
    ChangeKind check(QFileInfo fileInfo) {
        if (mModified.isNull()) return ckSkip;
        if (!fileInfo.exists()) {
            // TODO(JM) #106: find a file in the path fitting created, modified and size values
            return ckNotFound;
        }
        if (fileInfo.lastModified() != mModified) return ckModified;
        return ckUnchanged;
    }
};

class FileGroupContext;

class FileContext : public FileSystemContext
{
    Q_OBJECT
public:

    /// The name of the current codec for this file.
    /// \return The name of the codec.
    QString codec() const;

    /// Changes the codec for this file.
    /// \param codec The name of the new codec.
    void setCodec(const QString& codec);

    /// The caption of this file, which is its extended display name.
    /// \return The caption of this node.
    virtual const QString caption();

    /// The CRUD-state of the node (Create,Read,Update,Delete).
    /// \return The CRUD-state of the node.
    CrudState crudState() const;

    /// Sets a new location (name and path) to the node. This sets the CRUD-state to "Create"
    /// \param location The new location
    void setLocation(const QString &location);

    /// The icon for this file type.
    /// \return The icon for this file type.
    QIcon icon();

    /// Sets a flag to the current file-context.
    /// \param flag The ContextFlag
    virtual void setFlag(ContextFlag flag);

    /// Unsets a flag in the current file-context.
    /// \param flag The ContextFlag
    virtual void unsetFlag(ContextFlag flag);

    /// Saves the file, if it is changed.
    void save();

    /// Loads the file into the current QTextDocument.
    /// \param codecName The text-codec to use.
    void load(QString codecName = QString());

    /// Assigns a QTextDocument to this file used in the editor. The document can only be assigned once.
    /// If the file already has a document, it can be used to assign it to a second editor.
    /// \param doc The new QTextDocument
    void setDocument(QTextDocument *doc);

    /// The current QTextDocument assigned to this file.
    /// \return The current QTextDocument
    QTextDocument* document();

signals:
    /// Signal is emitted when the file has been modified externally.
    /// \param fileId The file identifier
    void modifiedExtern(int fileId);

    /// Signal is emitted when the file has been deleted (or renamed) externally.
    /// \param fileId The file identifier
    void deletedExtern(int fileId);

public slots:
    /// Slot to handle a change of the assigned Document
    void modificationChanged(bool modiState);

protected slots:
    void onFileChangedExtern(QString filepath);

protected:
    friend class FileRepository;
    FileContext(FileGroupContext *parent, int id, QString name, QString location);
    void setCrudState(CrudState state);

private:
    CrudState mCrudState = CrudState::eCreate;
    FileMetrics mMetrics;
    QString mCodec = "UTF-8";
    QTextDocument* mDocument = nullptr;
    QFileSystemWatcher *mWatcher = nullptr;
    static const QStringList mDefaulsCodecs;
};

} // namespace studio
} // namespace gams

#endif // FILECONTEXT_H
