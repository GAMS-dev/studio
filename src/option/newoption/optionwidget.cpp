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
#include <QStandardItemModel>
#include <QClipboard>

#include "optionwidget.h"
#include "ui_optionwidget.h"

#include "theme.h"
#include "headerviewproxy.h"
#include "option/definitionitemdelegate.h"
#include "option/optionsortfilterproxymodel.h"
#include "option/optiondefinitionmodel.h"

namespace gams {
namespace studio {
namespace option {
namespace newoption {

OptionWidget::OptionWidget(bool isFileEditor, QWidget *parent) :
    AbstractView(parent),
    ui(new Ui::OptionWidget),
    mIsFileEditor(isFileEditor)
{
    ui->setupUi(this);
}

OptionWidget::~OptionWidget()
{
    delete mToolBar;
    delete ui;
}

void OptionWidget::initActions()
{
    ui->actionInsert->setEnabled(true);
    ui->actionInsert_Comment->setEnabled(true);
    ui->actionDelete->setEnabled(false);
    ui->actionMoveUp->setEnabled(false);
    ui->actionMoveDown->setEnabled(false);

    ui->actionSelect_Current_Row->setEnabled(true);
    ui->actionSelectAll->setEnabled(true);
    ui->actionShow_Option_Definition->setEnabled(false);
    ui->actionShowRecurrence->setEnabled(false);

    ui->actionAdd_This_Parameter->setEnabled(false);
    ui->actionRemove_This_Parameter->setEnabled(false);
    ui->actionResize_Columns_To_Contents->setEnabled(false);

    ui->actionResize_Columns_To_Contents->setEnabled(true);

    ui->actionInsert->setIcon(Theme::icon(":/%1/plus", true));
    ui->actionInsert_Comment->setIcon(Theme::icon(":/%1/plus", true));
    ui->actionDelete->setIcon(Theme::icon(":/%1/delete-all", true));
    ui->actionMoveUp->setIcon(Theme::icon(":/%1/move-up", true));
    ui->actionMoveDown->setIcon(Theme::icon(":/%1/move-down", true));

    ui->actionAdd_This_Parameter->setIcon(Theme::icon(":/%1/plus", true));
    ui->actionRemove_This_Parameter->setIcon(Theme::icon(":/%1/delete-all", true));
}

void OptionWidget::initToolBar()
{
    mToolBar = new QToolBar();
    mToolBar ->setIconSize(QSize(16,16));
    mToolBar->addAction(ui->actionInsert);
    mToolBar->addAction(ui->actionDelete);
    mToolBar->addSeparator();
    mToolBar->addAction(ui->actionMoveUp);
    mToolBar->addAction(ui->actionMoveDown);
    ui->optionActionCtrl->layout()->setMenuBar(mToolBar);
}

void OptionWidget::initTableView()
{
    ui->optionTableView->setModel( optionModel() );

    setOptionCompleter( new OptionItemDelegate(optionTokenizer(), ui->optionTableView) );
    ui->optionTableView->setItemDelegate( optionCompleter() );
//    connect(optionCompleter(), &QStyledItemDelegate::commitData, this, &ParamConfigEditor::parameterItemCommitted);

    ui->optionTableView->setEditTriggers(QAbstractItemView::DoubleClicked
                                           | QAbstractItemView::SelectedClicked
                                           | QAbstractItemView::EditKeyPressed
                                           | QAbstractItemView::AnyKeyPressed );
    ui->optionTableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->optionTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->optionTableView->setAutoScroll(true);
    ui->optionTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->optionTableView->setSortingEnabled(false);

    ui->optionTableView->setDragEnabled(true);
    ui->optionTableView->viewport()->setAcceptDrops(true);
    ui->optionTableView->setDropIndicatorShown(true);
    ui->optionTableView->setDragDropMode(QAbstractItemView::DropOnly);
    ui->optionTableView->setDragDropOverwriteMode(true);
    ui->optionTableView->setDefaultDropAction(Qt::CopyAction);

    ui->optionTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
//    ui->optionTableView->setColumnHidden(OptionTableModel::COLUMN_ID, true);
    ui->optionTableView->verticalHeader()->setMinimumSectionSize(1);
    ui->optionTableView->verticalHeader()->setDefaultSectionSize(static_cast<int>(fontMetrics().height()*TABLE_ROW_HEIGHT));
    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->optionTableView->horizontalHeader()->setStyle(HeaderViewProxy::instance());

    if (ui->optionTableView->model()->rowCount()>0) {
        ui->optionTableView->resizeColumnsToContents();
    }
    ui->optionTableView->horizontalHeader()->setStretchLastSection(true);
    ui->optionTableView->horizontalHeader()->setHighlightSections(false);

}

void OptionWidget::initTreeView()
{
    QList<OptionGroup> optionGroupList = optionTokenizer()->getOption()->getOptionGroupList();
    int groupsize = 0;
    for(const OptionGroup &group : std::as_const(optionGroupList)) {
        if (group.hidden || group.name.compare("deprecated", Qt::CaseInsensitive)==0)
            continue;
        else
            ++groupsize;
    }

    setDefinitionGroupModel( new QStandardItemModel(groupsize+1, 3) );
    int i = 0;
    definitionGroupModel()->setItem(0, 0, new QStandardItem("--- All Options ---"));
    definitionGroupModel()->setItem(0, 1, new QStandardItem("0"));
    definitionGroupModel()->setItem(0, 2, new QStandardItem("All Options"));
    for(const OptionGroup &group : std::as_const(optionGroupList)) {
        if (group.hidden || group.name.compare("deprecated", Qt::CaseInsensitive)==0)
            continue;
        ++i;
        definitionGroupModel()->setItem(i, 0, new QStandardItem(group.description));
        definitionGroupModel()->setItem(i, 1, new QStandardItem(QString::number(group.number)));
        definitionGroupModel()->setItem(i, 2, new QStandardItem(group.name));
    }
    ui->definitionGroup->setModel( definitionGroupModel() );
    ui->definitionGroup->setModelColumn(0);

    setDefintionProxyModel( new OptionSortFilterProxyModel(this) );
    definitionProxymodel()->setFilterKeyColumn(-1);
    definitionProxymodel()->setSourceModel( definitionModel() );
    definitionProxymodel()->setFilterCaseSensitivity(Qt::CaseInsensitive);
    definitionProxymodel()->setSortCaseSensitivity(Qt::CaseInsensitive);

    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->definitionTreeView->header()->setStyle(HeaderViewProxy::instance());
    ui->definitionTreeView->setModel( definitionProxymodel() );
    ui->definitionTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->definitionTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->definitionTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->definitionTreeView->setDragEnabled(true);
    ui->definitionTreeView->setDragDropMode(QAbstractItemView::DragOnly);

    ui->definitionTreeView->setItemDelegate( new DefinitionItemDelegate(ui->definitionTreeView) );
    ui->definitionTreeView->setItemsExpandable(true);
    ui->definitionTreeView->setSortingEnabled(true);
    ui->definitionTreeView->sortByColumn(0, Qt::AscendingOrder);
    ui->definitionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_OPTION_NAME);
    ui->definitionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_SYNONYM);
    ui->definitionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_DEF_VALUE);
    ui->definitionTreeView->setExpandsOnDoubleClick(false);
    if (!optionTokenizer()->getOption()->isSynonymDefined())
        ui->definitionTreeView->setColumnHidden( 1, true);
    ui->definitionTreeView->setColumnHidden(OptionDefinitionModel::COLUMN_ENTRY_NUMBER, true);

    headerRegister(ui->definitionTreeView->header());
}

void OptionWidget::initTabNavigation(bool isFileEditor)
{
    ui->optionTableView->setTabKeyNavigation(true);

    setTabOrder(ui->definitionGroup, ui->definitionSearch);
    setTabOrder(ui->definitionSearch, ui->definitionTreeView);
    if (isFileEditor) {
       setTabOrder(ui->definitionTreeView, ui->compactViewCheckBox);
       setTabOrder(ui->compactViewCheckBox, ui->openAsTextButton);
       setTabOrder(ui->openAsTextButton, ui->messageCtrlWidget);
    }
}

void OptionWidget::initMessageControl(bool isFileEditor)
{
    mLogEdit = new SystemLogEdit(this);
    mLogEdit->setObjectName("log-edit");
    optionTokenizer()->provideLogger(mLogEdit);

    ui->messageTabWidget->addTab( mLogEdit, "Messages" );
    ui->messageCtrlWidget->setVisible( isFileEditor );
    ui->messageTabWidget->setVisible( isFileEditor );
    ui->optionFileCtrlwidget->setVisible( isFileEditor );
}


bool OptionWidget::isThereARow() const
{
    return (ui->optionTableView->model()->rowCount() > 0);
}

bool OptionWidget::isThereAnIndexSelection() const
{
    QModelIndexList selection = ui->optionTableView->selectionModel()->selectedIndexes();
    return (selection.count() > 0);
}

bool OptionWidget::isThereARowSelection() const
{
    const QModelIndexList selection = ui->optionTableView->selectionModel()->selectedRows();
    return (selection.count() > 0);

}

bool OptionWidget::isEverySelectionARow() const
{
    const QModelIndexList selection = ui->optionTableView->selectionModel()->selectedRows();
    const QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();

    return ((selection.count() > 0) && (indexSelection.count() % ui->optionTableView->model()->columnCount() == 0));

}

bool OptionWidget::isInFocus(QWidget *focusWidget) const
{
    return (focusWidget==ui->optionTableView || focusWidget==ui->definitionTreeView ||
            focusWidget==ui->definitionSearch);
}

QString OptionWidget::getSelectedOptionName(QWidget *widget) const
{
    if (widget == ui->optionTableView) {
        const QModelIndexList selection = ui->optionTableView->selectionModel()->selectedIndexes();
        if (selection.count() > 0) {
            const QModelIndex index = selection.at(0);
            const QVariant headerData = ui->optionTableView->model()->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole);
            if (Qt::CheckState(headerData.toUInt())==Qt::PartiallyChecked) {
                return "";
            }
            const QVariant data = ui->optionTableView->model()->data( index.sibling(index.row(),0) );
            if (optionTokenizer()->getOption()->isValid(data.toString()))
                return data.toString();
            else if (optionTokenizer()->getOption()->isASynonym(data.toString()))
                return optionTokenizer()->getOption()->getNameFromSynonym(data.toString());
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
    return QString();
}

QStringList OptionWidget::getEnabledContextActions()
{
    QStringList res;
    if (!focusWidget()) return res;
    if (mLogEdit && focusWidget() == mLogEdit)
        return mLogEdit->getEnabledContextActions();

    QStringList namedSelectAll {"select-all", "actionSelect_all"};
    QStringList namedCopy {"edit-copy", "actionCopyDefinitionOptionName"};
    for (QAction *act : focusWidget()->actions()) {
        if (namedSelectAll.contains(act->objectName()) && !res.contains("select-all")) {
            res << "select-all";
        } else if (namedCopy.contains(act->objectName()) && !res.contains("edit-copy")) {
            res << "edit-copy";
        }
    }
    return res;
}

void OptionWidget::selectSearchField() const
{
//    if (!ui->messageCtrlWidget->isVisible())
//        return;
    ui->definitionSearch->setFocus();
}

void OptionWidget::copyAction()
{
    if (focusWidget() == mLogEdit)
        mLogEdit->copy();
    else
        copyDefinitionToClipboard(OptionDefinitionModel::COLUMN_OPTION_NAME);
}

void OptionWidget::completeEditingOption(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    Q_UNUSED(editor)
    Q_UNUSED(hint)
    showOptionDefinition(false);
}

void OptionWidget::showOptionDefinition(bool selectRow)
{
    if (ui->optionTableView->model()->rowCount() <= 0)
        return;

    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    if (indexSelection.count() <= 0)
        return;

    disconnect(ui->definitionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &OptionWidget::findAndSelectionOptionFromDefinition);

    ui->definitionSearch->clear();

    QModelIndexList selection;
    if (selectRow) {
        selectAnOption();
        selection = ui->optionTableView->selectionModel()->selectedRows();
        ui->optionTableView->selectionModel()->setCurrentIndex ( indexSelection.first(), QItemSelectionModel::Current ); //Select );
    } else {
        selection = indexSelection;
        ui->optionTableView->selectionModel()->setCurrentIndex ( indexSelection.first(), QItemSelectionModel::Current ); //Select );
    }

    QModelIndexList selectIndices;
    for (int i=0; i<selection.count(); i++) {
        const QModelIndex index = selection.at(i);
        if (Qt::CheckState(ui->optionTableView->model()->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole).toUInt())==Qt::PartiallyChecked)
            continue;

        const QString value = ui->optionTableView->model()->data( index.sibling(index.row(), optionModel()->column_value()), Qt::DisplayRole).toString();
        const QVariant optionId = ui->optionTableView->model()->data( index.sibling(index.row(), optionModel()->column_id()), Qt::DisplayRole);
        QModelIndexList indices = ui->definitionTreeView->model()->match(ui->definitionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_ENTRY_NUMBER),
                                                                         Qt::DisplayRole,
                                                                         optionId, 1, Qt::MatchExactly|Qt::MatchRecursive);
        for(const QModelIndex &idx : std::as_const(indices)) {
            const QModelIndex  parentIndex =  ui->definitionTreeView->model()->parent(idx);
            const QModelIndex optionIdx = ui->definitionTreeView->model()->index(idx.row(), OptionDefinitionModel::COLUMN_OPTION_NAME);

            if (parentIndex.row() < 0) {
                if (ui->definitionTreeView->model()->hasChildren(optionIdx) && !ui->definitionTreeView->isExpanded(optionIdx))
                    ui->definitionTreeView->expand(optionIdx);
            }
            bool found = false;
            for(int r=0; r <ui->definitionTreeView->model()->rowCount(optionIdx); ++r) {
                const QModelIndex i = ui->definitionTreeView->model()->index(r, OptionDefinitionModel::COLUMN_OPTION_NAME, optionIdx);
                const QString enumValue = ui->definitionTreeView->model()->data(i, Qt::DisplayRole).toString();
                if (QString::compare(value, enumValue, Qt::CaseInsensitive) == 0) {
                    selectIndices << i;
                    found = true;
                    break;
                }
            }
            if (!found)
                selectIndices << optionIdx;
        }
    }
    ui->definitionTreeView->selectionModel()->clearSelection();
    for(const QModelIndex &idx : std::as_const(selectIndices)) {
        QItemSelection selection = ui->definitionTreeView->selectionModel()->selection();
        const QModelIndex  parentIdx =  ui->definitionTreeView->model()->parent(idx);
        if (parentIdx.row() < 0) {
            selection.select(ui->definitionTreeView->model()->index(idx.row(), OptionDefinitionModel::COLUMN_OPTION_NAME),
                             ui->definitionTreeView->model()->index(idx.row(), ui->definitionTreeView->model()->columnCount()-1));
        } else  {
            selection.select(ui->definitionTreeView->model()->index(idx.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIdx),
                             ui->definitionTreeView->model()->index(idx.row(), ui->definitionTreeView->model()->columnCount()-1, parentIdx));
        }
        ui->definitionTreeView->selectionModel()->select(selection, QItemSelectionModel::Select);
    }
    if (!selectIndices.isEmpty()) {
        const QModelIndex parentIndex = ui->definitionTreeView->model()->parent(selectIndices.first());
        const QModelIndex scrollToIndex = (parentIndex.row() < 0  ? ui->definitionTreeView->model()->index(selectIndices.first().row(), OptionDefinitionModel::COLUMN_OPTION_NAME)
                                                                 : ui->definitionTreeView->model()->index(selectIndices.first().row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex));
        ui->definitionTreeView->scrollTo(scrollToIndex, QAbstractItemView::EnsureVisible);
        if (parentIndex.row() >= 0) {
            ui->definitionTreeView->scrollTo(parentIndex, QAbstractItemView::EnsureVisible);
            const QRect r = ui->definitionTreeView->visualRect(parentIndex);
            ui->definitionTreeView->horizontalScrollBar()->setValue(r.x());
        } else {
            const QRect r = ui->definitionTreeView->visualRect(scrollToIndex);
            ui->definitionTreeView->horizontalScrollBar()->setValue(r.x());
        }
    }

    connect(ui->definitionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &OptionWidget::findAndSelectionOptionFromDefinition, Qt::UniqueConnection);
}

void OptionWidget::showOptionRecurrence()
{
    if (!isInFocus(focusWidget()))
        return;

    const QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    if (indexSelection.size() <= 0) {
        showOptionDefinition();
        return;
    }

    QItemSelection selection = ui->optionTableView->selectionModel()->selection();
    selection.select(ui->optionTableView->model()->index(indexSelection.at(0).row(), 0),
                     ui->optionTableView->model()->index(indexSelection.at(0).row(), optionModel()->columnCount()));
    ui->optionTableView->selectionModel()->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows );

    QList<int> rowList = getRecurrentOption(indexSelection.at(0));
    if (rowList.size() <= 0) {
        showOptionDefinition();
        return;
    }

    for(const int row : std::as_const(rowList)) {
        QItemSelection rowSelection = ui->optionTableView->selectionModel()->selection();
        rowSelection.select(ui->optionTableView->model()->index(row, 0),
                            ui->optionTableView->model()->index(row, optionModel()->columnCount()));
        ui->optionTableView->selectionModel()->select(rowSelection, QItemSelectionModel::Select | QItemSelectionModel::Rows );
    }

    showOptionDefinition();
}

void OptionWidget::on_actionSelect_Current_Row_triggered()
{
    if (!ui->actionSelect_Current_Row->isEnabled())
        return;
    QList<int> rowList;
    const auto indexes = ui->optionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex &idx : indexes) {
        if (!rowList.contains(idx.row())) {
            on_selectRow(idx.row());
            rowList << idx.row();
        }
    }
}

void OptionWidget::on_actionSelectAll_triggered()
{
    if (!ui->actionSelectAll->isEnabled())
        return;
    selectAllOptions();
}

void OptionWidget::on_actionShowRecurrence_triggered()
{
    if (!ui->actionShowRecurrence->isEnabled())
        return;

    const QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    if (indexSelection.size() <= 0) {
        showOptionDefinition();
        return;
    }

    QItemSelection selection = ui->optionTableView->selectionModel()->selection();
    selection.select(ui->optionTableView->model()->index(indexSelection.at(0).row(), 0),
                     ui->optionTableView->model()->index(indexSelection.at(0).row(), optionModel()->columnCount()));
    ui->optionTableView->selectionModel()->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows );

    QList<int> rowList = getRecurrentOption(indexSelection.at(0));
    if (rowList.size() <= 0) {
        showOptionDefinition();
        return;
    }

    for(const int row : std::as_const(rowList)) {
        QItemSelection rowSelection = ui->optionTableView->selectionModel()->selection();
        rowSelection.select(ui->optionTableView->model()->index(row, 0),
                            ui->optionTableView->model()->index(row, optionModel()->columnCount()));
        ui->optionTableView->selectionModel()->select(rowSelection, QItemSelectionModel::Select | QItemSelectionModel::Rows );
    }

    showOptionDefinition();
}

void OptionWidget::on_actionResize_Columns_To_Contents_triggered()
{
    if (!ui->actionResize_Columns_To_Contents->isEnabled())
        return;

    if (focusWidget()==ui->optionTableView) {
        if (ui->optionTableView->model()->rowCount()<=0)
            return;
        ui->optionTableView->resizeColumnToContents(OptionTableModel::COLUMN_KEY);
        ui->optionTableView->resizeColumnToContents(OptionTableModel::COLUMN_VALUE);
    } else  if (focusWidget()==ui->definitionTreeView) {
        ui->definitionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_OPTION_NAME);
        ui->definitionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_SYNONYM);
        ui->definitionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_DEF_VALUE);
        ui->definitionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_RANGE);
        ui->definitionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_TYPE);
    }
}

void OptionWidget::on_actionShow_Option_Definition_triggered()
{
    if (!ui->actionShow_Option_Definition->isEnabled())
        return;
    showOptionDefinition(true);
}

void OptionWidget::on_actionAdd_This_Parameter_triggered()
{
    if (!ui->actionAdd_This_Parameter->isEnabled())
        return;

    const QModelIndexList selection = ui->definitionTreeView->selectionModel()->selectedIndexes(); // Rows();
    if (selection.size()>0) {
        addOptionFromDefinition(selection.at(0));
        const QModelIndex index = ui->definitionTreeView->selectionModel()->currentIndex();
        updateDefinitionActionsState(index);
    }
}

void OptionWidget::on_actionRemove_This_Parameter_triggered()
{
    if (!ui->actionRemove_This_Parameter->isEnabled())
        return;

    findAndSelectionOptionFromDefinition();
    deleteOption();
}

MainWindow *OptionWidget::getMainWindow() const
{
    const auto widgets = qApp->topLevelWidgets();
    for(QWidget *widget : widgets)
        if (MainWindow *mainWindow = qobject_cast<MainWindow*>(widget))
            return mainWindow;
    return nullptr;
}

void OptionWidget::selectAllOptions()
{
    if (focusWidget() == mLogEdit) {
        mLogEdit->selectAll();
        return;
    }
    if (isViewCompact()) return;

    ui->optionTableView->setFocus();
    ui->optionTableView->selectAll();

    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    if (!indexSelection.isEmpty())
        ui->optionTableView->selectionModel()->setCurrentIndex( indexSelection.first(), QItemSelectionModel::Current );

    updateActionsState();
}

void OptionWidget::deSelectOptions()
{
    if (ui->optionTableView->hasFocus() && ui->optionTableView->selectionModel()->hasSelection()) {
        QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
        ui->definitionTreeView->selectionModel()->clearSelection();
        ui->optionTableView->selectionModel()->clearSelection();
        if (!indexSelection.isEmpty())
            ui->optionTableView->selectionModel()->setCurrentIndex( indexSelection.first(), QItemSelectionModel::Current );
        ui->optionTableView->setFocus();
    } else if (ui->definitionTreeView->hasFocus() && ui->definitionTreeView->selectionModel()->hasSelection()) {
        ui->definitionTreeView->selectionModel()->clearSelection();
        ui->definitionTreeView->setFocus();
    } else {
        this->focusNextChild();
    }
}

int OptionWidget::getItemCount() const
{
    return ui->optionTableView->model()->rowCount();
}

void OptionWidget::on_selectRow(int logicalIndex) const
{
    if (ui->optionTableView->model()->rowCount() <= 0)
        return;

    QItemSelectionModel *selectionModel = ui->optionTableView->selectionModel();
    const QModelIndex topLeft = ui->optionTableView->model()->index(logicalIndex, OptionTableModel::COLUMN_ID, QModelIndex());
    const QModelIndex  bottomRight = ui->optionTableView->model()->index(logicalIndex, OptionTableModel::COLUMN_ID, QModelIndex());
    const QItemSelection selection( topLeft, bottomRight);
    selectionModel->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

void OptionWidget::selectAnOption() const
{
    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    if (indexSelection.empty())
        indexSelection <<  ui->optionTableView->selectionModel()->currentIndex();

    QList<int> rowIndex;
    for(int i=0; i<indexSelection.count(); ++i) {
        if (!rowIndex.contains(i)) {
            rowIndex << i;
            on_selectRow( indexSelection.at(i).row() );
        }
    }
    ui->optionTableView->selectionModel()->clearCurrentIndex();
    ui->optionTableView->selectionModel()->setCurrentIndex( indexSelection.first(), QItemSelectionModel::Current );
}

bool OptionWidget::isViewCompact() const
{
    return ( ui->messageCtrlWidget->isVisible() && ui->compactViewCheckBox->isChecked() );
}

void OptionWidget::updateActionsState()
{
    QModelIndexList idxSelection = ( ui->optionTableView->selectionModel()->selectedRows().isEmpty()
                                    ? ui->optionTableView->selectionModel()->selectedIndexes()
                                    : ui->optionTableView->selectionModel()->selectedRows() );

    bool thereIsSelection = (isThereARow() && !idxSelection.isEmpty());

    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() < b.row(); });

    ui->actionInsert->setEnabled( true );
    ui->actionInsert_Comment->setEnabled( true );
    ui->actionDelete->setEnabled(  thereIsSelection ? idxSelection.first().row() < optionModel()->rowCount() : false );

    ui->actionMoveUp->setEnabled( thereIsSelection ? idxSelection.first().row() > 0 : false );
    ui->actionMoveDown->setEnabled( thereIsSelection ? idxSelection.last().row() < optionModel()->rowCount()-1 : false );

    ui->actionSelect_Current_Row->setEnabled( thereIsSelection );
    ui->actionSelectAll->setEnabled( thereIsSelection );

    ui->actionShow_Option_Definition->setEnabled( thereIsSelection
                                                     ? (idxSelection.first().row() < optionModel()->rowCount() && optionModel()->data(idxSelection.first().siblingAtColumn(optionModel()->column_id())).toInt()!=-1)
                                                     : false );
    ui->actionResize_Columns_To_Contents->setEnabled( thereIsSelection ? idxSelection.first().row() < optionModel()->rowCount() : false);
    ui->actionShowRecurrence->setEnabled( thereIsSelection ? idxSelection.first().row() < optionModel()->rowCount()
                                                                && getRecurrentOption(idxSelection.first()).size() >0 : false );
    ui->actionInsert->icon().pixmap( QSize(16, 16), ui->actionInsert->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                    QIcon::Off);
    ui->actionInsert_Comment->icon().pixmap( QSize(16, 16), ui->actionInsert_Comment->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                    QIcon::Off);
    ui->actionDelete->icon().pixmap( QSize(16, 16), ui->actionDelete->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                    QIcon::Off);
    ui->actionMoveUp->icon().pixmap( QSize(16, 16), ui->actionMoveUp->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                    QIcon::Off);
    ui->actionMoveUp->icon().pixmap( QSize(16, 16), ui->actionMoveDown->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                    QIcon::Off);
    mToolBar->repaint();
}

void OptionWidget::updateActionsState(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    QModelIndexList idxSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    bool thereIsSelection = (isThereARow() && !idxSelection.isEmpty());

    bool singleSelection = (idxSelection.size() ==1);
    bool singleSelectionIsRow = (singleSelection && ui->optionTableView->selectionModel()->isRowSelected(idxSelection.first().row()));
    bool multiSelectionIsRow = false;
    bool multiSelectionIsCell_sameRow = multiSelectionIsRow;
    if (!singleSelection) {
        QList<int> rowList;
        for (QModelIndex& idx :idxSelection) {
            if (ui->optionTableView->selectionModel()->isRowSelected(idx.row())) {
                multiSelectionIsRow = true;
                break;
            }
            if (rowList.contains(idx.row())) {
                continue;
            }
            rowList.append(idx.row());
        }
        multiSelectionIsCell_sameRow = (rowList.size() == 1);
    }

    ui->actionInsert->setEnabled( true );
    ui->actionInsert_Comment->setEnabled( true );
    ui->actionDelete->setEnabled( thereIsSelection && idxSelection.first().row() < optionModel()->rowCount() );
    ui->actionMoveUp->setEnabled(   (singleSelection || singleSelectionIsRow || multiSelectionIsRow || multiSelectionIsCell_sameRow) && idxSelection.first().row() > 0 );
    ui->actionMoveDown->setEnabled( (singleSelection || singleSelectionIsRow || multiSelectionIsRow || multiSelectionIsCell_sameRow) && idxSelection.last().row() < optionModel()->rowCount()-1 );
    ui->actionSelect_Current_Row->setEnabled( thereIsSelection );
    ui->actionSelectAll->setEnabled( thereIsSelection );
    ui->actionShow_Option_Definition->setEnabled( thereIsSelection && index.row() < optionModel()->rowCount() );
    ui->actionResize_Columns_To_Contents->setEnabled( thereIsSelection );
    ui->actionShowRecurrence->setEnabled( false );

    ui->actionInsert->icon().pixmap( QSize(16, 16), ui->actionInsert->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                    QIcon::Off);
    ui->actionInsert_Comment->icon().pixmap( QSize(16, 16), ui->actionInsert_Comment->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                    QIcon::Off);
    ui->actionDelete->icon().pixmap( QSize(16, 16), ui->actionDelete->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                    QIcon::Off);
    ui->actionMoveUp->icon().pixmap( QSize(16, 16), ui->actionMoveUp->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                    QIcon::Off);
    ui->actionMoveUp->icon().pixmap( QSize(16, 16), ui->actionMoveDown->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                    QIcon::Off);
    mToolBar->repaint();
}

void OptionWidget::updateDefinitionActionsState(const QModelIndex &index)
{
    QModelIndex parentIndex =  ui->definitionTreeView->model()->parent(index);
    QVariant data = (parentIndex.row() < 0) ? ui->definitionTreeView->model()->data(index, Qt::CheckStateRole)
                                            : ui->definitionTreeView->model()->data(parentIndex, Qt::CheckStateRole);
    ui->actionAdd_This_Parameter->setEnabled( Qt::CheckState(data.toInt()) == Qt::Unchecked );
    ui->actionRemove_This_Parameter->setEnabled( Qt::CheckState(data.toInt()) == Qt::Checked );
    ui->actionResize_Columns_To_Contents->setEnabled( true );

    ui->actionAdd_This_Parameter->icon().pixmap( QSize(16, 16), ui->actionAdd_This_Parameter->isEnabled() ? QIcon::Selected : QIcon::Disabled,
                                                QIcon::Off);
    ui->actionRemove_This_Parameter->icon().pixmap( QSize(16, 16), ui->actionRemove_This_Parameter ? QIcon::Selected : QIcon::Disabled,
                                                   QIcon::Off);
}

QList<int> OptionWidget::getRecurrentOption(const QModelIndex &index)
{
    QList<int> optionList;

    if (!isInFocus(focusWidget()))
        return optionList;

    const QVariant data = ui->optionTableView->model()->headerData(index.row(), Qt::Vertical,  Qt::CheckStateRole);
    if (Qt::CheckState(data.toUInt())==Qt::PartiallyChecked)
        return optionList;

    const QString optionId = ui->optionTableView->model()->data( index.sibling(index.row(), optionModel()->column_id()), Qt::DisplayRole).toString();
    QModelIndexList indices = ui->optionTableView->model()->match(ui->optionTableView->model()->index(0, optionModel()->column_id()),
                                                                  Qt::DisplayRole,
                                                                  optionId, -1);
    for(const QModelIndex &idx : std::as_const(indices)) {
        if (idx.row() == index.row())
            continue;
        else
            optionList << idx.row();
    }
    return optionList;
}

void OptionWidget::copyDefinitionToClipboard(int column)
{
    if (ui->definitionTreeView->selectionModel()->selectedRows().count() <= 0)
        return;

    const QModelIndex index = ui->definitionTreeView->selectionModel()->selectedRows().at(0);
    if (!index.isValid())
        return;

    QString text = "";
    const QModelIndex parentIndex = ui->definitionTreeView->model()->parent(index);
    if (column == -1) { // copy all
        QStringList strList;
        if (parentIndex.isValid()) {
            QModelIndex idx = ui->definitionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex);
            strList << ui->definitionTreeView->model()->data(idx, Qt::DisplayRole).toString();
            idx = ui->definitionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_DESCIPTION, parentIndex);
            strList << ui->definitionTreeView->model()->data(idx, Qt::DisplayRole).toString();
            text = strList.join(", ");
        } else {
            for (int j=0; j<ui->definitionTreeView->model()->columnCount(); j++) {
                if (j==OptionDefinitionModel::COLUMN_ENTRY_NUMBER)
                    continue;
                const QModelIndex columnindex = ui->definitionTreeView->model()->index(index.row(), j);
                strList << ui->definitionTreeView->model()->data(columnindex, Qt::DisplayRole).toString();
            }
            text = strList.join(", ");
        }
    } else {
        if (parentIndex.isValid()) {
            const QModelIndex idx = ui->definitionTreeView->model()->index(index.row(), column, parentIndex);
            text = ui->definitionTreeView->model()->data( idx, Qt::DisplayRole ).toString();
        } else {
            text = ui->definitionTreeView->model()->data( ui->definitionTreeView->model()->index(index.row(), column), Qt::DisplayRole ).toString();
        }
    }
    QClipboard* clip = QApplication::clipboard();
    clip->setText( text );
}

} // namepsace newoption
} // namepsace option
} // namespace studio
} // namespace gams
