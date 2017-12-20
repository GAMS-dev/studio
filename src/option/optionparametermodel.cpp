#include "option.h"
#include "optionparametermodel.h"

namespace gams {
namespace studio {

OptionParameterModel::OptionParameterModel(const QList<OptionItem>& optionItem, CommandLineTokenizer* tokenizer, QObject* parent):
    QAbstractTableModel(parent), mOptionItem(optionItem), mTokenizer(tokenizer)
{
    mHeader.append("Key");
    mHeader.append("Value");

    for(int idx = 0; idx<mOptionItem.size(); ++idx)
       mCheckState[idx] = QVariant();

}

QVariant OptionParameterModel::headerData(int index, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
       if (role == Qt::DisplayRole) {
          if (index <= mHeader.size())
              return mHeader.at(index);
       }
       return QVariant();
    }

    switch(role) {
    case Qt::CheckStateRole:
        return mCheckState[index];
    case Qt::DecorationRole:
        QPixmap p{12,12};
        p.fill(Qt::CheckState(headerData(index, orientation, Qt::CheckStateRole).toUInt()) ? Qt::red : Qt::green);
        return p;
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
        if (col==0)
            return mOptionItem.at(row).key;
        else if (col== 1)
                 return mOptionItem.at(row).value;
        else
            break;
    case Qt::TextAlignmentRole:
        if (col == 0) //change text alignment only for cell(i,0)
           return Qt::AlignLeft;
        break;
//    case Qt::BackgroundRole:
    case Qt::TextColorRole:
        if (col==0)
           return QVariant::fromValue(QColor(Qt::blue));
        break;
    }
    return QVariant();
}

Qt::ItemFlags OptionParameterModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

bool OptionParameterModel::setHeaderData(int index, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation != Qt::Vertical)
        return false;
//    if (role != Qt::EditRole )
//        return false;

    mCheckState[index] = value;
    emit headerDataChanged(orientation, index, index);

    return true;
}

bool OptionParameterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    qDebug() << QString("(%1, %2) : [%3] %4").arg(index.row()).arg(index.column()).arg(value.toString()).arg(role);

//    mOptionItem.at(row).key;

    if (role == Qt::EditRole)   {
       if (index.column() == 0) { // key
           mOptionItem[index.row()].key = value.toString();
       } else if (index.column() == 1) { // value
                 mOptionItem[index.row()].value = value.toString();
       }
    }

    emit editCompleted(  mTokenizer->normalize( mOptionItem ) );
    return true;
}

QModelIndex OptionParameterModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return QAbstractTableModel::createIndex(row, column);
    return QModelIndex();
}

} // namespace studio
} // namespace gams
