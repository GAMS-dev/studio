#ifndef GAMS_STUDIO_LIBRARYMODEL_H
#define GAMS_STUDIO_LIBRARYMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "libraryitem.h"

namespace gams {
namespace studio {

class LibraryModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit LibraryModel(QList<LibraryItem> data, QObject *parent = 0);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

private:
    QList<LibraryItem> mData;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_LIBRARYMODEL_H
