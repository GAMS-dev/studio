/*
 * This file is part of the GAMS Studio project.
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
#include "symboltablemodel.h"

namespace gams {
namespace studio {
namespace reference {

SymbolTableModel::SymbolTableModel(SymbolDataType::SymbolType type, QObject *parent) :
    QAbstractTableModel(parent), mType(type), mReference(nullptr)
{
    mAllSymbolsHeader << "Entry" << "Name" << "Type" << "Dim" << "Domain" << "Text";
    mSymbolsHeader    << "Entry" << "Name"           << "Dim" << "Domain" << "Text";
    mFileHeader       << "Entry" << "Name"           << "Text";
    mFileUsedHeader   << "File Location";

    resetSizeAndIndices();

    if (mType == SymbolDataType::FileUsed)
        mCurrentSortedColumn = 0;
    else
        mCurrentSortedColumn = 1;
    mCurrentAscendingSort = Qt::AscendingOrder;
}

QVariant SymbolTableModel::headerData(int index, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
       if (role == Qt::DisplayRole) {
           switch(mType) {
           case SymbolDataType::Unknown :
           case SymbolDataType::Unused :
               if (index < mAllSymbolsHeader.size())
                  return mAllSymbolsHeader[index];
               break;
           case SymbolDataType::Set :
           case SymbolDataType::Acronym :
           case SymbolDataType::Parameter :
           case SymbolDataType::Variable :
           case SymbolDataType::Equation :
               if (index < mSymbolsHeader.size())
                  return mSymbolsHeader[index];
               break;
           case SymbolDataType::Model :
           case SymbolDataType::Funct :
           case SymbolDataType::Macro :
           case SymbolDataType::File :
               if (index < mFileHeader.size())
                  return mFileHeader[index];
               break;
           case SymbolDataType::FileUsed :
               if (index < mFileUsedHeader.size())
                  return mFileUsedHeader[index];
               break;
           }
       }
    }
    return QVariant();
}

int SymbolTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(mFilteredRecordSize);
}

int SymbolTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    switch(mType) {
    case SymbolDataType::Unknown :
    case SymbolDataType::Unused :
        return mAllSymbolsHeader.size();
    case SymbolDataType::Set :
    case SymbolDataType::Acronym :
    case SymbolDataType::Parameter :
    case SymbolDataType::Variable :
    case SymbolDataType::Equation :
        return mSymbolsHeader.size();
    case SymbolDataType::File :
    case SymbolDataType::Funct :
    case SymbolDataType::Macro :
    case SymbolDataType::Model :
        return mFileHeader.size();
    case SymbolDataType::FileUsed :
        return mFileUsedHeader.size();
    }
    return 0;

}

QVariant SymbolTableModel::data(const QModelIndex &index, int role) const
{
    if (!mReference)
        return QVariant();

    if (mReference->isEmpty())
        return QVariant();

    switch (role) {
    case Qt::TextAlignmentRole: {
        if (mType==SymbolDataType::FileUsed) {
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
        Qt::AlignmentFlag aFlag;
        switch(index.column()) {
        case 0: aFlag = Qt::AlignRight; break;
        case 1: aFlag = Qt::AlignLeft; break;
        case 2: if (mType == SymbolDataType::Unknown || mType == SymbolDataType::Unused ||
                    mType == SymbolDataType::Model || mType == SymbolDataType::Funct ||
                    mType == SymbolDataType::File || mType == SymbolDataType::Macro )
                        aFlag = Qt::AlignLeft;
                    else
                        aFlag = Qt::AlignRight;
                    break;
        case 3: if (mType == SymbolDataType::Unknown || mType == SymbolDataType::Unused)
                        aFlag = Qt::AlignRight;
                    else
                        aFlag = Qt::AlignLeft;
                    break;
            default: aFlag = Qt::AlignLeft; break;
        }
        return QVariant(aFlag | Qt::AlignVCenter);
    }
    case Qt::DisplayRole: {
         QList<SymbolReferenceItem*> refList = mReference->findReferenceFromType(mType);
         int idx = static_cast<int>( mSortIdxMap[mFilterIdxMap[static_cast<size_t>(index.row())]] );
         if (idx < 0 || idx >= refList.size())
             break;
         switch(mType) {
         case SymbolDataType::Set :
         case SymbolDataType::Acronym :
         case SymbolDataType::Parameter :
         case SymbolDataType::Variable :
         case SymbolDataType::Equation :
             switch(index.column()) {
             case 0: return QString::number( refList.at(idx)->id() );
             case 1: return refList.at(idx)->name();
             case 2: return QString::number( refList.at(idx)->dimension() );
             case 3: return getDomainStr( refList.at(idx)->domain() );
             case 4: return refList.at(idx)->explanatoryText();
             default: break;
             }
             break;
         case SymbolDataType::Unknown :
         case SymbolDataType::Unused :
              switch(index.column()) {
              case 0: return QString::number( refList.at(idx)->id() );
              case 1: return refList.at(idx)->name();
              case 2: return SymbolDataType::from( refList.at(idx)->type() ).name();
              case 3: return QString::number( refList.at(idx)->dimension() );
              case 4: return getDomainStr( refList.at(idx)->domain() );
              case 5: return refList.at(idx)->explanatoryText();
              default: break;
              }
              break;
         case SymbolDataType::File :
         case SymbolDataType::Funct :
         case SymbolDataType::Macro :
         case SymbolDataType::Model :
             switch(index.column()) {
             case 0: return QString::number( refList.at(idx)->id() );
             case 1: return refList.at(idx)->name();
             case 2: return SymbolDataType::from( refList.at(idx)->type() ).name();
             case 3: return refList.at(idx)->explanatoryText();
             default: break;
             }
             break;
         case SymbolDataType::FileUsed :
             return mReference->getFileUsed().at(idx);
         }
         break;
    }
    default:
        break;
    }

    return QVariant();
}

void SymbolTableModel::sort(int column, Qt::SortOrder order)
{
    mCurrentSortedColumn = column;
    mCurrentAscendingSort = order;

    if (!mReference)
        return;

    QList<SymbolReferenceItem *> items = mReference->findReferenceFromType(mType);
    SortType sortType = getSortTypeOf(column);
    ColumnType colType = getColumnTypeOf(column);

    switch(sortType) {
    case sortInt: {

        QList<QPair<int, int>> idxList;
        for(int rec=0; rec<items.size(); rec++) {
            int idx = static_cast<int>(mSortIdxMap[static_cast<size_t>(rec)]);
            if (colType == columnId)
               idxList.append(QPair<int, int>(idx, items.at(idx)->id()) );
            else if (colType == columnDimension)
                    idxList.append(QPair<int, int>(idx, items.at(idx)->dimension()) );
        }
        if (order == Qt::SortOrder::AscendingOrder)
           std::stable_sort(idxList.begin(), idxList.end(), [](QPair<int, int> a, QPair<int, int> b) { return a.second < b.second; });
        else
           std::stable_sort(idxList.begin(), idxList.end(), [](QPair<int, int> a, QPair<int, int> b) { return a.second > b.second; });

        for(int rec=0; rec<items.size(); rec++) {
            mSortIdxMap[static_cast<size_t>(rec)] = static_cast<size_t>(idxList.at(rec).first);
        }
        filterRows();
        emit layoutChanged();
        emit symbolSelectionToBeUpdated();
        break;
    }
    case sortString: {

        QList<QPair<int, QString>> idxList;
        if (colType == columnFileLocation) {
            QStringList fileUsed = mReference->getFileUsed();
            for(int rec=0; rec<fileUsed.size(); rec++) {
                idxList.append(QPair<int, QString>(rec, mReference->getFileUsed().at(rec)) );
            }
        } else  {
            for(int rec=0; rec<items.size(); rec++) {
                int idx = static_cast<int>(mSortIdxMap[static_cast<size_t>(rec)]);
                if (colType == columnName) {
                    idxList.append(QPair<int, QString>(idx, items.at(idx)->name()) );
                } else if (colType == columnText) {
                          idxList.append(QPair<int, QString>(idx, items.at(idx)->explanatoryText()) );
                } else if (colType == columnType) {
                          SymbolDataType::SymbolType type = items.at(idx)->type();
                          idxList.append(QPair<int, QString>(idx, SymbolDataType::from(type).name()) );
                } else if (colType == columnDomain) {
                          QString domainStr = getDomainStr( items.at(idx)->domain() );
                          idxList.append(QPair<int, QString>(idx, domainStr) );
                } else  {
                    idxList.append(QPair<int, QString>(idx, "") );
                }
            }
        }
        if (order == Qt::SortOrder::AscendingOrder)
           std::stable_sort(idxList.begin(), idxList.end(), [](const QPair<int, QString> &a, const QPair<int, QString> &b) { return (QString::localeAwareCompare(a.second, b.second) < 0); });
        else
           std::stable_sort(idxList.begin(), idxList.end(), [](const QPair<int, QString> &a, const QPair<int, QString> &b) { return (QString::localeAwareCompare(a.second, b.second) > 0); });

        if (colType == columnFileLocation) {
            for(int rec=0; rec< mReference->getFileUsed().size(); rec++) {
                mSortIdxMap[static_cast<size_t>(rec)] = static_cast<size_t>(idxList.at(rec).first);
            }
            filterRows();
            emit layoutChanged();
        } else {
            for(int rec=0; rec<items.size(); rec++) {
                mSortIdxMap[static_cast<size_t>(rec)] = static_cast<size_t>(idxList.at(rec).first);
            }
            filterRows();
            emit layoutChanged();
            emit symbolSelectionToBeUpdated();
        }
        break;
    }
    case sortUnknown:  {
        break;
    }
    }
}

QModelIndex SymbolTableModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return QAbstractTableModel::createIndex(row, column);
    return QModelIndex();
}

void SymbolTableModel::resetModel()
{
    beginResetModel();
    resetSizeAndIndices();
    if (rowCount() > 0) {
        removeRows(0, rowCount(), QModelIndex());
    }
    if (mType != SymbolDataType::FileUsed)
       sort(mCurrentSortedColumn, mCurrentAscendingSort);
    endResetModel();
}

void SymbolTableModel::initModel(Reference *ref)
{
    mReference = ref;
    resetModel();
}

bool SymbolTableModel::isModelLoaded()
{
    return (mReference!=nullptr);
}

int SymbolTableModel::getSortedIndexOf(const SymbolId id) const
{
    if (!mReference)
        return -1;

    QList<SymbolReferenceItem *> items = mReference->findReferenceFromType(mType);
    int rec;
    for(rec=0; rec<items.size(); rec++) {
        int idx = static_cast<int>( mSortIdxMap[mFilterIdxMap[ static_cast<size_t>(rec) ] ] );
        if (items.at(idx)->id() == id)
            break;
    }
    return rec < items.size() ? rec : -1;
}

int SymbolTableModel::getSortedIndexOf(const QString &name) const
{
    if (!mReference)
        return -1;

    if (mType == SymbolDataType::FileUsed) {
       QStringList items = mReference->getFileUsed();
       int rec = -1;
       for(rec=0; rec<items.size(); rec++) {
           int idx = static_cast<int>( mSortIdxMap[mFilterIdxMap[ static_cast<size_t>(rec) ] ] );
           if (QString::localeAwareCompare(items.at(idx), name) == 0)
               break;
       }
       return rec < items.size() ? rec : -1;
    } else {
       QList<SymbolReferenceItem *> items = mReference->findReferenceFromType(mType);
       int rec = -1;
       for(rec=0; rec<items.size(); rec++) {
           int idx = static_cast<int>( mSortIdxMap[mFilterIdxMap[ static_cast<size_t>(rec) ] ] );
           if (QString::localeAwareCompare(items.at(idx)->name(), name) == 0)
               break;
       }
       return rec < items.size() ? rec : -1;
    }
}

void SymbolTableModel::toggleSearchColumns(bool checked)
{
    if (checked) {
        mFilteredKeyColumn = -1;
    } else {
        if (mType == SymbolDataType::FileUsed)
            mFilteredKeyColumn = 0;
        else
            mFilteredKeyColumn = 1;
    }
    filterRows();
    emit layoutChanged();
    emit symbolSelectionToBeUpdated();
}

void SymbolTableModel::setFilterPattern(const QRegularExpression &pattern)
{
    mFilteredPattern = pattern;
    filterRows();
    emit layoutChanged();
    emit symbolSelectionToBeUpdated();
}

int SymbolTableModel::getLastSectionIndex()
{
    switch(mType) {
    case SymbolDataType::Set :
    case SymbolDataType::Acronym :
    case SymbolDataType::Parameter :
    case SymbolDataType::Variable :
    case SymbolDataType::Equation :
        return 4;
    case SymbolDataType::Model :
    case SymbolDataType::Funct :
    case SymbolDataType::File :
    case SymbolDataType::Macro :
        return 2;
    case SymbolDataType::FileUsed :
        return 0;
    case SymbolDataType::Unknown :
    case SymbolDataType::Unused :
        return 5;
    default:
       return 0;
    }
}

void SymbolTableModel::sortFileUsed(FileUsedSortOrder order)
{
    if (mType != SymbolDataType::FileUsed)
        return;

    QList<QPair<int, QString>> idxList;
    QStringList fileUsed = mReference->getFileUsed();
    for(int rec=0; rec<fileUsed.size(); rec++) {
        idxList.append(QPair<int, QString>(rec, mReference->getFileUsed().at(rec)) );
    }

    if (order == FileUsedSortOrder::AscendingOrder)
       std::stable_sort(idxList.begin(), idxList.end(), [](const QPair<int, QString> &a, const QPair<int, QString> &b) { return (QString::localeAwareCompare(a.second, b.second) < 0); });
    else if (order == FileUsedSortOrder::DescendingOrder)
             std::stable_sort(idxList.begin(), idxList.end(), [](const QPair<int, QString> &a, const QPair<int, QString> &b) { return (QString::localeAwareCompare(a.second, b.second) > 0); });

    for(int rec=0; rec< mReference->getFileUsed().size(); rec++) {
        mSortIdxMap[static_cast<size_t>(rec)] = static_cast<size_t>(idxList.at(rec).first);
    }
    filterRows();
    emit layoutChanged();
}

SymbolTableModel::SortType SymbolTableModel::getSortTypeOf(int column) const
{
    switch(mType) {
    case SymbolDataType::Set :
    case SymbolDataType::Acronym :
    case SymbolDataType::Parameter :
    case SymbolDataType::Variable :
    case SymbolDataType::Equation : {
        if (column == 0 || column == 2)
           return sortInt;
        else if (column == 1 || column == 3 || column == 4)
                return sortString;
        else
            return sortUnknown;
    }
    case SymbolDataType::Model :
    case SymbolDataType::Funct :
    case SymbolDataType::Macro :
    case SymbolDataType::File : {
        if (column == 0)
           return sortInt;
        else if (column == 1 || column ==2 || column == 3)
                return sortString;
        else
            return sortUnknown;
    }
    case SymbolDataType::FileUsed : {
           return sortString;
    }
    case SymbolDataType::Unknown :
    case SymbolDataType::Unused : {
        if (column == 0 || column == 3)
           return sortInt;
        else if (column == 1 || column == 2 || column == 4 || column == 5)
                return sortString;
        else
            return sortUnknown;
    }
    }
    return sortUnknown;
}


SymbolTableModel::ColumnType SymbolTableModel::getColumnTypeOf(int column) const
{
    switch(mType) {
    case SymbolDataType::Set :
    case SymbolDataType::Acronym :
    case SymbolDataType::Parameter :
    case SymbolDataType::Variable :
    case SymbolDataType::Equation : {
        switch(column) {
        case 0 : return columnId;
        case 1 : return columnName;
        case 2 : return columnDimension;
        case 3 : return columnDomain;
        case 4:  return columnText;
        default: break;
        }
        break;
    }
    case SymbolDataType::Model :
    case SymbolDataType::Funct :
    case SymbolDataType::Macro :
    case SymbolDataType::File : {
        switch(column) {
        case 0 : return columnId;
        case 1 : return columnName;
        case 2 : return columnText;
        default: break;
        }
        break;
    }
    case SymbolDataType::Unknown :
    case SymbolDataType::Unused : {
        switch(column) {
        case 0 : return columnId;
        case 1 : return columnName;
        case 2 : return columnType;
        case 3 : return columnDimension;
        case 4 : return columnDomain;
        case 5:  return columnText;
        default: break;
        }
        break;
    }
    case SymbolDataType::FileUsed :
        return columnFileLocation;
    }
    return columnUnknown;
}

QString SymbolTableModel::getDomainStr(const QList<SymbolId>& domain) const
{
    if (!mReference)
        return "";
    if (domain.size() > 0) {
       QString domainStr = "(";
       SymbolReferenceItem* dom = mReference->findReferenceFromId( domain.at(0) );
       if (dom) domainStr.append( dom->name() );
       for(int i=1; i<domain.size(); i++) {
           dom = mReference->findReferenceFromId( domain.at(i) );
           if (dom) {
              domainStr.append( "," );
              domainStr.append( dom->name() );
           }
       }
       domainStr.append( ")" );
       return domainStr;
    } else {
        return "";
    }
}

bool SymbolTableModel::isFilteredActive(SymbolReferenceItem *item, int column, const QRegularExpression &regExp)
{
    if (mType == SymbolDataType::SymbolType::FileUsed) {
        return false;
    } else {
        ColumnType type = getColumnTypeOf(column);
        switch(type) {
        case columnId: return (!regExp.match(QString::number(item->id())).hasMatch());
        case columnName: return (!regExp.match(item->name()).hasMatch());
        default: // search every column
            QStringList strList = {
                QString::number( item->id() ),
                item->name(),
                SymbolDataType::from( item->type() ).name(),
                QString::number(item->dimension() ),
                getDomainStr( item->domain() ),
                item->explanatoryText()
            };
            for (const QString &val: strList)
                if (regExp.match(val).hasMatch()) return false;
            return true;
        }
    }
}

bool SymbolTableModel::isLocationFilteredActive(int idx, const QRegularExpression &regExp)
{
    if (mType == SymbolDataType::SymbolType::FileUsed) {
        return (!regExp.match(mReference->getFileUsed().at(idx)).hasMatch());
    } else {
        return false;
    }
}

void SymbolTableModel::filterRows()
{
    if (!mReference)
        return;

    size_t size = 0;
    // there is no filter
    if (mFilteredPattern.pattern().isEmpty()) {
        if (mType == SymbolDataType::SymbolType::FileUsed)
            size = static_cast<size_t>(mReference->getFileUsed().size());
        else
            size = static_cast<size_t>(mReference->findReferenceFromType(mType).size());
        for(size_t rec=0; rec<size; rec++) {
            mFilterActive[rec] = false;
            mFilterIdxMap[rec] = rec;
        }
        mFilteredRecordSize = size;
        beginResetModel();
        endResetModel();
        return;
    }

    // there is a filter
    if (mType == SymbolDataType::SymbolType::FileUsed) {
        QStringList items = mReference->getFileUsed();
        size = static_cast<size_t>(items.size());
        for(size_t rec=0; rec<size; rec++) {
            mFilterActive[mSortIdxMap[rec]] = isLocationFilteredActive(static_cast<int>(rec), mFilteredPattern);
        }

        size_t filteredRecordSize = 0;
        for(size_t rec=0; rec<size; rec++) {
            if (mFilterActive[mSortIdxMap[rec]])
               filteredRecordSize++;
        }

        size_t filteredRec = 0;
        for(size_t i=0; i<size; i++) {
           if (mFilterActive[mSortIdxMap[i]]) {
               continue;
           } else {
               mFilterIdxMap[filteredRec++] = i;
           }
        }

        mFilteredRecordSize = filteredRec;
        for(filteredRec=mFilteredRecordSize; filteredRec<size; filteredRec++) {
           mFilterIdxMap[filteredRec] = mSortIdxMap[filteredRec];
        }
    } else {
        QList<SymbolReferenceItem *> items = mReference->findReferenceFromType(mType);
        size = static_cast<size_t>(items.size());
        for(size_t rec=0; rec<size; rec++) {
            int idx = static_cast<int>(mSortIdxMap[rec]);
            mFilterActive[mSortIdxMap[rec]] = isFilteredActive(items.at(idx), mFilteredKeyColumn, mFilteredPattern);
        }

        size_t filteredRecordSize = 0;
        for(size_t rec=0; rec<size; rec++) {
            if (mFilterActive[mSortIdxMap[rec]])
               filteredRecordSize++;
        }

        size_t filteredRec = 0;
        for(size_t i=0; i<size; i++) {
           if (mFilterActive[mSortIdxMap[i]]) {
               continue;
           } else {
               mFilterIdxMap[filteredRec++] = i;
           }
        }

        mFilteredRecordSize = filteredRec;
        for(filteredRec=mFilteredRecordSize; filteredRec<size; filteredRec++) {
           mFilterIdxMap[filteredRec] = mSortIdxMap[filteredRec];
        }
    }
    beginResetModel();
    endResetModel();
}

void SymbolTableModel::resetSizeAndIndices()
{
    size_t size = 0;

    if (mType == SymbolDataType::SymbolType::FileUsed) {
        if (mReference)
            size = static_cast<size_t>(mReference->getFileUsed().size());
        mFilteredKeyColumn = 0;
    } else {
        if (mReference)
           size = static_cast<size_t>(mReference->findReferenceFromType(mType).size());
        mFilteredKeyColumn = 1;
    }
    mSortIdxMap.resize( size );
    for(size_t i=0; i<size; i++) {
        mSortIdxMap[i] = i;
    }

    mFilterIdxMap.resize( size );
    mFilterActive.resize( size );
    for(size_t i=0; i<size; i++) {
        mFilterIdxMap[i] = i;
        mFilterActive[i] = false;
    }

    mFilteredRecordSize = size;
    mFilteredPattern = QRegularExpression();
}

} // namespace reference
} // namespace studio
} // namespace gams
