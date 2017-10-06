#include "librarymodel.h"
#include "library.h"
#include <QDebug>

namespace gams {
namespace studio {

LibraryModel::LibraryModel(QList<LibraryItem> data, QObject *parent)
    : QAbstractTableModel(parent), mData(data)
{
}

QVariant LibraryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
            return mData.at(0).library()->columns().at(section);
    }
    return QVariant();
}

int LibraryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mData.size();
}

int LibraryModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mData.at(0).library()->nrColumns();
}

QVariant LibraryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
        return mData.at(index.row()).values().at(mData.at(index.row()).library()->colOrder().at(index.column()));
    return QVariant();
}

} // namespace studio
} // namespace gams
