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
#include "library.h"

namespace gams {
namespace studio {
namespace modeldialog {

Library::Library(const QString& name, int version, int nrColumns, const QStringList &columns,int initSortCol,
                 const QStringList &toolTips, const QList<int> &colOrder, const QString &glbFile)
    : mName(name),
      mLongName(name),
      mVersion(version),
      mNrColumns(nrColumns),
      mInitSortCol(initSortCol),
      mColumns(columns),
      mtoolTips(toolTips),
      mColOrder(colOrder),
      mGlbFile(glbFile)
{
}

int Library::version() const
{
    return mVersion;
}

QString Library::name() const
{
    return mName;
}

int Library::nrColumns() const
{
    return mNrColumns;
}

QStringList Library::columns() const
{
    return mColumns;
}

QList<int> Library::colOrder() const
{
    return mColOrder;
}

QStringList Library::toolTips() const
{
    return mtoolTips;
}

void Library::setName(const QString &name)
{
    mName = name;
}

QString Library::glbFile() const
{
    return mGlbFile;
}

QString Library::longName() const
{
    return mLongName;
}

void Library::setLongName(const QString &longName)
{
    mLongName = longName;
}

int Library::initSortCol() const
{
    return mInitSortCol;
}

} // namespace modeldialog
} // namespace studio
} // namespace gams
