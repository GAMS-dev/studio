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
#ifndef GAMS_STUDIO_CONNECT_SCHEMADEFINITIONMODEL_H
#define GAMS_STUDIO_CONNECT_SCHEMADEFINITIONMODEL_H

#include <QAbstractItemModel>
#include "connect.h"
#include "schemadefinitionitem.h"

namespace gams {
namespace studio {
namespace connect {

enum class SchemaItemColumn {
    Field        = 0,
    Required     = 1,
    Type         = 2,
    Nullable     = 3,
    Default      = 4,
    AllowedValue = 5,
    min          = 6,
    SchemaKey    = 7,
    DragEnabled  = 8,
    Excludes     = 9
};

class SchemaDefinitionModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit SchemaDefinitionModel(Connect* connect, const QString& schemaName, QObject *parent = nullptr);
    ~SchemaDefinitionModel() override;

    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList & indexes) const override;

public slots:
    void loadSchemaFromName(const QString& name);

private:
    void addTypeList(QList<SchemaType>& typeList, QList<QVariant>& data);
    void addValueList(QList<ValueWrapper>& valueList, QList<QVariant>& data);
    void addValue(ValueWrapper& value, QList<QVariant>& data);
    QString getValue(ValueWrapper& value);
    QStringList gettAllAllowedValues(Schema* schemaHelper);

    void setupOneofAnyofSchemaTree(const QString& schemaName, const QString& key,
                              QStringList& schemaKeys, QList<SchemaDefinitionItem*>& parents, ConnectSchema* schema);
    void setupSchemaTree(const QString& schemaName, const QString& key,
                         QStringList& schemaKeys, QList<SchemaDefinitionItem*>& parents, ConnectSchema* schema);

protected:
    void setupTreeItemModelData();

    QString   mCurrentSchemaName;
    Connect*  mConnect;
    QMap<QString, SchemaDefinitionItem*> mRootItems;
    static QRegularExpression mRexDigits;
};

} // namespace connect
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_CONNECT_SCHEMADEFINITIONMODEL_H
