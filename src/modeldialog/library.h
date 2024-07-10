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
#ifndef LIBRARY_H
#define LIBRARY_H

#include <QString>
#include <QStringList>
#include <QList>

namespace gams {
namespace studio {
namespace modeldialog {

class Library
{
public:
    Library(const QString& name, int version, int nrColumns, const QStringList &columns, int initSortCol,
            const QStringList &toolTips, const QList<int> &colOrder, const QString &glbFile);

    int version() const;
    QString name() const;
    int nrColumns() const;
    int initSortCol() const;
    QStringList columns() const;
    QList<int> colOrder() const;
    QStringList toolTips() const;
    void setName(const QString &name);
    QString glbFile() const;
    QString longName() const;
    void setLongName(const QString &longName);

private:
    QString mName;
    QString mLongName;
    int mVersion;
    int mNrColumns;
    int mInitSortCol;
    QStringList mColumns;
    QStringList mtoolTips;
    QList<int> mColOrder;
    QString mGlbFile;
};

} // namespace modeldialog
} // namespace studio
} // namespace gams

#endif // LIBRARY_H
