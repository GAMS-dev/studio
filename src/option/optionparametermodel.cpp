#include "option.h"
#include "optionparametermodel.h"

namespace gams {
namespace studio {

OptionParameterModel::OptionParameterModel(const QString& initCommandLineStr, CommandLineTokenizer* tokenizer, QObject* parent):
    QAbstractTableModel(parent), mCommandLineTokenizer(tokenizer)
{
    mHeader.append("Active");
    mHeader.append("Key");
    mHeader.append("Value");

    mOptionItem = mCommandLineTokenizer->tokenize(initCommandLineStr);
}

QVariant OptionParameterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section <= mHeader.size())
                return mHeader.at(section);
        }
    }
    return QVariant();
}

int OptionParameterModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return (mOptionItem.size()==0 ) ? 1 : mOptionItem.size();
}

int OptionParameterModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mHeader.size();
}

QVariant OptionParameterModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    switch (role) {
    case Qt::DisplayRole:
        if (mOptionItem.isEmpty())
            return QVariant();
        if (col==1)
            return mOptionItem.at(row).key;
        else if (col== 2)
                 return mOptionItem.at(row).value;
        else
            break;
    case Qt::TextAlignmentRole:
        if (col == 0) //change text alignment only for cell(i,0)
           return Qt::AlignHCenter;
        break;
    case Qt::CheckStateRole:
        if (col == 0) {
            if (mOptionItem.isEmpty()) {
                return Qt::Unchecked;
            } else {
                   return Qt::Checked;
                }
            }
    }
    return QVariant();
}

QModelIndex OptionParameterModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return QAbstractTableModel::createIndex(row, column);
    return QModelIndex();
}

} // namespace studio
} // namespace gams
