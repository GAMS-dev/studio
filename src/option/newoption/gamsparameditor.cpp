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
#include "option/newoption/gamsparameditor.h"
#include "option/gamsparamtablemodel.h"
#include "option/gamsoptiondefinitionmodel.h"
#include "ui_optionwidget.h"

namespace gams {
namespace studio {
namespace option {
namespace newoption {

GamsParamEditor::GamsParamEditor(const QString &commandLineParameter,
                                 OptionTokenizer* tokenizer,
                                 QWidget *parent):
    OptionWidget(false, parent), mOptionTokenizer(tokenizer)
{
    QList<OptionItem *> optionItem = mOptionTokenizer->tokenize(commandLineParameter);
//    QString normalizedText = mOptionTokenizer->normalize(optionItem);
    mOptionModel = new GamsParamTableModel(optionItem, mOptionTokenizer,  this);

    mDefinitionModel = new GamsOptionDefinitionModel(mOptionTokenizer->getOption(), 0, this);

    initActions();
    initToolBar();
    initOptionTableView();
    initDefintionTreeView();
    initTabNavigation( false );
    initMessageControl( false );

    connect(ui->optionTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &GamsParamEditor::selectionChanged);

    connect(mOptionModel, &GamsParamTableModel::optionModelChanged, mDefinitionModel, &GamsOptionDefinitionModel::modifyOptionDefinition, Qt::UniqueConnection);

}

GamsParamEditor::~GamsParamEditor()
{
    if (mDefinitionGroupModel)
        delete mDefinitionGroupModel;
    if (mDefinitionModel)
        delete mDefinitionModel;
    if (mDefinitionGroupModel)
        delete mDefinitionProxymodel;
    if (mOptionModel)
        delete mOptionModel;
}

void gams::studio::option::newoption::GamsParamEditor::setEditorExtended(bool extended)
{
    if (extended) {
        ui->definitionTreeView->clearSelection();
        ui->definitionTreeView->collapseAll();
    }
    mExtended = extended;
}

bool gams::studio::option::newoption::GamsParamEditor::isEditorExtended()
{
    return mExtended;
}

void gams::studio::option::newoption::GamsParamEditor::insertOption()
{
    qDebug() << "103:" << (mExtended ? "extended":"not extended") << ", hasFocus:" << (ui->optionTableView->hasFocus() ? "Focus": "NO focus");
    if (!mExtended || !ui->optionTableView->hasFocus() )
        return;

    qDebug() << "107:";
    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex index : std::as_const(indexSelection)) {
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    QModelIndexList selection = ui->optionTableView->selectionModel()->selectedRows();

    if (mOptionModel->rowCount() <= 0 || selection.count() <= 0) {
        mOptionModel->insertRows(mOptionModel->rowCount(), 1, QModelIndex());
        const QModelIndex index = mOptionModel->index( mOptionModel->rowCount()-1, GamsParamTableModel::COLUMN_KEY);
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
        ui->optionTableView->edit( index );

        ui->optionTableView->scrollTo(index, QAbstractItemView::EnsureVisible);
    } else if (selection.count() > 0) {
        QList<int> rows;
        for(const QModelIndex idx : std::as_const(selection)) {
            rows.append( idx.row() );
        }
        std::sort(rows.begin(), rows.end());
        ui->optionTableView->model()->insertRows(rows.at(0), 1, QModelIndex());
        const QModelIndex index = ui->optionTableView->model()->index(rows.at(0), GamsParamTableModel::COLUMN_KEY);
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
        ui->optionTableView->edit( mOptionModel->index(index.row(), GamsParamTableModel::COLUMN_KEY) );

        ui->optionTableView->scrollTo(index, QAbstractItemView::EnsureVisible);
    }
}

void gams::studio::option::newoption::GamsParamEditor::deleteOption()
{
    if (!mExtended)
        return;

    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex index : std::as_const(indexSelection)) {
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    const QModelIndexList selection = ui->optionTableView->selectionModel()->selectedRows();
    if (selection.count() <= 0)
        return;

    disconnect(ui->definitionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
               this, &GamsParamEditor::findAndSelectionOptionFromDefinition);

    QList<int> rows;
    const auto indexes = ui->optionTableView->selectionModel()->selectedRows();
    for(const QModelIndex & index : indexes) {
        rows.append( index.row() );
    }
    std::sort(rows.begin(), rows.end());
    int prev = -1;
    for(int i=rows.count()-1; i>=0; i--) {
        const int current = rows[i];
        if (current != prev) {
            ui->optionTableView->model()->removeRows( current, 1 );
            prev = current;
        }
    }

    ui->optionTableView->clearSelection();
    ui->optionTableView->setFocus();
    connect(ui->definitionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &GamsParamEditor::findAndSelectionOptionFromDefinition, Qt::UniqueConnection);
    updateActionsState();
}

void gams::studio::option::newoption::GamsParamEditor::moveOptionUp()
{
    if (!mExtended)
        return;

    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex index : std::as_const(indexSelection)) {
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    const QModelIndexList selection = ui->optionTableView->selectionModel()->selectedRows();
    if (selection.count() <= 0)
        return;

    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() < b.row(); });
    if (idxSelection.first().row() <= 0)
        return;

    for(int i=0; i<idxSelection.size(); i++) {
        const QModelIndex idx = idxSelection.at(i);
        ui->optionTableView->model()->moveRows(QModelIndex(), idx.row(), 1,
                                                      QModelIndex(), idx.row()-1);
    }

    disconnect(ui->definitionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
               this, &GamsParamEditor::findAndSelectionOptionFromDefinition);
    disconnect(optionModel(), &QAbstractTableModel::dataChanged, optionModel(), &OptionTableModel::on_updateOptionItem);
    QItemSelection select;
    for(const QModelIndex indx : std::as_const(idxSelection)) {
        const QModelIndex leftIndex  = ui->optionTableView->model()->index(indx.row()-1, GamsParamTableModel::COLUMN_KEY);
        const QModelIndex rightIndex = ui->optionTableView->model()->index(indx.row()-1, ui->optionTableView->model()->columnCount()-1);
        const QItemSelection rowSelection(leftIndex, rightIndex);
        select.merge(rowSelection, QItemSelectionModel::Select);
    }
    ui->optionTableView->selectionModel()->select(select, QItemSelectionModel::ClearAndSelect);
    connect(ui->definitionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &GamsParamEditor::findAndSelectionOptionFromDefinition, Qt::UniqueConnection);
    connect(optionModel(), &QAbstractTableModel::dataChanged, optionModel(), &OptionTableModel::on_updateOptionItem, Qt::UniqueConnection);

    updateActionsState();
}

void gams::studio::option::newoption::GamsParamEditor::moveOptionDown()
{
    if (!mExtended)
        return;

    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex index : std::as_const(indexSelection)) {
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    const QModelIndexList selection = ui->optionTableView->selectionModel()->selectedRows();
    if (selection.count() <= 0)
        return;

    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() > b.row(); });
    if (idxSelection.first().row() >= mOptionModel->rowCount()-1)
        return;

    for(int i=0; i<idxSelection.size(); i++) {
        const QModelIndex idx = idxSelection.at(i);
        mOptionModel->moveRows(QModelIndex(), idx.row(), 1,
                                       QModelIndex(), idx.row()+2);
    }

    disconnect(ui->definitionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
               &GamsParamEditor::findAndSelectionOptionFromDefinition);
    QItemSelection select;
    for(const QModelIndex indx : std::as_const(idxSelection)) {
        const QModelIndex leftIndex  = ui->optionTableView->model()->index(indx.row()+1, GamsParamTableModel::COLUMN_KEY);
        const QModelIndex rightIndex = ui->optionTableView->model()->index(indx.row()+1, GamsParamTableModel::COLUMN_ID);
        const QItemSelection rowSelection(leftIndex, rightIndex);
        select.merge(rowSelection, QItemSelectionModel::Select);
    }
    ui->optionTableView->selectionModel()->select(select, QItemSelectionModel::ClearAndSelect);
    connect(ui->definitionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &GamsParamEditor::findAndSelectionOptionFromDefinition, Qt::UniqueConnection);
    updateActionsState();
}

void GamsParamEditor::addOptionModelFromDefinition(int row, const QModelIndex &defindex, const QModelIndex &parentIndex)
{
    disconnect(mOptionModel, &GamsParamTableModel::optionModelChanged, mDefinitionModel, &GamsOptionDefinitionModel::modifyOptionDefinition);
    if (row ==  ui->optionTableView->model()->rowCount()) {
        mOptionModel->insertRows(row, 1, QModelIndex());
    }

    const QModelIndex optionNameIndex    = (parentIndex.row()<0) ? defindex.siblingAtColumn(OptionDefinitionModel::COLUMN_OPTION_NAME)
                                                                 : parentIndex.siblingAtColumn(OptionDefinitionModel::COLUMN_OPTION_NAME) ;
    const QModelIndex defValueIndex      = (parentIndex.row()<0) ? defindex.siblingAtColumn(OptionDefinitionModel::COLUMN_DEF_VALUE)
                                                                 : parentIndex.siblingAtColumn(OptionDefinitionModel::COLUMN_DEF_VALUE);
    const QModelIndex selectedValueIndex = (parentIndex.row()<0) ? defValueIndex
                                                                 : defindex.siblingAtColumn(OptionDefinitionModel::COLUMN_OPTION_NAME);

    const QString optionNameData    = ui->definitionTreeView->model()->data(optionNameIndex, Qt::DisplayRole).toString();
    const QString defValueData      = ui->definitionTreeView->model()->data(defValueIndex, Qt::DisplayRole).toString();
    const QString selectedValueData = ui->definitionTreeView->model()->data(selectedValueIndex, Qt::DisplayRole).toString();

    const QModelIndex insertNumberIndex = mOptionModel->index(row, OptionTableModel::COLUMN_ID);
    const QModelIndex insertKeyIndex    = mOptionModel->index(row, OptionTableModel::COLUMN_KEY);
    const QModelIndex insertValueIndex  = mOptionModel->index(row, OptionTableModel::COLUMN_VALUE);
    const int optionEntryNumber = mOptionTokenizer->getOption()->getOptionDefinition(optionNameData).number;

    mOptionModel->setData( insertNumberIndex, optionEntryNumber, Qt::EditRole);
    mOptionModel->setData( insertKeyIndex,    optionNameData,    Qt::EditRole);
    mOptionModel->setData( insertValueIndex,  selectedValueData, Qt::EditRole);

    mOptionModel->setHeaderData( row, Qt::Vertical, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole );
    connect(mOptionModel, &GamsParamTableModel::optionModelChanged, mDefinitionModel, &GamsOptionDefinitionModel::modifyOptionDefinition, Qt::UniqueConnection);
}

void GamsParamEditor::clearDefintionSelection()
{
    ui->definitionTreeView->clearSelection();
    ui->definitionTreeView->collapseAll();
}

void GamsParamEditor::clearOptionSelection()
{
    ui->optionTableView->clearSelection();
}

void GamsParamEditor::parameterItemCommitted(const QModelIndex &index)
{
    if (index.isValid()) {
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect );
        ui->optionTableView->setCurrentIndex( index );
        ui->optionTableView->setFocus();
    }
}

void gams::studio::option::newoption::GamsParamEditor::focus()
{
    if (ui->optionTableView->hasFocus())
        ui->definitionSearch->setFocus(Qt::ShortcutFocusReason);
    else if (ui->definitionSearch->hasFocus())
        ui->definitionTreeView->setFocus(Qt::ShortcutFocusReason);
    else
        ui->optionTableView->setFocus(Qt::TabFocusReason);
}

bool GamsParamEditor::hasFocus()
{
    return (ui->optionTableView->hasFocus() || ui->optionDefinitionView->hasFocus());
}

bool GamsParamEditor::hasSelection()
{
    return (ui->optionTableView->selectionModel()->hasSelection() || ui->definitionTreeView->selectionModel()->hasSelection());
}

QString GamsParamEditor::getSelectedParameterName(QWidget *widget) const
{
    if (widget == ui->optionTableView) {
        const QModelIndexList selection = ui->optionTableView->selectionModel()->selectedIndexes();
        if (selection.count() > 0) {
            const QModelIndex index = selection.at(0);
            const QVariant headerData = mOptionModel->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole);
            if (Qt::CheckState(headerData.toUInt())==Qt::PartiallyChecked) {
                return "";
            }
            const QVariant data = mOptionModel->data( index.sibling(index.row(),0) );
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
            const QModelIndex  parentIndex =  mDefinitionModel->parent(index);
            if (parentIndex.row() >= 0) {
                return mDefinitionModel->data( parentIndex.sibling(parentIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME),
                                               Qt::DisplayRole ).toString();
            } else {
                return mDefinitionModel->data( index.sibling(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME),
                                               Qt::DisplayRole ).toString();
            }
        }
    }
    return "";
}

void GamsParamEditor::on_ParameterTableModelChanged(const QString &text)
{
    mOptionModel->on_ParameterTableModelChanged(text);
}

void GamsParamEditor::on_parameterTableNameChanged(const QString &from, const QString &to)
{
    if (QString::compare(from, to, Qt::CaseInsensitive)==0)
        return;

    QModelIndexList fromDefinitionItems = ui->definitionTreeView->model()->match(ui->definitionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                                    Qt::DisplayRole,
                                                                                    from, 1);
    if (fromDefinitionItems.size() <= 0) {
        fromDefinitionItems = ui->definitionTreeView->model()->match(ui->definitionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_SYNONYM),
                                                                        Qt::DisplayRole,
                                                                        from, 1);
    }
    for(const QModelIndex item : std::as_const(fromDefinitionItems)) {
        const QModelIndex index = ui->definitionTreeView->model()->index(item.row(), OptionDefinitionModel::COLUMN_OPTION_NAME);
        ui->definitionTreeView->model()->setData(index, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole);
    }

    QModelIndexList toDefinitionItems = ui->definitionTreeView->model()->match(ui->definitionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                                  Qt::DisplayRole,
                                                                                  to, 1);
    if (toDefinitionItems.size() <= 0) {
        toDefinitionItems = ui->definitionTreeView->model()->match(ui->definitionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_SYNONYM),
                                                                      Qt::DisplayRole,
                                                                      to, 1);
    }
    for(const QModelIndex item : std::as_const(toDefinitionItems)) {
        const QModelIndex index = ui->definitionTreeView->model()->index(item.row(), OptionDefinitionModel::COLUMN_OPTION_NAME);
        ui->definitionTreeView->model()->setData(index, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);
    }

    ui->definitionTreeView->selectionModel()->clearSelection();
    if (!toDefinitionItems.isEmpty()) {
        ui->definitionTreeView->selectionModel()->select(
            QItemSelection (
                ui->definitionTreeView->model ()->index (toDefinitionItems.first().row() , 0),
                ui->definitionTreeView->model ()->index (toDefinitionItems.first().row(), ui->definitionTreeView->model ()->columnCount () - 1)),
            QItemSelectionModel::Select);
        ui->definitionTreeView->scrollTo(toDefinitionItems.first(), QAbstractItemView::EnsureVisible);
    }
}

void GamsParamEditor::on_parameterValueChanged(const QModelIndex &index)
{
    disconnect(ui->definitionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &GamsParamEditor::findAndSelectionOptionFromDefinition);
    ui->definitionTreeView->selectionModel()->clearSelection();

    const QModelIndex idx = index.sibling(index.row(), GamsParamTableModel::COLUMN_KEY);
    const QString data = ui->optionTableView->model()->data(idx, Qt::DisplayRole).toString();
    QModelIndexList toDefinitionItems = ui->definitionTreeView->model()->match(ui->definitionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                                  Qt::DisplayRole,
                                                                                  data, 1);
    if (toDefinitionItems.isEmpty()) {
        toDefinitionItems = ui->definitionTreeView->model()->match(ui->definitionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_SYNONYM),
                                                                      Qt::DisplayRole,
                                                                      data, 1);
    } /*else {*/
    if (!toDefinitionItems.isEmpty()) {
        ui->definitionTreeView->selectionModel()->select(
            QItemSelection (
                ui->definitionTreeView->model ()->index (toDefinitionItems.first().row() , 0),
                ui->definitionTreeView->model ()->index (toDefinitionItems.first().row(), ui->definitionTreeView->model ()->columnCount () - 1)),
            QItemSelectionModel::Select);
        ui->definitionTreeView->scrollTo(toDefinitionItems.first(), QAbstractItemView::EnsureVisible);
    }
    connect(ui->definitionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &GamsParamEditor::findAndSelectionOptionFromDefinition, Qt::UniqueConnection);
}

void GamsParamEditor::deSelectParameters()
{
    if (ui->optionTableView->hasFocus() && ui->optionTableView->selectionModel()->hasSelection()) {
        ui->optionTableView->selectionModel()->clearSelection();
        ui->definitionTreeView->selectionModel()->clearSelection();
    } else if (ui->definitionTreeView->hasFocus() && ui->definitionTreeView->selectionModel()->hasSelection()) {
        ui->definitionTreeView->selectionModel()->clearSelection();
    }
}

} // namepsace newoption
} // namepsace option
} // namespace studio
} // namespace gams
