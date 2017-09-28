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
#ifndef FILECONTEXT_H
#define FILECONTEXT_H

#include "filesystemcontext.h"

namespace gams {
namespace ide {

// TODO(JM) define extra type class that gathers all type info (enum, suffix, description, icon, ...)
enum class FileType {
    ftGpr,
    ftGms,
    ftTxt,
    ftInc,
    ftLog,
    ftLst,
    ftLxi,
};

enum class CrudState {
    eCreate,
    eRead,
    eUpdate,
    eDelete
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
    /// signal is emitted if the current CRUD-state has changed.
    /// \param state The new CRUD-state
    void crudChanged(CrudState state);

public slots:
    /// Slot to handle a change of the assigned Document
    /// TODO (JM) bind to signal QTextDocument::modificationChanged instead
    void textChanged();

protected:
    friend class FileRepository;
    FileContext(FileGroupContext *parent, int id, QString name, QString location);
    void setCrudState(CrudState state);

private:
    CrudState mCrudState = CrudState::eCreate;
    QString mCodec = "UTF-8";
    QTextDocument* mDocument = nullptr;
    QFileSystemWatcher *mWatcher = nullptr;
    static const QStringList mDefaulsCodecs;

};

} // namespace ide
} // namespace gams

#endif // FILECONTEXT_H
