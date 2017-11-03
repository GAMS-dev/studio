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
#ifndef FILESYSTEMCONTEXT_H
#define FILESYSTEMCONTEXT_H

#include <QtGui>

namespace gams {
namespace studio {

class FileGroupContext;

class FileSystemContext : public QObject
{ // TODO(AF) Make this thing abstract and use is as a interface for all common functions?
    // TODO(AF) Rename this to AbstractFileContext?
    Q_OBJECT

public:
    enum ContextFlag { // TODO(AF) for global methods (e.g. save all) add changed state?
        cfNone          = 0x00,
        cfActive        = 0x01,
        cfFileMod       = 0x02,
        cfEditMod       = 0x04,
        cfMissing       = 0x08,
        cfExtendCaption = 0x10,
        cfVirtual       = 0x20,
    };

    enum ContextType {
        File,
        FileAction, // TODO(AF) still required?
        FileGroup,
        FileSystem
    };

    typedef QFlags<ContextFlag> ContextFlags;

    virtual ~FileSystemContext();

    int id() const;

    /// \brief File context type.
    /// \return Returns the file context type as <c>int</c>.
    int type() const;

    /// The caption of this file, which is its extended display name.
    /// \return The caption of this node.
    virtual const QString caption();

    /// The raw name of this node.
    /// \return The raw name of this node.
    virtual const QString name();

    /// Sets the raw name of this node.
    /// \param name The raw name of this node.
    void setName(const QString& name);

    /// The location of the node. This is a directory or file with full path.
    /// \param location The new location
    const QString& location() const;

    /// Sets a new location (name and path) to the node. This sets the CRUD-state to "Create"
    /// \param location The new location
    virtual void setLocation(const QString& location);

    /// The icon for this file type.
    /// \return The icon for this file type.
    virtual QIcon icon() = 0;

    const ContextFlags &flags() const;
    virtual void setFlag(ContextFlag flag, bool value = true);
    virtual void unsetFlag(ContextFlag flag);
    virtual bool testFlag(ContextFlag flag);

    FileGroupContext* parentEntry() const;
    void setParentEntry(FileGroupContext *parent);
    virtual FileSystemContext* childEntry(int index);
    virtual int childCount();

    FileSystemContext *findFile(QString filePath);
signals:
    void changed(int fileId);

protected:
    FileSystemContext(int id, QString name, QString location);
    FileSystemContext(int id, QString name, QString location, ContextType type);
    virtual void checkFlags();

private:
    int mId;
    FileGroupContext* mParent;
    QString mName;
    QString mLocation;
    ContextFlags mFlags;
    ContextType mType;
};

} // namespace studio
} // namespace gams

#endif // FILESYSTEMCONTEXT_H
