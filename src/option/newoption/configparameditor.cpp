/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "option/configoptiondefinitionmodel.h"
#include "option/gamsuserconfig.h"
#include "option/optiondefinitionmodel.h"
#include "option/newoption/configparameditor.h"
#include "msgbox.h"
#include "ui_optionwidget.h"

namespace gams {
namespace studio {
namespace option {
namespace newoption {

ConfigParamEditor::ConfigParamEditor(const QList<ConfigItem *> &initParamItems, const QString &encodingName, QWidget *parent) :
    OptionWidget(true, parent),
    mEncoding(encodingName.isEmpty() ? "UTF-8" : encodingName),
    mModified(false),
    mFileHasChangedExtern(false)
{
    mOptionTokenizer = new OptionTokenizer(GamsOptDefFile);

    QList<ParamConfigItem *> optionItem;
    optionItem.reserve(initParamItems.size());
    for(ConfigItem* item: initParamItems) {
        optionItem.append( new ParamConfigItem(-1, item->key, item->value, item->minVersion, item->maxVersion) );
    }
    mParameterTableModel = new ConfigTableModel(optionItem, mOptionTokenizer, this);

    mOptionCompleter = new OptionItemDelegate(optionTokenizer(), ui->optionTableView);
    mDefinitionModel = new ConfigOptionDefinitionModel(mOptionTokenizer->getOption(), 0, this);

    initToolBar();
    initActions();
    initTableView();
    initTreeView();
    initTabNavigation( false );
    initMessageControl( false );

    connect(ui->optionTableView->verticalHeader(), &QHeaderView::sectionClicked, this, &ConfigParamEditor::on_selectRow, Qt::UniqueConnection);
    connect(ui->optionTableView, &QTableView::customContextMenuRequested,this, &ConfigParamEditor::showOptionContextMenu, Qt::UniqueConnection);
    connect(ui->optionTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ConfigParamEditor::selectionChanged);
    connect(mParameterTableModel, &ConfigTableModel::newTableRowDropped, this, &ConfigParamEditor::on_newTableRowDropped, Qt::UniqueConnection);

    connect(ui->definitionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ConfigParamEditor::findAndSelectionOptionFromDefinition, Qt::UniqueConnection);
    connect(ui->definitionTreeView, &QTreeView::customContextMenuRequested, this, &ConfigParamEditor::showDefinitionContextMenu, Qt::UniqueConnection);
    connect(ui->definitionTreeView, &QAbstractItemView::doubleClicked, this, &ConfigParamEditor::addOptionFromDefinition, Qt::UniqueConnection);

    connect(ui->definitionGroup, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int index) {
        mDefinitionModel->loadOptionFromGroup( mDefinitionGroupModel->data(mDefinitionGroupModel->index(index, 1)).toInt() );
        mParameterTableModel->on_groupDefinitionReloaded();
    });
    connect(mParameterTableModel, &QAbstractTableModel::dataChanged, this, &ConfigParamEditor::on_dataItemChanged, Qt::UniqueConnection);
    connect(mParameterTableModel, &QAbstractTableModel::dataChanged, mParameterTableModel, &ConfigTableModel::on_updateConfigParamItem, Qt::UniqueConnection);
    connect(mParameterTableModel, &ConfigTableModel::configParamModelChanged, mDefinitionModel, &ConfigOptionDefinitionModel::modifyOptionDefinition, Qt::UniqueConnection);
    connect(mParameterTableModel, &ConfigTableModel::optionItemRemoved, mParameterTableModel, &ConfigTableModel::on_removeConfigParamItem, Qt::UniqueConnection);

    connect(this, &ConfigParamEditor::modificationChanged, this, &ConfigParamEditor::setModified, Qt::UniqueConnection);
    emit mParameterTableModel->configParamModelChanged(optionItem);

    QTimer::singleShot(0, this, [this]() {
        mParameterTableModel->updateRecurrentStatus();
        ui->optionTableView->horizontalHeader()->setSectionResizeMode(ConfigTableModel::COLUMN_KEY, QHeaderView::Interactive);
        ui->optionTableView->horizontalHeader()->setSectionResizeMode(ConfigTableModel::COLUMN_VALUE, QHeaderView::Interactive);
    });

}

ConfigParamEditor::~ConfigParamEditor()
{
    if (mOptionTokenizer)
        delete mOptionTokenizer;
    if (mOptionCompleter)
        delete mOptionCompleter;
    if (mDefinitionGroupModel)
        delete mDefinitionGroupModel;
    if (mDefinitionModel)
        delete mDefinitionModel;
    if (mDefinitionGroupModel)
        delete mDefinitionProxymodel;
    if (mParameterTableModel)
        delete mParameterTableModel;
}

bool ConfigParamEditor::isInFocus(QWidget *focusWidget) const
{
    return (focusWidget==ui->optionTableView || focusWidget==ui->definitionTreeView || focusWidget==ui->definitionSearch    );
}

QList<QHeaderView *> ConfigParamEditor::headers()
{
    return QList<QHeaderView *>() << ui->optionTableView->horizontalHeader()
                                  << ui->definitionTreeView->header()
                                  << ui->optionTableView->verticalHeader();

}

QString ConfigParamEditor::getSelectedParameterName(QWidget *widget) const
{
    if (widget == ui->optionTableView) {
        QModelIndexList selection = ui->optionTableView->selectionModel()->selectedIndexes();
        if (selection.count() > 0) {
            const QModelIndex index = selection.at(0);
            const QVariant headerData = ui->optionTableView->model()->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole);
            if (Qt::CheckState(headerData.toUInt())==Qt::PartiallyChecked) {
                return "";
            }
            const QVariant data = ui->optionTableView->model()->data( index.sibling(index.row(),0) );
            if (mOptionTokenizer->getOption()->isValid(data.toString()))
                return data.toString();
            else if (mOptionTokenizer->getOption()->isASynonym(data.toString()))
                return mOptionTokenizer->getOption()->getNameFromSynonym(data.toString());
            else
                return "";
        }
    } else if (widget == ui->definitionTreeView) {
        const QModelIndexList selection = ui->definitionTreeView->selectionModel()->selectedRows();
        if (selection.count() > 0) {
            const QModelIndex index = selection.at(0);
            const QModelIndex  parentIndex =  ui->definitionTreeView->model()->parent(index);
            if (parentIndex.row() >= 0) {
                return ui->definitionTreeView->model()->data( parentIndex.sibling(parentIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ).toString();
            } else {
                return ui->definitionTreeView->model()->data( index.sibling(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ).toString();
            }
        }
    }
    return "";
}

void ConfigParamEditor::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)
    if (selected.isEmpty()) {
        updateActionsState();
        return;
    }

    updateActionsState(selected.indexes().first());
}

void ConfigParamEditor::addOptionFromDefinition(const QModelIndex &index)
{
    emit modificationChanged(true);

    const QModelIndex parentIndex =  ui->definitionTreeView->model()->parent(index);
    const QModelIndex optionNameIndex = (parentIndex.row()<0) ? ui->definitionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME)
                                                                : ui->definitionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ;
    const QModelIndex defValueIndex = (parentIndex.row()<0) ? ui->definitionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_DEF_VALUE)
                                                              : ui->definitionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_DEF_VALUE) ;
    const QModelIndex selectedValueIndex = (parentIndex.row()<0) ? defValueIndex
                                                                   : ui->definitionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex) ;

    disconnect( mParameterTableModel, &QAbstractTableModel::dataChanged,  mParameterTableModel, &ConfigTableModel::on_updateConfigParamItem);

    bool replaceExistingEntry = false;
    const QString optionNameData = ui->definitionTreeView->model()->data(optionNameIndex).toString();
    const QModelIndex optionIdIndex = (parentIndex.row()<0) ? ui->definitionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER)
                                                              : ui->definitionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER) ;
    const QVariant optionIdData = ui->definitionTreeView->model()->data(optionIdIndex);

    int rowToBeAdded = ui->optionTableView->model()->rowCount();
    //    StudioSettings* settings = SettingsLocator::settings();
    //    if (settings && settings->overridExistingOption()) {
    QModelIndexList indices = ui->optionTableView->model()->match(ui->optionTableView->model()->index(0, ConfigTableModel::COLUMN_ID),
                                                                    Qt::DisplayRole,
                                                                    optionIdData, -1, Qt::MatchExactly|Qt::MatchRecursive);
    ui->optionTableView->clearSelection();
    QItemSelection selection;
    for(const QModelIndex &idx: std::as_const(indices)) {
        const QModelIndex leftIndex  = ui->optionTableView->model()->index(idx.row(), ConfigTableModel::COLUMN_KEY);
        const QModelIndex rightIndex = ui->optionTableView->model()->index(idx.row(), ConfigTableModel::COLUMN_ID);
        const QItemSelection rowSelection(leftIndex, rightIndex);
        selection.merge(rowSelection, QItemSelectionModel::Select);
    }
    ui->optionTableView->selectionModel()->select(selection, QItemSelectionModel::Select);

    const bool singleEntryExisted = (indices.size()==1);
    const bool multipleEntryExisted = (indices.size()>1);
    if (singleEntryExisted ) {
        const QString detailText = QString("Entry:  '%1'\nDescription:  %2 %3")
        .arg(getParameterTableEntry(indices.at(0).row()),
             "When a GAMS config file contains multiple entries of the same parameter, only the value of the last entry will be utilized by GAMS.",
             "The value of all other entries except the last entry will be ignored.");
        const int answer = MsgBox::question("Parameter Entry exists", "Parameter '" + optionNameData + "' already exists.",
                                            "How do you want to proceed?", detailText,
                                            nullptr, "Replace existing entry", "Add new entry", "Abort", 2, 2);
        switch(answer) {
        case 0: // replace
            replaceExistingEntry = true;
            indices = ui->optionTableView->model()->match(ui->optionTableView->model()->index(0, ConfigTableModel::COLUMN_ID),
                                                            Qt::DisplayRole,
                                                            optionIdData, -1, Qt::MatchExactly|Qt::MatchRecursive);
            rowToBeAdded = (indices.size()>0) ? indices.at(0).row() : 0;
            break;
        case 1: // add
            break;
        default:
            return;
        }
    } else if (multipleEntryExisted) {
        QString entryDetailedText = QString("Entries:\n");
        int i = 0;
        for (const QModelIndex &idx : std::as_const(indices))
            entryDetailedText.append(QString("   %1. '%2'\n").arg(++i).arg(getParameterTableEntry(idx.row())));
        const QString detailText = QString("%1Description:  %2 %3").arg(entryDetailedText,
                                                                        "When a GAMS config file contains multiple entries of the same parameter, only the value of the last entry will be utilized by GAMS.",
                                                                        "The value of all other entries except the last entry will be ignored.");
        int answer = MsgBox::question("Multiple Parameter Entries exist",
                                      "Multiple entries of Parameter '" + optionNameData + "' already exist.",
                                      "How do you want to proceed?", detailText, nullptr,
                                      "Replace first entry and delete other entries", "Add new entry", "Abort", 2, 2);
        switch(answer) {
        case 0: // delete and replace
            disconnect( mParameterTableModel, &ConfigTableModel::optionItemRemoved, mParameterTableModel, &ConfigTableModel::on_removeConfigParamItem);
            ui->optionTableView->selectionModel()->clearSelection();
            for(int i=1; i<indices.size(); i++) {
                ui->optionTableView->selectionModel()->select( indices.at(i), QItemSelectionModel::Select|QItemSelectionModel::Rows );
            }
            deleteOption();
            connect( mParameterTableModel, &ConfigTableModel::optionItemRemoved,
                    mParameterTableModel, &ConfigTableModel::on_removeConfigParamItem, Qt::UniqueConnection);
            replaceExistingEntry = true;
            indices = ui->optionTableView->model()->match(ui->optionTableView->model()->index(0, ConfigTableModel::COLUMN_ID),
                                                            Qt::DisplayRole,
                                                            optionIdData, -1, Qt::MatchExactly|Qt::MatchRecursive);
            rowToBeAdded = (indices.size()>0) ? indices.at(0).row() : 0;
            break;
        case 1: // add
            break;
        default:
            return;
        }

    } // else entry not exist
    //    }

    ui->optionTableView->selectionModel()->clearSelection();
    //    QString synonymData = ui->definitionTreeView->model()->data(synonymIndex).toString();
    const QString selectedValueData = ui->definitionTreeView->model()->data(selectedValueIndex).toString();
    mOptionTokenizer->getOption()->setModified(optionNameData, true);
    ui->definitionTreeView->model()->setData(optionNameIndex, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);

    // insert option row
    if (rowToBeAdded == ui->optionTableView->model()->rowCount()) {
        ui->optionTableView->model()->insertRows(rowToBeAdded, 1, QModelIndex());
    }
    const QModelIndex insertKeyIndex = ui->optionTableView->model()->index(rowToBeAdded, ConfigTableModel::COLUMN_KEY);
    const QModelIndex insertValueIndex = ui->optionTableView->model()->index(rowToBeAdded, ConfigTableModel::COLUMN_VALUE);
    const QModelIndex minVersionIndex = ui->optionTableView->model()->index(rowToBeAdded, ConfigTableModel::COLUMN_MIN_VERSION);
    const QModelIndex maxVersionIndex = ui->optionTableView->model()->index(rowToBeAdded, ConfigTableModel::COLUMN_MIN_VERSION);
    ui->optionTableView->model()->setData( insertKeyIndex, optionNameData, Qt::EditRole);
    ui->optionTableView->model()->setData( insertValueIndex, (selectedValueData.simplified().isEmpty()
                                                                   ? OptionTokenizer::valueGeneratedStr
                                                                   : selectedValueData)
                                            , Qt::EditRole);
    ui->optionTableView->model()->setData( minVersionIndex, "", Qt::EditRole);
    ui->optionTableView->model()->setData( maxVersionIndex, "", Qt::EditRole);

    QModelIndex insertNumberIndex = ui->optionTableView->model()->index(rowToBeAdded, ConfigTableModel::COLUMN_ID);
    const int optionEntryNumber = mOptionTokenizer->getOption()->getOptionDefinition(optionNameData).number;
    ui->optionTableView->model()->setData( insertNumberIndex, optionEntryNumber, Qt::EditRole);
    if (selectedValueData.isEmpty())
        ui->optionTableView->model()->setHeaderData( rowToBeAdded, Qt::Vertical, Qt::CheckState(Qt::Checked), Qt::CheckStateRole );
    else
        ui->optionTableView->model()->setHeaderData( rowToBeAdded, Qt::Vertical, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole );
    ui->optionTableView->selectRow(rowToBeAdded);
    selectAnOption();

    const QString text =  mParameterTableModel->getParameterTableEntry(insertNumberIndex.row());
    if (replaceExistingEntry)
        mOptionTokenizer->logger()->append(QString("Parameter entry '%1' has been replaced").arg(text), LogMsgType::Info);
    else
        mOptionTokenizer->logger()->append(QString("Parameter entry '%1' has been added").arg(text), LogMsgType::Info);

    const int lastColumn = ui->optionTableView->model()->columnCount()-1;
    const int lastRow = rowToBeAdded;
    int firstRow = lastRow;
    if (firstRow<0)
        firstRow = 0;
    mParameterTableModel->on_updateConfigParamItem( ui->optionTableView->model()->index(firstRow, lastColumn),
                                                   ui->optionTableView->model()->index(lastRow, lastColumn),
                                                   {Qt::EditRole});

    ui->optionTableView->resizeColumnToContents(ConfigTableModel::COLUMN_KEY);
    ui->optionTableView->resizeColumnToContents(ConfigTableModel::COLUMN_VALUE);

    connect( mParameterTableModel, &QAbstractTableModel::dataChanged,  mParameterTableModel, &ConfigTableModel::on_updateConfigParamItem, Qt::UniqueConnection);

    showOptionDefinition(true);

    if (parentIndex.row()<0) {
        if (mOptionTokenizer->getOption()->getOptionSubType(optionNameData) != optsubNoValue)
            ui->optionTableView->edit(insertValueIndex);
    }

}

void ConfigParamEditor::parameterItemCommitted(QWidget *editor)
{
    Q_UNUSED(editor)
    if (mOptionCompleter->currentEditedIndex().isValid()) {
        ui->optionTableView->resizeColumnToContents( mOptionCompleter->currentEditedIndex().column() );
        ui->optionTableView->selectionModel()->select( mOptionCompleter->currentEditedIndex(), QItemSelectionModel::ClearAndSelect );
        ui->optionTableView->setCurrentIndex( mOptionCompleter->currentEditedIndex() );
        ui->optionTableView->setFocus();
    }
}

QList<ConfigItem *> ConfigParamEditor::parameterConfigItems()
{
    QList<ConfigItem *> itemList;
    itemList.reserve(mParameterTableModel->parameterConfigItems().size());
    for(ParamConfigItem* item : mParameterTableModel->parameterConfigItems()) {
        itemList.append( new ConfigItem(item->key, item->value, item->minVersion, item->maxVersion) );
    }
    return itemList;
}

void ConfigParamEditor::on_reloadGamsUserConfigFile(const QList<ConfigItem *> &initParams)
{
    QList<ParamConfigItem *> optionItem;
    optionItem.reserve(initParams.size());
    for(ConfigItem* item: initParams) {
        optionItem.append( new ParamConfigItem(-1, item->key, item->value, item->minVersion, item->maxVersion) );
    }
    mParameterTableModel->on_reloadConfigParamModel(optionItem);
    setModified(false);
}

//QList<ParamConfigItem *> ConfigParamEditor::parameterConfigItems()
//{
//    QList<ConfigItem *> itemList;
//    itemList.reserve(mParameterTableModel->parameterConfigItems().size());
//    for(ParamConfigItem* item : mParameterTableModel->parameterConfigItems()) {
//        itemList.append( new ConfigItem(item->key, item->value, item->minVersion, item->maxVersion) );
//    }
//    return itemList;
//}

void ConfigParamEditor::on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)
    emit modificationChanged(true);

    QModelIndexList toDefinitionItems = ui->definitionTreeView->model()->match(ui->definitionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                                Qt::DisplayRole,
                                                                                ui->optionTableView->model()->data( topLeft, Qt::DisplayRole), 1);
    if (toDefinitionItems.size() <= 0) {
        toDefinitionItems = ui->definitionTreeView->model()->match(ui->definitionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_SYNONYM),
                                                                    Qt::DisplayRole,
                                                                    ui->optionTableView->model()->data( topLeft, Qt::DisplayRole), 1);
    }

    ui->definitionTreeView->clearSelection();
    for(const QModelIndex item : std::as_const(toDefinitionItems)) {
        ui->definitionTreeView->selectionModel()->select(
            QItemSelection (
                ui->definitionTreeView->model ()->index (item.row() , 0),
                ui->definitionTreeView->model ()->index (item.row(), ui->definitionTreeView->model ()->columnCount () - 1)),
            QItemSelectionModel::Select);
        ui->definitionTreeView->scrollTo(toDefinitionItems.first(), QAbstractItemView::EnsureVisible);
    }
    ui->optionTableView->selectionModel()->select(topLeft, QItemSelectionModel::Select);

    updateActionsState(topLeft);

}

void ConfigParamEditor::on_newTableRowDropped(const QModelIndex &index)
{
    disconnect(ui->definitionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
               this, &ConfigParamEditor::findAndSelectionOptionFromDefinition);
    ui->optionTableView->selectRow(index.row());

    const QString optionName = ui->optionTableView->model()->data(index, Qt::DisplayRole).toString();
    QModelIndexList definitionItems = ui->definitionTreeView->model()->match(ui->definitionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                              Qt::DisplayRole,
                                                                              optionName, 1);
    mOptionTokenizer->getOption()->setModified(optionName, true);
    for(const QModelIndex item : std::as_const(definitionItems)) {
        ui->definitionTreeView->model()->setData(item, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);
    }

    ui->optionTableView->selectionModel()->clearSelection();
    ui->optionTableView->resizeColumnToContents(index.column());
    ui->optionTableView->edit( mParameterTableModel->index(index.row(), ConfigTableModel::COLUMN_VALUE));

    showOptionDefinition(false);

    connect(ui->definitionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ConfigParamEditor::findAndSelectionOptionFromDefinition, Qt::UniqueConnection);
}

QString ConfigParamEditor::getParameterTableEntry(int row)
{
    const QModelIndex keyIndex = ui->optionTableView->model()->index(row, ConfigTableModel::COLUMN_KEY);
    const QVariant optionKey = ui->optionTableView->model()->data(keyIndex, Qt::DisplayRole);
    const QModelIndex valueIndex = ui->optionTableView->model()->index(row, ConfigTableModel::COLUMN_VALUE);
    const QVariant optionValue = ui->optionTableView->model()->data(valueIndex, Qt::DisplayRole);
    const QModelIndex minVersionIndex = ui->optionTableView->model()->index(row, ConfigTableModel::COLUMN_MIN_VERSION);
    const QVariant minVersionValue = ui->optionTableView->model()->data(minVersionIndex, Qt::DisplayRole);
    const QModelIndex maxVersionIndex = ui->optionTableView->model()->index(row, ConfigTableModel::COLUMN_MAX_VERSION);
    const QVariant maxVersionValue = ui->optionTableView->model()->data(maxVersionIndex, Qt::DisplayRole);
    return QString("%1%2%3 %4 %5").arg(optionKey.toString(), mOptionTokenizer->getOption()->getDefaultSeparator(),
                                       optionValue.toString(), minVersionValue.toString(), maxVersionValue.toString());
}

void ConfigParamEditor::insertOption()
{
    if (!ui->actionInsert->isEnabled())
        return;

    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex index : std::as_const(indexSelection)) {
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    disconnect(mParameterTableModel, &QAbstractTableModel::dataChanged, mParameterTableModel, &ConfigTableModel::on_updateConfigParamItem);
    int rowToBeInserted = -1;
    if (isThereAnIndexSelection()) {
        QList<int> rows;
        const auto indexes = ui->optionTableView->selectionModel()->selectedIndexes();
        rows.reserve(indexes.size());
        for(const QModelIndex idx : indexes) {
            rows.append( idx.row() );
        }
        std::sort(rows.begin(), rows.end());
        rowToBeInserted = rows.at(0);
        if (rowToBeInserted < ui->optionTableView->model()->rowCount()) {
            QVariant header = ui->optionTableView->model()->headerData(rowToBeInserted, Qt::Vertical, Qt::CheckStateRole);
            ui->optionTableView->model()->insertRows(rowToBeInserted, 1, QModelIndex());
            ui->optionTableView->model()->setHeaderData(rowToBeInserted+1, Qt::Vertical,
                                                          header,
                                                          Qt::CheckStateRole );
        } else {
            ui->optionTableView->model()->insertRows(rowToBeInserted, 1, QModelIndex());
        }
    } else {
        ui->optionTableView->model()->insertRows(ui->optionTableView->model()->rowCount(), 1, QModelIndex());
        rowToBeInserted = mParameterTableModel->rowCount()-1;
    }

    const QModelIndex insertKeyIndex = ui->optionTableView->model()->index(rowToBeInserted, ConfigTableModel::COLUMN_KEY);
    const QModelIndex insertValueIndex = ui->optionTableView->model()->index(rowToBeInserted, ConfigTableModel::COLUMN_VALUE);
    const QModelIndex insertNumberIndex = ui->optionTableView->model()->index(rowToBeInserted, ConfigTableModel::COLUMN_ID);
    const QModelIndex minVersionIndex = ui->optionTableView->model()->index(rowToBeInserted, ConfigTableModel::COLUMN_MIN_VERSION);
    const QModelIndex maxVersionIndex = ui->optionTableView->model()->index(rowToBeInserted, ConfigTableModel::COLUMN_MIN_VERSION);

    ui->optionTableView->model()->setHeaderData(rowToBeInserted, Qt::Vertical,
                                                  Qt::CheckState(Qt::Checked),
                                                  Qt::CheckStateRole );

    ui->optionTableView->model()->setData( insertKeyIndex, OptionTokenizer::keyGeneratedStr, Qt::EditRole);
    ui->optionTableView->model()->setData( insertValueIndex, OptionTokenizer::valueGeneratedStr, Qt::EditRole);
    ui->optionTableView->model()->setData( minVersionIndex, "", Qt::EditRole);
    ui->optionTableView->model()->setData( maxVersionIndex, "", Qt::EditRole);
    ui->optionTableView->scrollTo(insertKeyIndex, QAbstractItemView::EnsureVisible);
    ui->optionTableView->model()->setData( insertNumberIndex, -1, Qt::EditRole);

    connect(mParameterTableModel, &QAbstractTableModel::dataChanged,
            mParameterTableModel, &ConfigTableModel::on_updateConfigParamItem, Qt::UniqueConnection);

    emit modificationChanged(true);

    ui->optionTableView->selectionModel()->clearSelection();
    ui->optionTableView->selectionModel()->select(insertKeyIndex, QItemSelectionModel::ClearAndSelect );

    const QModelIndex index = mParameterTableModel->index(rowToBeInserted, ConfigTableModel::COLUMN_KEY);
    ui->optionTableView->edit( index );
    updateActionsState(index);
}

void ConfigParamEditor::deleteOption()
{
    if (!ui->actionDelete->isEnabled())
        return;

    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex index : std::as_const(indexSelection)) {
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }
    if  (!isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    if (isThereARowSelection() && isEverySelectionARow()) {
        const QItemSelection selection( ui->optionTableView->selectionModel()->selection() );

        QList<int> rows;
        const auto indexes = ui->optionTableView->selectionModel()->selectedRows();
        rows.reserve(indexes.size());
        for(const QModelIndex & index : indexes) {
            rows.append( index.row() );
        }

        std::sort(rows.begin(), rows.end());
        int prev = -1;
        for(int i=rows.count()-1; i>=0; i--) {
            int current = rows[i];
            if (current != prev) {
                const QString text = mParameterTableModel->getParameterTableEntry(current);
                ui->optionTableView->model()->removeRows( current, 1 );
                mOptionTokenizer->logger()->append(QString("Parameter entry '%1' has been deleted").arg(text), LogMsgType::Info);
                prev = current;
            }
        }
        emit modificationChanged(true);
        updateActionsState();
    }
}

void ConfigParamEditor::moveOptionUp()
{
    if (!ui->actionMoveUp->isEnabled())
        return;
    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex index : std::as_const(indexSelection)) {
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  ( !isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    const QModelIndexList selection = ui->optionTableView->selectionModel()->selectedRows();
    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() < b.row(); });
    if  (idxSelection.first().row() <= 0)
        return;

    for(int i=0; i<idxSelection.size(); i++) {
        const QModelIndex idx = idxSelection.at(i);
        mParameterTableModel->moveRows(QModelIndex(), idx.row(), 1,
                                       QModelIndex(), idx.row()-1);
    }

    emit modificationChanged(true);
    updateActionsState();

}

void ConfigParamEditor::moveOptionDown()
{
    if (!ui->actionMoveDown->isEnabled())
        return;
    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex index : std::as_const(indexSelection)) {
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  ( !isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    const QModelIndexList selection = ui->optionTableView->selectionModel()->selectedRows();
    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() < b.row(); });
    if  (idxSelection.first().row() >= mParameterTableModel->rowCount()-1)
        return;

    for(int i=0; i<idxSelection.size(); i++) {
        const QModelIndex idx = idxSelection.at(i);
        mParameterTableModel->moveRows(QModelIndex(), idx.row(), 1,
                                       QModelIndex(), idx.row()+2);
    }
    emit modificationChanged(true);
    updateActionsState( );

}

} // namepsace newoption
} // namepsace option
} // namespace studio
} // namespace gams

