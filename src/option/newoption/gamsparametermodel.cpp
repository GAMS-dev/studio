/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include <QApplication>

#include "gamsparametermodel.h"
#include "theme.h"

namespace gams {
namespace studio {
namespace option {
namespace newoption {

GamsParameterModel::GamsParameterModel(const GamsParameterItem &item, OptionTokenizer *tokenizer, QObject *parent):
    QAbstractTableModel(parent), mOptionItem(item), mOptionTokenizer(tokenizer), mOption(mOptionTokenizer->getOption())
{
    for(int i = 0; i<mOptionItem.size(); ++i) {
        QVariant value =  QVariant(Qt::Unchecked);
        if (mOptionItem.at(i)->disabled)
            value = QVariant(Qt::PartiallyChecked);
        else if (mOptionItem.at(i)->error == OptionErrorType::No_Error)
            value = QVariant(Qt::Unchecked);
        else
            value = QVariant(Qt::Checked);

        mCheckState[i] = value;
    }
}

QVariant GamsParameterModel::headerData(int index, Qt::Orientation orientation, int role) const
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
    case Qt::ToolTipRole: {
        const QString lineComment = mOption->isEOLCharDefined() ? QString(mOption->getEOLChars().at(0)) : QString("*");
        if (mOptionItem.at(index)->disabled) {
            if (mOptionItem.at(index)->key.startsWith(lineComment))
                return QString("%1 %2").arg(mOptionItem.at(index)->key, mOptionItem.at(index)->value);
            else
                return QString("%1 %2 %3").arg(lineComment, mOptionItem.at(index)->key, mOptionItem.at(index)->value);
        } else {
            QString tooltipText = "";
            switch(mOptionItem.at(index)->error) {
            case OptionErrorType::Invalid_Key:
                tooltipText.append( QString("Unknown option '%1'").arg(mOptionItem.at(index)->key) );
                break;
            case OptionErrorType::Incorrect_Value_Type:
                tooltipText.append( QString("Option key '%1' has a value of incorrect type").arg(mOptionItem.at(index)->key));
                break;
            case OptionErrorType::Value_Out_Of_Range:
                tooltipText.append( QString("Value '%1' for option key '%2' is out of range").arg(mOptionItem.at(index)->value, mOptionItem.at(index)->key));
                break;
            case OptionErrorType::Deprecated_Option:
                tooltipText.append( QString("Option '%1' is deprecated, will be eventually ignored").arg(mOptionItem.at(index)->key));
                break;
            default:
                break;
            }
            if (mOptionItem.at(index)->recurrent) {
                if (!tooltipText.isEmpty())
                    tooltipText.append("\n");
                tooltipText.append( QString("Recurrent option '%1', only last entry of same options will not be ignored").arg(mOptionItem.at(index)->key));
            }
            return tooltipText;
        }
    }
    case Qt::DecorationRole:
        if (Qt::CheckState(mCheckState[index].toUInt())==Qt::Checked) {
            if (mOptionItem.at(index)->recurrent)
                return QVariant::fromValue(Theme::icon(":/img/square-red-yellow"));
            else
                return QVariant::fromValue(Theme::icon(":/img/square-red"));
        } else if (Qt::CheckState(mCheckState[index].toUInt())==Qt::PartiallyChecked) {
            if (mOptionItem.at(index)->recurrent)
                return QVariant::fromValue(Theme::icon(":/img/square-gray-yellow"));
            else
                return QVariant::fromValue(Theme::icon(":/img/square-gray"));
        } else {
            if (mOptionItem.at(index)->recurrent)
                return QVariant::fromValue(Theme::icon(":/img/square-green-yellow"));
            else
                return QVariant::fromValue(Theme::icon(":/img/square-green"));
        }
    }

    return QVariant();
}

int GamsParameterModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return  mOptionItem.size();
}

int GamsParameterModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mHeader.size();
}

QVariant GamsParameterModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    const int col = index.column();

    if (mOptionItem.isEmpty())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        if (col==COLUMN_KEY) {
            return mOptionItem.at(row)->key;
        } else if (col==COLUMN_VALUE) {
                  return mOptionItem.at(row)->value;
        } else if (col==COLUMN_ENTRY) {
                   return QVariant(mOptionItem.at(row)->optionId);
        }
        break;
    }
    case Qt::TextAlignmentRole: {
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    }
    case Qt::ToolTipRole: {
        if (mOptionItem.at(row)->disabled) {
            return mOptionItem.at(row)->key;
        } else {
            QString tooltipText = "";
            switch(mOptionItem.at(row)->error) {
            case OptionErrorType::Invalid_Key:
                tooltipText.append( QString("Unknown option '%1'").arg(mOptionItem.at(row)->key));
                break;
            case OptionErrorType::Incorrect_Value_Type:
                tooltipText.append( QString("Option key '%1' has a value of incorrect type").arg(mOptionItem.at(row)->key) );
                break;
            case OptionErrorType::Value_Out_Of_Range:
                tooltipText.append( QString("Value '%1' for option key '%2' is out of range").arg(mOptionItem.at(row)->value, mOptionItem.at(row)->key) );
                break;
            case OptionErrorType::Deprecated_Option:
                tooltipText.append( QString("Option '%1' is deprecated, will be eventually ignored").arg(mOptionItem.at(row)->key) );
                break;
            case OptionErrorType::UserDefined_Error:
                tooltipText.append( QString("Invalid option key or value or comment defined") );
                break;
            default:
                break;
            }
            if (mOptionItem.at(row)->recurrent) {
                if (!tooltipText.isEmpty())
                    tooltipText.append("\n");
                tooltipText.append( QString("Recurrent option '%1', only last entry of same options will not be ignored").arg(mOptionItem.at(row)->key));
            }
            return tooltipText;
        }
    }
    case Qt::ForegroundRole: {
        if (mOptionItem.at(row)->disabled) {
            return QVariant::fromValue(Theme::color(Theme::Disable_Gray));
        } else {
            switch(mOptionItem.at(row)->error) {
            case OptionErrorType::UserDefined_Error:
            case OptionErrorType::Invalid_Key:
            case OptionErrorType::Incorrect_Value_Type:
            case OptionErrorType::Value_Out_Of_Range:
                return QVariant::fromValue(Theme::color(Theme::Normal_Red));
            case OptionErrorType::Deprecated_Option:
                if (mOptionItem.at(row)->recurrent && index.column()==COLUMN_KEY)
                    return QVariant::fromValue(Theme::color(Theme::Normal_Yellow));
                else return QVariant::fromValue(Theme::color(Theme::Disable_Gray));
            case OptionErrorType::No_Error:
                if (mOptionItem.at(row)->recurrent && index.column()==COLUMN_KEY)
                    return QVariant::fromValue(Theme::color(Theme::Normal_Yellow));
                else
                    return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
            default:
                if (mOptionItem.at(row)->recurrent && index.column()==COLUMN_KEY)
                    return QVariant::fromValue(Theme::color(Theme::Normal_Yellow));
                else
                    return QVariant::fromValue(QApplication::palette().color(QPalette::Text));
            }
        }
    }
    default:
        break;
    }
    return QVariant();

}

bool GamsParameterModel::setHeaderData(int index, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation != Qt::Vertical || role != Qt::CheckStateRole)
        return false;

    mCheckState[index] = value;
    mOptionItem.at(index)->disabled = (value.toInt()==Qt::CheckState(Qt::PartiallyChecked));

    emit headerDataChanged(orientation, index, index);
    return true;
}

bool GamsParameterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() > mOptionItem.size())
        return false;

    QVector<int> roles;
    if (role == Qt::EditRole)   {
        roles = { Qt::EditRole };
        const QString dataValue = value.toString();
        if (index.column()==COLUMN_KEY) {
            if (mOptionItem[index.row()]->disabled) {
                const QString lineComment = mOption->isEOLCharDefined() ? QString(mOption->getEOLChars().at(0)) : QString("*");
                if (!dataValue.startsWith(lineComment))
                    mOptionItem[index.row()]->key = QString("%1 %2").arg(lineComment, dataValue);
                else
                    mOptionItem[index.row()]->key = dataValue;
            } else {
                mOptionItem[index.row()]->key = dataValue;
            }

        } else if (index.column()==COLUMN_VALUE) {
            mOptionItem[index.row()]->value = dataValue;
        } else if (index.column()==COLUMN_ENTRY) {
                    mOptionItem[index.row()]->optionId = dataValue.toInt();
        }
        emit dataChanged(index, index, roles);
    } else if (role == Qt::CheckStateRole) {
        roles = { Qt::CheckStateRole };
        mOptionItem[index.row()]->disabled = (Qt::CheckState(value.toUInt())==Qt::PartiallyChecked);
        mCheckState[index.row()] = value;
        emit dataChanged(index, index, roles);
    }
    return true;
}

QModelIndex GamsParameterModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return QAbstractTableModel::createIndex(row, column);
    return QModelIndex();
}

} // namepsace newoption
} // namepsace option
} // namespace studio
} // namespace gams
