#include "option.h"
#include "optionparametermodel.h"

namespace gams {
namespace studio {

OptionParameterModel::OptionParameterModel(const QString normalizedCommandLineStr, CommandLineTokenizer* tokenizer, QObject* parent):
    QAbstractTableModel(parent), commandLineTokenizer(tokenizer)
{
    mHeader.append("Key");
    mHeader.append("Value");

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

    // orientation == Qt::Vertical
    switch(role) {
    case Qt::CheckStateRole:
        if (mOptionItem.isEmpty())
            return QVariant();
        else
            return mCheckState[index];
    case Qt::DecorationRole:
        if (Qt::CheckState(mCheckState[index].toUInt())==Qt::Checked) {
            QIcon icon(":/img/square-red");
            QPixmap p = icon.pixmap(12,12);
            return p;
        } else {
            QIcon icon(":/img/square-green");
            QPixmap p = icon.pixmap(12,12);
            return p;
        }
    }

    return QVariant();
}

int OptionParameterModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return  mOptionItem.size();
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

    if (mOptionItem.isEmpty())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
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
    case Qt::ToolTipRole: {
//        if (Qt::CheckState(mCheckState[index.row()].toUInt()))
//            return QString("'%1' has been disabled").arg(mOptionItem.at(row).key);
        if (col==0) {
            if ( !gamsOption->isValid(mOptionItem.at(row).key) &&
                 !gamsOption->isThereASynonym(mOptionItem.at(row).key) )  {
                return QString("'%1' is an unknown Option Key").arg(mOptionItem.at(row).key);
            } else if (gamsOption->isDeprecated(mOptionItem.at(row).key)) {
                      return QString("Option '%1' is deprecated, will be ignored").arg(mOptionItem.at(row).key);
            }
        } else if (col==1) {
            switch (gamsOption->getValueErrorType(mOptionItem.at(row).key, mOptionItem.at(row).value)) {
              case Incorrect_Value_Type:
                   return QString("Option key '%1' has an incorrect value type").arg(mOptionItem.at(row).key);
              case Value_Out_Of_Range:
                   return QString("Value '%1' for option key '%2' is out of range").arg(mOptionItem.at(row).value).arg(mOptionItem.at(row).key);
              default:
                   break;
            }
        }
        break;
    }
    case Qt::TextColorRole: {
//        if (Qt::CheckState(headerData(index.row(), Qt::Vertical, Qt::CheckStateRole).toBool()))
//            return QVariant::fromValue(QColor(Qt::gray));

        if (gamsOption->isDoubleDashedOption(mOptionItem.at(row).key)) { // double dashed parameter
           if (mOptionItem.at(row).key.mid(2).contains(QRegExp("^[a-zA-Z]")) )
                return QVariant::fromValue(QColor(Qt::black));
           else
                 return QVariant::fromValue(QColor(Qt::red));
        }
        if (gamsOption->isValid(mOptionItem.at(row).key) || gamsOption->isThereASynonym(mOptionItem.at(row).key)) { // valid option
           if (gamsOption->isDeprecated(mOptionItem.at(row).key)) { // deprecated option
               return QVariant::fromValue(QColor(Qt::gray));
           } else { // valid and not deprected Option
                if (col==0) {
                   return  QVariant::fromValue(QColor(Qt::black));
                } else {

                    switch (gamsOption->getValueErrorType(mOptionItem.at(row).key, mOptionItem.at(row).value)) {
                     case Incorrect_Value_Type:
                           return QVariant::fromValue(QColor(Qt::red/*Qt::blue*/));
                     case Value_Out_Of_Range:
                           return QVariant::fromValue(QColor(Qt::red/*Qt::blue*/));
                     case No_Error:
                           return QVariant::fromValue(QColor(Qt::black));
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
    if (role == Qt::EditRole)   {
        QString data = value.toString().simplified();
        if (data.isEmpty())
            return false;

        if (index.row() > mOptionItem.size())
            return false;

        if (index.column() == 0) { // key
            mOptionItem[index.row()].key = data;
        } else if (index.column() == 1) { // value
                  mOptionItem[index.row()].value = data;
        }
        emit optionModelChanged(  mOptionItem );
    } else if (role == Qt::CheckStateRole) {
        if (index.row() > mOptionItem.size())
            return false;

        mOptionItem[index.row()].disabled = value.toBool();
        emit optionModelChanged(  mOptionItem );
    }
    emit dataChanged(index, index);
    return true;
}

QModelIndex OptionParameterModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return QAbstractTableModel::createIndex(row, column);
    return QModelIndex();
}

bool OptionParameterModel::insertRows(int row, int count, const QModelIndex &parent = QModelIndex())
{
    if (count < 1 || row < 0 || row > mOptionItem.size())
         return false;

     beginInsertRows(QModelIndex(), row, row + count - 1);
     if (mOptionItem.size() == row)
         mOptionItem.append(OptionItem("[KEY]", "[VALUE]", -1, -1));
     else
         mOptionItem.insert(row, OptionItem(OptionItem("[KEY]", "[VALUE]", -1, -1)));

    endInsertRows();
    emit optionModelChanged(mOptionItem);
    return true;
}

bool OptionParameterModel::removeRows(int row, int count, const QModelIndex &parent = QModelIndex())
{
    if (count < 1 || row < 0 || row > mOptionItem.size() || mOptionItem.size() ==0)
         return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    mOptionItem.removeAt(row);
    endRemoveRows();
    emit optionModelChanged(mOptionItem);
    return true;
}

bool OptionParameterModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if (mOptionItem.size() == 0 || count < 1 || destinationChild < 0 ||  destinationChild > mOptionItem.size())
         return false;

    Q_UNUSED(sourceParent); Q_UNUSED(destinationParent);
    beginMoveRows(QModelIndex(), sourceRow, sourceRow  + count - 1, QModelIndex(), destinationChild);
    mOptionItem.insert(destinationChild, mOptionItem.at(sourceRow));
    int removeIndex = destinationChild > sourceRow ? sourceRow : sourceRow+1;
    mOptionItem.removeAt(removeIndex);
    endMoveRows();
    emit optionModelChanged(mOptionItem);
    return true;
}

QList<OptionItem> OptionParameterModel::getCurrentListOfOptionItems()
{
    return mOptionItem;
}

void OptionParameterModel::toggleActiveOptionItem(int index)
{
    if (mOptionItem.isEmpty() || index >= mOptionItem.size())
        return;

    bool checked = (headerData(index, Qt::Vertical, Qt::CheckStateRole).toUInt() != Qt::Checked) ? true : false;
    setHeaderData( index, Qt::Vertical,
                          Qt::CheckState(checked ? Qt::Checked : Qt::Unchecked),
                          Qt::CheckStateRole );
    setData(QAbstractTableModel::createIndex(index, 0), QVariant(checked), Qt::CheckStateRole);
    emit optionModelChanged(mOptionItem);
}

void OptionParameterModel::updateCurrentOption(const QString &text)
{
    beginResetModel();
    itemizeOptionFromCommandLineStr(text);
    validateOption();

    setRowCount(mOptionItem.size());

    for (int i=0; i<mOptionItem.size(); ++i) {
        setData(QAbstractTableModel::createIndex(i, 0), QVariant(mOptionItem.at(i).key), Qt::EditRole);
        setData(QAbstractTableModel::createIndex(i, 1), QVariant(mOptionItem.at(i).value), Qt::EditRole);
        if (mOptionItem.at(i).error == No_Error)
            setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::Unchecked),
                              Qt::CheckStateRole );
        else
            setHeaderData( i, Qt::Vertical,
                              Qt::CheckState(Qt::Checked),
                              Qt::CheckStateRole );
    }
    endResetModel();
    emit optionModelChanged(mOptionItem);
}

void OptionParameterModel::setRowCount(int rows)
{
   int rc = mOptionItem.size();
   if (rows < 0 ||  rc == rows)
      return;

   if (rc < rows)
      insertRows(qMax(rc, 0), rows - rc);
   else
      removeRows(qMax(rows, 0), rc - rows);
}

void OptionParameterModel::itemizeOptionFromCommandLineStr(const QString text)
{
    QMap<int, QVariant> previousCheckState = mCheckState;
    mOptionItem.clear();
    mOptionItem = commandLineTokenizer->tokenize(text);
    for(int idx = 0; idx<mOptionItem.size(); ++idx) {
       mCheckState[idx] = QVariant();
    }
}

void OptionParameterModel::validateOption()
{
   for(OptionItem& item : mOptionItem) {
       if (gamsOption->isDoubleDashedOption(item.key)) { // double dashed parameter
           if (!item.key.mid(2).contains(QRegExp("^[a-zA-Z]")) )
               item.error = OptionErrorType::Invalid_Key;
           else
              item.error = OptionErrorType::No_Error;
           continue;
       }
       if (gamsOption->isValid(item.key) || gamsOption->isThereASynonym(item.key)) { // valid option
           if (gamsOption->isDeprecated(item.key)) { // deprecated option
               item.error = OptionErrorType::Deprecated_Option;
           } else { // valid and not deprected Option
               item.error = gamsOption->getValueErrorType(item.key, item.value);
           }
       } else { // invalid option
           item.error = OptionErrorType::Invalid_Key;
       }
   }
}

} // namespace studio
} // namespace gams
