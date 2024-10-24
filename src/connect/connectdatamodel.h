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
#ifndef GAMS_STUDIO_CONNECT_CONNECTDATAMODEL_H
#define GAMS_STUDIO_CONNECT_CONNECTDATAMODEL_H

#include <QAbstractItemModel>

#include "connect.h"
#include "connectdata.h"
#include "connectdataitem.h"

namespace gams {
namespace studio {
namespace connect {

enum class DataCheckState {
    Root         = 0,
    SchemaName   = 1,
    ListItem     = 2,
    KeyItem      = 3,
    ElementKey   = 4,
    ElementValue = 5,
    ElementMap   = 6,
    ListAppend   = 7,
    MapAppend    = 8,
    SchemaAppend = 9,
};

enum class DataItemColumn {
    Key          = 0,
    Value        = 1,
    CheckState   = 2,
    SchemaType   = 3,
    AllowedValue = 4,
    Delete       = 5,
    MoveDown     = 6,
    MoveUp       = 7,
    ElementID    = 8,
    SchemaKey    = 9,
    Undefined    = 10,
    InvalidValue = 11,
    ExcludedKeys = 12,
    DefaultValue = 13
};

class ConnectDataModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ConnectDataModel(const QString& filename, Connect* c, QObject *parent = nullptr);
    ~ConnectDataModel() override;

    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex indexForTreeItem(ConnectDataItem* item);

    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    ConnectDataItem* getItem(const QModelIndex &index) const;
    void insertItem(int position, ConnectDataItem* item, const QModelIndex &parent);
    bool removeItem(const QModelIndex &index);

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                              const QModelIndex &destinationParent, int destinationChild) override;

    QStringList mimeTypes() const override;

    Qt::DropActions supportedDropActions() const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData * mimedata, Qt::DropAction action, int row, int column, const QModelIndex & parent) override;

    ConnectData* getConnectData();

signals:
    void fromSchemaInserted(const QString& schemaname, int insertPosition);
    void indexExpandedAndResized(const QModelIndex &index);

    void modificationChanged(bool modifiedState);

public slots:
    void addFromSchema(const QString& schemaname, int position);
    void appendMapElement(const QModelIndex& index);
    void appendMapElement(const QString& schemaname, QStringList& keys, gams::studio::connect::ConnectData* data, int position, const QModelIndex& index);
    void appendMapSchemaElement(const QString& schemaname, QStringList& keys, gams::studio::connect::ConnectData* data, const QModelIndex& parentIndex);
    void appendListElement(const QString& schemaname, QStringList& keys, gams::studio::connect::ConnectData* data, const QModelIndex& index);

    void insertLastListElement(const QString& schemaname, QStringList& keys, gams::studio::connect::ConnectData* data, const QModelIndex& index);
    void onlyRequriedAttributedChanged(int state);
    void reloadConnectDataModel();

    void onEditDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void editDataChanged(const QModelIndex &index, bool preValidValue);

protected:
    bool isIndexValueValid(int column, ConnectDataItem* item);
    bool hasSameParent(const QStringList& tobeinsertSchema, const QStringList& schemaKey, bool samelevel=true) const;
    bool existsUnderSameParent(const QString& tobeinsertSchema, const QModelIndex& parent, bool samelevel=true) const;
    bool isRequired(const QStringList& tobeinsertSchema, const QStringList& schemaKey) const;
    int numberOfExcludedSibling(ConnectDataItem* item);
    int numberOfExcludedChildren(ConnectDataItem* item);

    QModelIndex getSchemaParentIndex(const QModelIndex& idx);
    ConnectDataItem* getSchemaParentItem(ConnectDataItem* item);

    int whichAnyOfSchema(const YAML::Node &data, ConnectSchema* schema, const QStringList& keylist, const QString& key);
    int whichOneOfSchema(const YAML::Node &data, ConnectSchema *schema, const QStringList &keylist, const QString &key);
    int whichOneOfSchema(const YAML::Node &data, ConnectSchema* schema, const QString& key);
    void getSchemaListFromData(const YAML::Node& data, QStringList& schemaList);
    void getData(ConnectDataItem* item, YAML::Node& node);
    void informDataChanged(const QModelIndex& parent);

    void setupTreeItemModelData();
    void insertSchemaData(const QString& schemaname, const QStringList& keys, ConnectData* data, int position, QList<ConnectDataItem*>& parents);
    void updateInvalidExcludedItem(ConnectDataItem* item);
    bool updateInvaldItem(int column, ConnectDataItem* item);

    bool             mOnlyRequriedAttributesAdded;
    int              mItemIDCount;
    QString          mLocation;
    Connect*         mConnect;
    ConnectData*     mConnectData;
    ConnectDataItem* mRootItem;

    static const QString TooltipStrHeader;
    static const QString TooltipStrFooter;
    static const QString TooltipOpenedBoldStr;
    static const QString TooltipClosedBoldStr;
    static QRegularExpression mRexWhitechar;
    static QRegularExpression mRexNumber;
};

} // namespace connect
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_CONNECT_CONNECTDATAMODEL_H
