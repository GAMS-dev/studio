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
#ifndef SCHEMALISTMODEL_H
#define SCHEMALISTMODEL_H

#include <QStandardItemModel>

namespace gams {
namespace studio {
namespace connect {

class SchemaListModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit SchemaListModel(const QStringList& schema, QObject *parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;

signals:
    void schemaItemChanged(const QString& schemaname) const;

private:
    QStringList mSchema;
};

} // namespace connect
} // namespace studio
} // namespace gams

#endif // SCHEMALISTMODEL_H
