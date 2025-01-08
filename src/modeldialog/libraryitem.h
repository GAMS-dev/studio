/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#ifndef LIBRARYITEM_H
#define LIBRARYITEM_H

#include "library.h"
#include <QSharedPointer>

namespace gams {
namespace studio {
namespace modeldialog {

class LibraryItem
{
public:
    LibraryItem(const QSharedPointer<Library> &library,
                const QStringList &values,
                const QString &description,
                const QString &longDescription,
                const QStringList &files,
                int suffixNumber);

    QSharedPointer<Library> library() const;
    QStringList values() const;
    QString name() const;
    QStringList files() const;
    QString longDescription() const;
    QString nameWithSuffix() const;

private:
    QSharedPointer<Library> mLibrary;
    QString mDescription;
    QString mLongDescription;
    QStringList mFiles;
    QStringList mValues;
    int mSuffixNumber;

};

} // namespace modeldialog
} // namespace sutdio
} // namespace gams

#endif // LIBRARYITEM_H
