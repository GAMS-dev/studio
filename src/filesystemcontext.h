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
{
    Q_OBJECT
public:
    enum ContextFlag {
        cfNone          = 0x00,
        cfActive        = 0x01,
        cfFileMod       = 0x02,
        cfEditMod       = 0x04,
        cfMissing       = 0x08,
        cfExtendCaption = 0x10,
    };

    enum ContextType {
        File,
        FileAction,
        FileGroup,
        FileSystem
    };

    typedef QFlags<ContextFlag> ContextFlags;

    virtual ~FileSystemContext();

    int id() const;
    int type() const;

    virtual const QString caption();
    virtual const QString name();
    void setName(const QString& name);
    const QString& location() const;
    virtual void setLocation(const QString& location);
    virtual QIcon icon();

    const ContextFlags &flags() const;
    virtual void setFlag(ContextFlag flag, bool value = true);
    virtual void unsetFlag(ContextFlag flag);
    virtual bool testFlag(ContextFlag flag);

    FileGroupContext* parentEntry() const;
    void setParentEntry(FileGroupContext *parent);
    virtual FileSystemContext* childEntry(int index);
    virtual int childCount();

signals:
    void changed(int fileId);

protected:
    FileSystemContext(FileGroupContext* parent, int id, QString name, QString location);
    FileSystemContext(FileGroupContext* parent, int id, QString name, QString location, ContextType type);
    virtual void checkFlags();

protected:
    int mId;
    FileGroupContext* mParent;
    QString mName;
    QString mLocation;
    ContextFlags mFlags;

private:
    ContextType mType;
};

} // namespace studio
} // namespace gams

#endif // FILESYSTEMCONTEXT_H
