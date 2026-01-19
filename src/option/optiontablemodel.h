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
#ifndef OPTIONTABLEMODEL_H
#define OPTIONTABLEMODEL_H

#include <QAbstractTableModel>

#include "option/optiontokenizer.h"
#include "theme.h"
#include "msgbox.h"

namespace gams {
namespace studio {
namespace option {

struct OptionItem;
struct SolverOptionItem;
class ConfigItem;

class OptionTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    OptionTableModel(const QString& callstr, const bool allowLineComment,
                     OptionTokenizer* tokenizer, QObject *parent = nullptr) :
        QAbstractTableModel(parent),
        mCallstr(callstr),
        isLineCommentAllowed(allowLineComment),
        mOptionTokenizer(tokenizer)
    { }

    Qt::ItemFlags flags(const QModelIndex &index) const override {
        Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
        if (!index.isValid())
            return Qt::NoItemFlags | Qt::ItemIsDropEnabled ;
        else
            return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    }

    static const int COLUMN_ID    = 0;
    static const int COLUMN_KEY   = 1;
    static const int COLUMN_VALUE = 2;

    inline static int column_id()    { return COLUMN_ID;    }
    inline static int column_key()   { return COLUMN_KEY;   }
    inline static int column_value() { return COLUMN_VALUE; }

    inline QStringList headers()  { return mHeader; }

    QVariant headerTooltip(const bool disabled, const QString& EOLChar,  const bool recurrent,
                           const OptionErrorType& type, const QString& key, const QString& value,
                           const QString minVersion="", const QString maxVersion="") const {
        QString tooltipText = "";
        if (isLineCommentAllowed && disabled) {
            if (key.startsWith(EOLChar))
                return QString("%1 %2").arg(key, value);
            else
                return QString("%1 %2 %3").arg(EOLChar, key, value);
        } else {
            if (!tooltipText.isEmpty())
                tooltipText.append("\n");
            switch(type) {
            case OptionErrorType::Invalid_Key:
                tooltipText.append( QString("Unknown %1 '%2'").arg(mCallstr, key) );
                break;
            case OptionErrorType::Incorrect_Value_Type:
                tooltipText.append( QString("%1 '%2' has a value of incorrect type").arg(mCallstr, key));
                break;
            case OptionErrorType::Value_Out_Of_Range:
                tooltipText.append( QString("Value '%1' for option key '%2' is out of range").arg(value, key));
                break;
            case OptionErrorType::Deprecated_Option:
                tooltipText.append( QString("%1 '%2' is deprecated, will be eventually ignored").arg(mCallstr, key));
                break;
            case OptionErrorType::Invalid_minVersion:
                tooltipText.append( QString("Invalid minVersion '%1', must be  conformed to [xx[.y[.z]]] format").arg(minVersion) );
                break;
            case OptionErrorType::Invalid_maxVersion:
                tooltipText.append( QString("Invalid maxVersion '%1', must be  conformed to [xx[.y[.z]]] format").arg(maxVersion) );
                break;

            default:
                break;
            }
            if (recurrent) {
                if (!tooltipText.isEmpty())
                    tooltipText.append("\n");
                tooltipText.append( QString("Recurrent %1 '%2', only last entry of same parameters will not be ignored").arg(mCallstr, key));
            }
        }
        return QVariant(tooltipText);
    };

    QVariant headerDecoration(const uint checkstate, const bool recurrent) const {
        if (Qt::CheckState(checkstate)==Qt::Checked) {
            if (recurrent)
                return QVariant::fromValue(Theme::icon(":/img/square-red-yellow"));
            else
                return QVariant::fromValue(Theme::icon(":/img/square-red"));
        } else if (Qt::CheckState(checkstate)==Qt::PartiallyChecked) {
            if (recurrent)
                return QVariant::fromValue(Theme::icon(":/img/square-gray-yellow"));
            else
                return QVariant::fromValue(Theme::icon(":/img/square-gray"));
        } else {
            if (recurrent)
                return QVariant::fromValue(Theme::icon(":/img/square-green-yellow"));
            else
                return QVariant::fromValue(Theme::icon(":/img/square-green"));
        }
    };

    QVariant dataTooltip(const bool disabled, const bool recurrent,
                         const OptionErrorType& type, const QString& key, const QString& value
                        ) const {
        if (disabled) {
            return QVariant(key);
        } else if (value.isEmpty()) {
                return QVariant( QString("Missing value for %2 '%1' ").arg(key, mCallstr.toLower()) );
        } else {
            QString tooltipText = "";
            switch(type) {
            case OptionErrorType::Missing_Value:
                tooltipText.append( QString("Missing value for %2  '%1' ").arg(key, mCallstr.toLower()) );
                break;
            case OptionErrorType::Invalid_Key:
                tooltipText.append( QString("Unknown %1 '%2'").arg(mCallstr.toLower(), key));
                break;
            case OptionErrorType::Incorrect_Value_Type:
                tooltipText.append( QString("%1 '%2' has a value of incorrect type").arg(mCallstr).arg(key) );
                break;
            case OptionErrorType::Value_Out_Of_Range:
                tooltipText.append( QString("Value '%1' for %3 key '%2' is out of range").arg(value, key, mCallstr.toLower()) );
                break;
            case OptionErrorType::Deprecated_Option:
                tooltipText.append( QString("%1 '%2' is deprecated, will be ignored when run").arg(mCallstr).arg(key) );
                break;
            case OptionErrorType::UserDefined_Error:
                tooltipText.append( QString("Invalid %1 key or value or comment defined").arg(mCallstr.toLower()) );
                break;
            case OptionErrorType::Invalid_minVersion:
                tooltipText.append( QString("Invalid minVersion format, must be [xx[.y[.z]]") );
                break;
            case OptionErrorType::Invalid_maxVersion:
                tooltipText.append( QString("Invalid maxVersion format, must be [xx[.y[.z]]") );
                break;
            default:
                break;
            }
            if (recurrent) {
                if (!tooltipText.isEmpty())
                    tooltipText.append("\n");
                tooltipText.append( QString("Recurrent %1 '%2', only last entry value of '%2' will not be ignored when run").arg(mCallstr.toLower(), key));
            }
            return QVariant(tooltipText);
        }
    };

    int questionEntryExisted(QList<int> &overrideIdRowList) {
        Q_ASSERT(overrideIdRowList.size() > 0);
        if (overrideIdRowList.size() == 1) { // singleEntryExisted
            const QString option = data(index(overrideIdRowList.at(0), COLUMN_KEY)).toString();
            const QString detailText = QString("Entry:  '%1'\nDescription:  %2 %3")
                                                 .arg(getOptionTableEntry(overrideIdRowList.at(0)),
                "When a file contains multiple entries of the same "+mCallstr.toLower()+", only the value of the last entry will be utilized.",
                "The value of all other entries except the last entry will be ignored.");
            return MsgBox::question(mCallstr+" Entry exists", mCallstr+" '" + option + "' already exists.",
                                     "How do you want to proceed?", detailText,
                                     nullptr, "Replace existing entry", "Add new entry", "Abort", 2, 2);
        } else if (overrideIdRowList.size() > 1) { //  multipleEntryExisted
            QString option = data(index(overrideIdRowList.at(0), COLUMN_KEY)).toString();
            QString entryDetailedText = QString("Entries:\n");
            int i = 0;
            for (const int id : overrideIdRowList)
                entryDetailedText.append(QString("   %1. '%2'\n").arg(++i).arg(getOptionTableEntry(id)));
            const QString detailText = QString("%1Description:  %2 %3").arg(entryDetailedText,
                                                                            "When a file contains multiple entries of the same "+mCallstr.toLower()+", only the value of the last entry will be utilized.",
                                                                            "The value of all other entries except the last entry will be ignored.");
            return MsgBox::question("Multiple "+mCallstr+" entries exist",
                                    "Multiple entries of "+mCallstr+" '" + option + "' already exist.",
                                    "How do you want to proceed?", detailText, nullptr,
                                    "Replace first entry and delete other entries", "Add new entry", "Abort", 2, 2);
        } else {
            return -1;
        }
    };

signals:
    void newTableRowDropped(const QModelIndex &index);
    void optionItemRemoved();

    void optionModelChanged(const QList<OptionItem*> &optionItem);
    void optionNameChanged(const QString &from, const QString &to);
    void optionValueChanged(const QModelIndex &index);

public slots:
    virtual void on_groupDefinitionReloaded() = 0;
    virtual QString getOptionTableEntry(int row) =0;

    virtual void on_updateOptionItem(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) = 0;
    virtual void on_removeOptionItem() = 0;

//    Qt::CheckState checkState(int index) const { return mError[index]; }
//    QList<int> recurrent(int index) const { return (index < mRecurrence.size() ? mRecurrence[index] : QList<int>()); }

protected:
    QString mCallstr;
    bool isLineCommentAllowed;

    OptionTokenizer* mOptionTokenizer;
    QStringList mHeader;
    QMap<int, QVariant> mCheckState;

    QList<Qt::CheckState> mError;
    QList<QList<int>> mRecurrence;
};

} // namepsace option
} // namespace studio
} // namespace gams

#endif // OPTIONTABLEMODEL_H
