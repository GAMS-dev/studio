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
#include "option/configoptiondefinitionmodel.h"
#include "option/gamsuserconfig.h"
#include "option/optiondefinitionmodel.h"
#include "option/configparameditor.h"
#include "ui_optionwidget.h"
#include "editors/sysloglocator.h"

namespace gams {
namespace studio {
namespace option {

ConfigParamEditor::ConfigParamEditor(FileKind kind,
                                     const QList<ConfigItem *> &initParamItems,
                                     const QString &encodingName,
                                     QWidget *parent) :
    OptionWidget(true, kind, parent),
    mEncoding(encodingName.isEmpty() ? "UTF-8" : encodingName),
    mModified(false),
    mFileHasChangedExtern(false)
{
    mOptionTokenizer = new OptionTokenizer(GamsOptDefFile);
    if (!mOptionTokenizer->getOption()->available())
        SysLogLocator::systemLog()->append(QString("Missing a library from GAMS installation, GAMS User Configuration editor might not function properly. Please check your GAMS installation."),
                                           LogMsgType::Error);

    QList<ParamConfigItem *> optionItem;
    optionItem.reserve(initParamItems.size());
    for(ConfigItem* item: initParamItems) {
        optionItem.append( new ParamConfigItem(-1, item->key, item->value, item->minVersion, item->maxVersion) );
    }
    mParameterTableModel = new ConfigParamTableModel(callstr(),
                                                     optionItem, mOptionTokenizer, this);

    mDefinitionModel = new ConfigOptionDefinitionModel(callstr(),
                                                       mOptionTokenizer->getOption(), 0, this);

    initActions();
    initToolBar();
    initOptionTableView();
    initDefinitionTreeView();
    initTabNavigation( false );
    initMessageControl( false );

    connect(ui->optionTableView->verticalHeader(), &QHeaderView::sectionClicked, this, &ConfigParamEditor::on_selectRow, Qt::UniqueConnection);

    connect(mParameterTableModel, &QAbstractTableModel::dataChanged, this, &ConfigParamEditor::on_dataItemChanged, Qt::UniqueConnection);
    connect(mParameterTableModel, &QAbstractTableModel::dataChanged, mParameterTableModel, &ConfigParamTableModel::on_updateOptionItem, Qt::UniqueConnection);
    connect(mParameterTableModel, &ConfigParamTableModel::configParamModelChanged, mDefinitionModel, &ConfigOptionDefinitionModel::modifyOptionDefinition, Qt::UniqueConnection);
    connect(mParameterTableModel, &ConfigParamTableModel::optionItemRemoved, mParameterTableModel, &ConfigParamTableModel::on_removeOptionItem, Qt::UniqueConnection);

    emit mParameterTableModel->configParamModelChanged(optionItem);

    QTimer::singleShot(0, this, [this]() {
        mParameterTableModel->updateRecurrentStatus();
        ui->optionTableView->horizontalHeader()->setSectionResizeMode(ConfigParamTableModel::COLUMN_KEY, QHeaderView::Interactive);
        ui->optionTableView->horizontalHeader()->setSectionResizeMode(ConfigParamTableModel::COLUMN_VALUE, QHeaderView::Interactive);
    });

}

ConfigParamEditor::~ConfigParamEditor()
{
    if (mOptionTokenizer)
        delete mOptionTokenizer;
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

void ConfigParamEditor::on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)
    setModified(true);

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

void ConfigParamEditor::insertOption()
{
    if (!ui->actionInsert->isEnabled())
        return;

    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex index : std::as_const(indexSelection)) {
        if (!mOptionCompleter->isLastEditorClosed() && mOptionCompleter->currentEditedIndex().row()==index.row())
            return;
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    disconnect(mParameterTableModel, &QAbstractTableModel::dataChanged, mParameterTableModel, &ConfigParamTableModel::on_updateOptionItem);
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

    const QModelIndex insertKeyIndex = ui->optionTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_KEY);
    const QModelIndex insertValueIndex = ui->optionTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_VALUE);
    const QModelIndex insertNumberIndex = ui->optionTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_ID);
    const QModelIndex minVersionIndex = ui->optionTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_MIN_VERSION);
    const QModelIndex maxVersionIndex = ui->optionTableView->model()->index(rowToBeInserted, ConfigParamTableModel::COLUMN_MIN_VERSION);

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
            mParameterTableModel, &ConfigParamTableModel::on_updateOptionItem, Qt::UniqueConnection);

    setModified(true);

    ui->definitionTreeView->clearSelection();
    ui->optionTableView->selectionModel()->clearSelection();

    const QModelIndex index = mParameterTableModel->index(rowToBeInserted, ConfigParamTableModel::COLUMN_KEY);
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
                const QString text = mParameterTableModel->getOptionTableEntry(current);
                ui->optionTableView->model()->removeRows( current, 1 );
                mOptionTokenizer->logger()->append(QString("Parameter entry '%1' has been deleted").arg(text), LogMsgType::Info);
                prev = current;
            }
        }
        setModified(true);
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

    setModified(true);
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
    setModified(true);
    updateActionsState( );

}

void ConfigParamEditor::addOptionModelFromDefinition(int row, const QModelIndex &index,
                                                              const QModelIndex &parentIndex)
{
    if (row == mParameterTableModel->rowCount()) {
        mParameterTableModel->insertRows(row, 1, QModelIndex());
    }

    const QModelIndex optionNameIndex   = (parentIndex.row()<0) ? index.siblingAtColumn(OptionDefinitionModel::COLUMN_OPTION_NAME)
                                                                : parentIndex.siblingAtColumn(OptionDefinitionModel::COLUMN_OPTION_NAME) ;
    const QModelIndex defValueIndex      = (parentIndex.row()<0) ? index.siblingAtColumn(OptionDefinitionModel::COLUMN_DEF_VALUE)
                                                                 : parentIndex.siblingAtColumn(OptionDefinitionModel::COLUMN_DEF_VALUE);
    const QModelIndex selectedValueIndex = (parentIndex.row()<0) ? defValueIndex
                                                                 : index.siblingAtColumn(OptionDefinitionModel::COLUMN_OPTION_NAME) ;

    const QString selectedValueData  = ui->definitionTreeView->model()->data(selectedValueIndex, Qt::DisplayRole).toString();
    const QString optionNameData     = ui->definitionTreeView->model()->data(optionNameIndex, Qt::DisplayRole).toString();

    const int optionEntryNumber = mOptionTokenizer->getOption()->getOptionDefinition(optionNameData).number;
    const QModelIndex insertKeyIndex    = mParameterTableModel->index(row, ConfigParamTableModel::COLUMN_KEY);
    const QModelIndex insertValueIndex  = mParameterTableModel->index(row, ConfigParamTableModel::COLUMN_VALUE);
    const QModelIndex insertNumberIndex = mParameterTableModel->index(row, ConfigParamTableModel::COLUMN_ID);
    const QModelIndex minVersionIndex   = mParameterTableModel->index(row, ConfigParamTableModel::COLUMN_MIN_VERSION);
    const QModelIndex maxVersionIndex   = mParameterTableModel->index(row, ConfigParamTableModel::COLUMN_MIN_VERSION);

    mParameterTableModel->setData( insertNumberIndex, optionEntryNumber, Qt::EditRole);
    mParameterTableModel->setData( insertKeyIndex,    optionNameData,    Qt::EditRole);
    mParameterTableModel->setData( insertValueIndex,  selectedValueData, Qt::EditRole);
    mParameterTableModel->setData( minVersionIndex, "", Qt::EditRole);
    mParameterTableModel->setData( maxVersionIndex, "", Qt::EditRole);
    mParameterTableModel->setHeaderData( row, Qt::Vertical, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole );
}

} // namepsace option
} // namespace studio
} // namespace gams

