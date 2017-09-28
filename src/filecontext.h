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
    QString codec() const;
    void setCodec(const QString& codec);
    virtual const QString caption();
    CrudState crudState() const;
    void setLocation(const QString &location); // equals save_as...
    QIcon icon();

    virtual void setFlag(ContextFlag flag);
    virtual void unsetFlag(ContextFlag flag);

    void save();
    void load(QString codecName = QString());
    void setDocument(QTextDocument *doc);
    QTextDocument* document();

signals:
    void crudChanged(CrudState state);

public slots:
    void textChanged();
//    void storageChanged();

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
