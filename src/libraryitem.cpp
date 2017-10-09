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
#include "libraryitem.h"

namespace gams {
namespace studio {

LibraryItem::LibraryItem(std::shared_ptr<Library> library, QStringList values, QString description, QStringList files):
    mLibrary(library), mValues(values), mDescription(description), mFiles(files)
{
}

std::shared_ptr<Library> LibraryItem::library() const
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
    return mValues.at(idx);
}


} // namespace sutdio
} // namespace gams
