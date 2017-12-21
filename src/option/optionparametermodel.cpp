#include "option.h"
#include "optionparametermodel.h"

namespace gams {
namespace studio {

OptionParameterModel::OptionParameterModel(const QString normalizedCommandLineStr, CommandLineTokenizer* tokenizer, QObject* parent):
    QAbstractTableModel(parent), commandLineTokenizer(tokenizer)
{
    mHeader.append("Key");
    mHeader.append("Value");

    mOptionItem = commandLineTokenizer->tokenize(normalizedCommandLineStr);
    for(int idx = 0; idx<mOptionItem.size(); ++idx)
       mCheckState[idx] = QVariant();

    gamsOption = commandLineTokenizer->getGamsOption();
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
    case Qt::DisplayRole: {
        if (mOptionItem.isEmpty())
            return QVariant();
        if (col==0)
            return mOptionItem.at(row).key;
        else if (col== 1)
                 return mOptionItem.at(row).value;
        else
            break;
    }
    case Qt::TextAlignmentRole: {
        return Qt::AlignLeft;
    }
//    case Qt::DecorationRole
//    case Qt::BackgroundRole:
    case Qt::TextColorRole: {
        if (gamsOption->isDoubleDashedOption(mOptionItem.at(row).key)) // double dashed parameter
            return QVariant::fromValue(QColor(Qt::black));

        if (gamsOption->isValid(mOptionItem.at(row).key) || gamsOption->isThereASynonym(mOptionItem.at(row).key)) { // valid option
           if (gamsOption->isDeprecated(mOptionItem.at(row).key)) { // deprecated option
               return QVariant::fromValue(QColor(Qt::gray));
           } else { // valid and not deprected Option
                if (col==0) {
                   return  QVariant::fromValue(QColor(Qt::black));
                } else {

                    switch (gamsOption->getValueErrorType(mOptionItem.at(row).key, mOptionItem.at(row).value)) {
                     case No_Error:
                           return QVariant::fromValue(QColor(Qt::black));
                     case Incorrect_Value_Type:
                        return QVariant::fromValue(QColor(Qt::green));
                     case Value_Out_Of_Range:
                           return QVariant::fromValue(QColor(Qt::blue));
                     default:
                          return QVariant::fromValue(QColor(Qt::black));
                    }
                }
           }
        } else { // invalid option
            if (col == 0)
               return QVariant::fromValue(QColor(Qt::red));
            else
                return QVariant::fromValue(QColor(Qt::black));
        }

     }
     default:
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
    if (orientation != Qt::Vertical || role != Qt::CheckStateRole)
        return false;

    mCheckState[index] = value;
    emit headerDataChanged(orientation, index, index);

    return true;
}

bool OptionParameterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    QString data = value.toString().simplified();
//    qDebug() << QString("(%1, %2) : [%3] %4 %5").arg(index.row()).arg(index.column()).arg(value.toString()).arg(role).arg(val.simplified().isEmpty());

    if (data.isEmpty())
        return false;

    if (role == Qt::EditRole)   {
       if (index.column() == 0) { // key
           mOptionItem[index.row()].key = data;
       } else if (index.column() == 1) { // value
                 mOptionItem[index.row()].value = data;
       }
    }

    emit editCompleted(  commandLineTokenizer->normalize( mOptionItem ) );
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
