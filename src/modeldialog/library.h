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
    Library(QString name, QString execName, int version, int nrColumns, QStringList columns, QStringList toolTips, QList<int> colOrder, QString glbFile);

    int version() const;
    QString name() const;
    int nrColumns() const;
    QStringList columns() const;
    QList<int> colOrder() const;
    QStringList toolTips() const;
    QString execName() const;
    void setName(const QString &name);
    QString glbFile() const;

private:
    QString mName;
    QString mExecName;
    int mVersion;
    int mNrColumns;
    QStringList mColumns;
    QStringList mtoolTips;
    QList<int> mColOrder;
    QString mGlbFile;
};

} // namespace studio
} // namespace gams

#endif // LIBRARY_H
