/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "libraryitem.h"

namespace gams {
namespace studio {
namespace modeldialog {

LibraryItem::LibraryItem(const QSharedPointer<Library> &library,
                         const QStringList &values,
                         const QString &description,
                         const QString &longDescription,
                         const QStringList &files,
                         int suffixNumber)
    : mLibrary(library)
    , mDescription(description)
    , mLongDescription(longDescription)
    , mFiles(files)
    , mValues(values)
    , mSuffixNumber(suffixNumber)
{
}

QSharedPointer<Library> LibraryItem::library() const
{
    return mLibrary;
}

QStringList LibraryItem::values() const
{
    return mValues;
}

QString LibraryItem::name() const
{
    int idx = mLibrary->columns().indexOf("Name");
    return mValues.at(mLibrary->colOrder().at(idx));
}

QStringList LibraryItem::files() const
{
    return mFiles;
}

QString LibraryItem::longDescription() const
{
    return mLongDescription;
}

QString LibraryItem::nameWithSuffix() const
{
    QString name = this->name();
    if (mSuffixNumber>0)
        name.append("_" + QString::number(mSuffixNumber));
    return name;
}

} // namespace modeldialog
} // namespace studio
} // namespace gams
