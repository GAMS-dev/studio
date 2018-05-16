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
#ifndef FILETYPE_H
#define FILETYPE_H

#include <QtCore>

namespace gams {
namespace studio {

///
/// The FileType class defines all kind of file types and additional data and description. The instances are accessed
/// via static functions. On the first usage, the list is initialized.
///
class FileType
{
public:
    enum Kind {
        None,
        Gsp,
        Gms,
        Txt,
        Lst,
        Lxi,
        Log,
        Gdx,
        Ref,
    };

public:
    Kind kind() const;
    QStringList suffix() const;
    QString description() const;
    bool autoReload() const;
    Kind dependant() const;

    bool operator ==(const FileType& fileType) const;
    bool operator !=(const FileType& fileType) const;
    bool operator ==(const FileType::Kind& kind) const;
    bool operator !=(const FileType::Kind& kind) const;

    static const QList<FileType*> list();
    static FileType& from(QString suffix);
    static FileType& from(Kind kind);


private:
    friend class FileRepository;

    static void clear();
    FileType(Kind kind, QString suffix, QString description, bool autoReload, const Kind dependant = None);

    const Kind mKind;
    const QStringList mSuffix;
    const QString mDescription;
    const bool mAutoReload;
    const Kind mDependant;

    static QList<FileType*> mList;
    static FileType* mNone;
};

} // namespace studio
} // namespace gams

#endif // FILETYPE_H
