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
#ifndef LIBRARY_H
#define LIBRARY_H

#include <QString>
#include <QStringList>
#include <QList>

namespace gams {
namespace studio {

class Library
{
public:
    Library(QString name, int version, int nrColumns, QStringList columns, QList<int> colOrder);

    int version() const;
    QString name() const;
    int nrColumns() const;
    QStringList columns() const;
    QList<int> colOrder() const;

private:
    int mVersion;
    QString mName;
    int mNrColumns;
    QStringList mColumns;
    QList<int> mColOrder;


};

} // namespace studio
} // namespace gams

#endif // LIBRARY_H
