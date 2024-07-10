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
#ifndef NAVIGATORMODEL_H
#define NAVIGATORMODEL_H

#include <QAbstractTableModel>
#include "navigator/navigatorcontent.h"

namespace gams {
namespace studio {

class NavigatorModel : public QAbstractTableModel
{
public:
    explicit NavigatorModel(QObject *parent);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    void setContent(const QSet<NavigatorContent> &content);
    QVector<NavigatorContent> content() const;
    QDir currentDir() const;
    void setCurrentDir(const QDir &dir);
    int findIndex(const QString &file);

private:
    QVector<NavigatorContent> mContent;
    QDir mCurrentDir;
};

}
}
#endif // NAVIGATORMODEL_H
