/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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

SymbolTableModel::SymbolTableModel(Reference *ref, SymbolDataType::SymbolType type, QObject *parent) :
     QAbstractTableModel(parent), mType(type), mReference(ref)
{
    mAllSymbolsHeader << "Entry" << "Name" << "Type" << "Dim" << "Domain" << "Text";
    mSymbolsHeader    << "Entry" << "Name"           << "Dim" << "Domain" << "Text";
    mFileHeader       << "Entry" << "Name"           << "Text";
    mFileUsedHeader   << "File Location";
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
           case SymbolDataType::File :
               if (index < mFileHeader.size())
                  return mFileHeader[index];
               break;
           case SymbolDataType::FileUsed :
               if (index < mFileUsedHeader.size())
                  return mFileUsedHeader[index];
               break;
           default:
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

    switch(mType) {
    case SymbolDataType::Set :
    case SymbolDataType::Acronym :
    case SymbolDataType::Parameter :
    case SymbolDataType::Variable :
    case SymbolDataType::Equation :
    case SymbolDataType::Funct :
    case SymbolDataType::Model :
    case SymbolDataType::File :
    case SymbolDataType::Unused :
        return mReference->findReference(mType).size();
    case SymbolDataType::FileUsed :
        return mReference->getFileUsed().size();
    case SymbolDataType::Unknown :
    default:
        return mReference->size();
    }
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
    case SymbolDataType::Model :
        return mFileHeader.size();
    case SymbolDataType::FileUsed :
        return mFileUsedHeader.size();
    default:
        break;
    }
    return 0;

}

QVariant SymbolTableModel::data(const QModelIndex &index, int role) const
{
    if (mReference->isEmpty())
        return QVariant();

    switch (role) {
    case Qt::TextAlignmentRole: {
        if (mType==SymbolDataType::FileUsed)
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        Qt::AlignmentFlag aFlag;
        switch(index.column()) {
            case 0: aFlag = Qt::AlignRight; break;
            case 1: aFlag = Qt::AlignLeft; break;
        case 2: if (mType == SymbolDataType::Unknown || mType == SymbolDataType::Unused ||
                    mType == SymbolDataType::Model || mType == SymbolDataType::Funct || mType == SymbolDataType::File )
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
         QList<SymbolReferenceItem*> refList = mReference->findReference(mType);
         switch(mType) {
         case SymbolDataType::Set :
         case SymbolDataType::Acronym :
         case SymbolDataType::Parameter :
         case SymbolDataType::Variable :
         case SymbolDataType::Equation :
             switch(index.column()) {
             case 0: return QString::number(refList.at(index.row())->id());
             case 1: return refList.at(index.row())->name();
             case 2: return QString::number(refList.at(index.row())->dimension());
             case 3: {
                 QList<SymbolId> dom = refList.at(index.row())->domain();
                 if (dom.size() > 0) {
                    QString domainStr = "(";
                    domainStr.append(  mReference->findReference( dom.at(0) )->name() );
                    for(int i=1; i<dom.size(); i++) {
                        domainStr.append( "," );
                        domainStr.append( mReference->findReference( dom.at(i) )->name() );
                    }
                    domainStr.append( ")" );
                    return domainStr;
                 }
                 break;
             }
             case 4: return refList.at(index.row())->explanatoryText();
             default: break;
             }
             break;
         case SymbolDataType::Unknown :
         case SymbolDataType::Unused :
              switch(index.column()) {
              case 0: return QString::number(refList.at(index.row())->id());
              case 1: return refList.at(index.row())->name();
              case 2: return SymbolDataType::from(refList.at(index.row())->type()).name();
              case 3: return QString::number(refList.at(index.row())->dimension());
              case 4: {
                  QList<SymbolId> dom = refList.at(index.row())->domain();
                  if (dom.size() > 0) {
                     QString domainStr = "(";
                     domainStr.append(  mReference->findReference( dom.at(0) )->name() );
                     for(int i=1; i<dom.size(); i++) {
                         domainStr.append( "," );
                         domainStr.append( mReference->findReference( dom.at(i) )->name() );
                     }
                     domainStr.append( ")" );
                     return domainStr;
                  }
                  break;
              }
              case 5: return refList.at(index.row())->explanatoryText();
              default: break;
              }
              break;
         case SymbolDataType::File :
         case SymbolDataType::Funct :
         case SymbolDataType::Model :
             switch(index.column()) {
             case 0: return QString::number(refList.at(index.row())->id());
             case 1: return refList.at(index.row())->name();;
             case 2: return SymbolDataType::from(refList.at(index.row())->type()).name();
             case 3: return refList.at(index.row())->explanatoryText();
             default: break;
             }
             break;
         case SymbolDataType::FileUsed :
             return mReference->getFileUsed().at(index.row());
         default:
             break;
         }
         break;
    }
    default:
        break;
    }

    return QVariant();
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
    if (rowCount() > 0) {
        removeRows(0, rowCount(), QModelIndex());
    }
    endResetModel();
}

} // namespace reference
} // namespace studio
} // namespace gams
