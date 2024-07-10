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
#ifndef ENVVARTABLEMODEL_H
#define ENVVARTABLEMODEL_H

#include <QAbstractTableModel>
#include <QRegularExpression>
#include "gamsuserconfig.h"

namespace gams {
namespace studio {
namespace option {

class EnvVarTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    EnvVarTableModel(const QList<EnvVarConfigItem *> &itemList, QObject *parent = nullptr);

    QVariant headerData(int index, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setHeaderData(int index, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;

    static const int COLUMN_PARAM_KEY = 0;
    static const int COLUMN_PARAM_VALUE = 1;
    static const int COLUMN_MIN_VERSION = 2;
    static const int COLUMN_MAX_VERSION = 3;
    static const int COLUMN_PATH_VAR = 4;

    QList<EnvVarConfigItem *> envVarConfigItems();

signals:
    void envVarItemRemoved();

public slots:
    void on_reloadEnvVarModel(const QList<gams::studio::option::EnvVarConfigItem *> &configItem);
    void on_updateEnvVarItem(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void on_removeEnvVarItem();

private slots:
    void setRowCount(int rows);
    void updateCheckState();

private:
    bool isThereAnError(gams::studio::option::EnvVarConfigItem* item) const;
    bool isConformatVersion(const QString &version) const;

private:
    QList<EnvVarConfigItem *> mEnvVarItem;
    QList<QString> mHeader;
    QMap<int, QVariant> mCheckState;
    bool mModified;
    static QRegularExpression mRexVersion;
};

} // namepsace option
} // namespace studio
} // namespace gams
#endif // ENVVARTABLEMODEL_H
