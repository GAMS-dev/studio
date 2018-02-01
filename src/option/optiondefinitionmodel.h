#ifndef OPTIONDEFINITIONMODEL_H
#define OPTIONDEFINITIONMODEL_H

#include "option/option.h"
#include "option/optiondefinitionitem.h"

namespace gams {
namespace studio {

class OptionDefinitionModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    OptionDefinitionModel(Option* data, QObject* parent=0);
    ~OptionDefinitionModel();

    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

private:
    void setupTreeItemModelData(Option* option, OptionDefinitionItem* parent);
    void setupModelData(const QStringList& lines, OptionDefinitionItem* parent);

    OptionDefinitionItem *rootItem;

};

} // namespace studio
} // namespace gams

#endif // OPTIONDEFINITIONMODEL_H
