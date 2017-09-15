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
#ifndef FILESYSTEMCONTEXT_H
#define FILESYSTEMCONTEXT_H

#include <QtCore>

namespace gams {
namespace ide {

class FileSystemContext : public QObject
{
    Q_OBJECT
public:
    virtual ~FileSystemContext();

    int id() const;
    bool isGist() const;
    virtual const QString name();
    void setName(const QString& name);

    const QString& location() const;
    bool matches(const QString& name, bool isGist) const;
    FileSystemContext* child(int index) const;
    FileSystemContext* parentEntry() const;
    int peekIndex(QString name, bool skipLast = false);
    bool active() const;

signals:
    void nameChanged(int id, QString newName);

protected:
    FileSystemContext(FileSystemContext* parent, int id, QString name, QString location, bool isGist);

protected:
    int mId;
    FileSystemContext* mParent;
    QString mName;
    QString mPath;
    bool mIsGist;
    bool mActive = false;

};

} // namespace ide
} // namespace gams

#endif // FILESYSTEMCONTEXT_H
