#ifndef OPTIONPARAMETERMODEL_H
#define OPTIONPARAMETERMODEL_H

#include "commandlinetokenizer.h"
#include "option.h"

namespace gams {
namespace studio {

class OptionParameterModel : public QAbstractTableModel
{
     Q_OBJECT
public:
    OptionParameterModel(const QString& initCommandLineStr, CommandLineTokenizer* tokenizer, QObject *parent = 0);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

private:
    QList<OptionItem> mOptionItem;
    QList<QString> mHeader;

    CommandLineTokenizer* mCommandLineTokenizer;
};

} // namespace studio
} // namespace gams

#endif // OPTIONPARAMETERMODEL_H
