#ifndef SOLVERTABLEMODEL_H
#define SOLVERTABLEMODEL_H

#include <QAbstractTableModel>

#include "gamslicenseinfo.h"

namespace gams {
namespace studio {
namespace support {

class SolverTableModel
        : public QAbstractTableModel
{
    Q_OBJECT

public:
    SolverTableModel(QObject *parent = nullptr);

    virtual QVariant headerData(int section, Qt::Orientation oriantation, int role = Qt::DisplayRole) const override;

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    virtual int rowCount(const QModelIndex &parennt = QModelIndex()) const override;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    GamsLicenseInfo mLicenseInfo;
    QMap<int, QString> mHorizontalHeaderData;
};

}
}
}

#endif // SOLVERTABLEMODEL_H
