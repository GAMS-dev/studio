/* This file is part of the GAMS Studio project.
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
#include <QMessageBox>
#include <QShortcut>

#include "optionwidget.h"
#include "ui_optionwidget.h"

#include "addoptionheaderview.h"
#include "commonpaths.h"
#include "optionsortfilterproxymodel.h"
#include "gamsoptiondefinitionmodel.h"
#include "mainwindow.h"

namespace gams {
namespace studio {
namespace option {

OptionWidget::OptionWidget(QAction *aRun, QAction *aRunGDX, QAction *aCompile, QAction *aCompileGDX, QAction *aInterrupt, QAction *aStop, MainWindow *parent):
    ui(new Ui::OptionWidget),
    actionRun(aRun), actionRun_with_GDX_Creation(aRunGDX), actionCompile(aCompile), actionCompile_with_GDX_Creation(aCompileGDX),
    actionInterrupt(aInterrupt), actionStop(aStop), main(parent)
{
    ui->setupUi(this);

    addActions();
    mOptionTokenizer = new OptionTokenizer(QString("optgams.def"));
//    mCommandLineHistory = new CommandLineHistory(this);

    setRunsActionGroup(actionRun, actionRun_with_GDX_Creation, actionCompile, actionCompile_with_GDX_Creation);
    setInterruptActionGroup(aInterrupt, actionStop);

    setFocusPolicy(Qt::StrongFocus);

    ui->gamsOptionWidget->hide();
    connect(ui->gamsOptionCommandLine, &CommandLineOption::optionRunChanged, main, &MainWindow::optionRunChanged);
    connect(ui->gamsOptionCommandLine, &QComboBox::editTextChanged, ui->gamsOptionCommandLine, &CommandLineOption::validateChangedOption);
    connect(ui->gamsOptionCommandLine, &CommandLineOption::commandLineOptionChanged, mOptionTokenizer, &OptionTokenizer::formatTextLineEdit);
    connect(ui->gamsOptionCommandLine, &CommandLineOption::commandLineOptionChanged, this, &OptionWidget::updateOptionTableModel );
    connect(ui->gamsOptionCommandLine, &CommandLineOption::optionEditCancelled, this, [this]() { ui->gamsOptionCommandLine->clearFocus(); });

    QList<OptionItem> optionItem = mOptionTokenizer->tokenize(ui->gamsOptionCommandLine->lineEdit()->text());
    QString normalizedText = mOptionTokenizer->normalize(optionItem);
    mOptionTableModel = new GamsOptionTableModel(normalizedText, mOptionTokenizer,  this);
    ui->gamsOptionTableView->setModel( mOptionTableModel );
    connect(mOptionTableModel, &GamsOptionTableModel::optionModelChanged, this, static_cast<void(OptionWidget::*)(const QList<OptionItem> &)> (&OptionWidget::updateCommandLineStr));
    connect(this, static_cast<void(OptionWidget::*)(QLineEdit*, const QList<OptionItem> &)>(&OptionWidget::commandLineOptionChanged), mOptionTokenizer, &OptionTokenizer::formatItemLineEdit);

    mOptionCompleter = new OptionCompleterDelegate(mOptionTokenizer, ui->gamsOptionTableView);
    ui->gamsOptionTableView->setItemDelegate( mOptionCompleter );
    connect(mOptionCompleter, &QStyledItemDelegate::commitData, this, &OptionWidget::optionItemCommitted);
    ui->gamsOptionTableView->setEditTriggers(QAbstractItemView::DoubleClicked
                       | QAbstractItemView::SelectedClicked
                       | QAbstractItemView::EditKeyPressed
                       | QAbstractItemView::AnyKeyPressed );
    ui->gamsOptionTableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->gamsOptionTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->gamsOptionTableView->setAutoScroll(true);
    ui->gamsOptionTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->gamsOptionTableView->setSortingEnabled(false);

    ui->gamsOptionTableView->setDragEnabled(true);
    ui->gamsOptionTableView->viewport()->setAcceptDrops(true);
    ui->gamsOptionTableView->setDropIndicatorShown(true);
    ui->gamsOptionTableView->setDragDropMode(QAbstractItemView::DropOnly);
    ui->gamsOptionTableView->setDragDropOverwriteMode(true);
    ui->gamsOptionTableView->setDefaultDropAction(Qt::CopyAction);

    AddOptionHeaderView* headerView = new AddOptionHeaderView(Qt::Horizontal, ui->gamsOptionTableView);
    headerView->setSectionResizeMode(QHeaderView::Interactive);
    headerView->setDefaultSectionSize( static_cast<int>(ui->gamsOptionTableView->sizeHint().width()/(ui->gamsOptionTableView->model()->columnCount()-1)) );
    ui->gamsOptionTableView->setHorizontalHeader(headerView);
    ui->gamsOptionTableView->setColumnHidden(GamsOptionTableModel::COLUMN_ENTRY_NUMBER, true);

    ui->gamsOptionTableView->verticalHeader()->setDefaultSectionSize(ui->gamsOptionTableView->verticalHeader()->minimumSectionSize());
    ui->gamsOptionTableView->horizontalHeader()->setStretchLastSection(true);
    ui->gamsOptionTableView->horizontalHeader()->setHighlightSections(false);
    if (ui->gamsOptionTableView->model()->rowCount()<=0)
         ui->gamsOptionTableView->horizontalHeader()->setDefaultSectionSize( ui->gamsOptionTableView->sizeHint().width()/(ui->gamsOptionTableView->model()->columnCount()-1) );
    else
        ui->gamsOptionTableView->resizeColumnsToContents();

    ui->gamsOptionTableView->setTabKeyNavigation(false);

    connect(ui->gamsOptionTableView, &QTableView::customContextMenuRequested,this, &OptionWidget::showOptionContextMenu);
    connect(this, &OptionWidget::optionTableModelChanged, this, &OptionWidget::on_optionTableModelChanged);
    connect(mOptionTableModel, &GamsOptionTableModel::newTableRowDropped, this, &OptionWidget::on_newTableRowDropped);
    connect(mOptionTableModel, &GamsOptionTableModel::optionNameChanged, this, &OptionWidget::on_optionTableNameChanged);

    QSortFilterProxyModel* proxymodel = new OptionSortFilterProxyModel(this);
    GamsOptionDefinitionModel* optdefmodel =  new GamsOptionDefinitionModel(mOptionTokenizer->getOption(), 0, this);
    proxymodel->setFilterKeyColumn(-1);
    proxymodel->setSourceModel( optdefmodel );
    proxymodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxymodel->setSortCaseSensitivity(Qt::CaseInsensitive);
    connect(ui->gamsOptionSearch, &QLineEdit::textChanged,
            proxymodel, static_cast<void(QSortFilterProxyModel::*)(const QString &)>(&QSortFilterProxyModel::setFilterRegExp));

    ui->gamsOptionTreeView->setModel( proxymodel );
    ui->gamsOptionTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->gamsOptionTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->gamsOptionTreeView->setDragEnabled(true);
    ui->gamsOptionTreeView->setDragDropMode(QAbstractItemView::DragOnly);

    ui->gamsOptionTreeView->setItemsExpandable(true);
    ui->gamsOptionTreeView->setSortingEnabled(true);
    ui->gamsOptionTreeView->sortByColumn(0, Qt::AscendingOrder);
    ui->gamsOptionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_OPTION_NAME);
    ui->gamsOptionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_SYNONYM);
    ui->gamsOptionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_DEF_VALUE);
    ui->gamsOptionTreeView->setAlternatingRowColors(true);
    ui->gamsOptionTreeView->setExpandsOnDoubleClick(false);
    ui->gamsOptionTreeView->setColumnHidden(OptionDefinitionModel::COLUMN_ENTRY_NUMBER, true);
    ui->gamsOptionTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->gamsOptionTreeView, &QAbstractItemView::doubleClicked, this, &OptionWidget::addOptionFromDefinition);
    connect(ui->gamsOptionTreeView, &QTreeView::customContextMenuRequested, this, &OptionWidget::showDefinitionContextMenu);

    connect(mOptionTableModel, &GamsOptionTableModel::optionModelChanged, optdefmodel, &GamsOptionDefinitionModel::modifyOptionDefinition);

    mExtendedEditor = new QDockWidget("GAMS Parameters", this);
    mExtendedEditor->setObjectName("gamsArguments");
    mExtendedEditor->setWidget(ui->gamsOptionWidget);
    mExtendedEditor->setFeatures(QDockWidget::NoDockWidgetFeatures);
    mExtendedEditor->setTitleBarWidget(new QWidget(this));
    main->addDockWidget(Qt::TopDockWidgetArea, mExtendedEditor);
    connect(mExtendedEditor, &QDockWidget::visibilityChanged, main, &MainWindow::setExtendedEditorVisibility);
    mExtendedEditor->setVisible(false);

#ifdef __APPLE__
    ui->verticalLayout->setContentsMargins(2,2,2,0);
#else
    ui->verticalLayout->setContentsMargins(2,0,2,2);
#endif
}

OptionWidget::~OptionWidget()
{
    delete ui;
    delete mOptionTokenizer;
}

void OptionWidget::runDefaultAction()
{
    ui->gamsRunToolButton->defaultAction()->trigger();
}

QString OptionWidget::on_runAction(RunActionState state)
{
    QString commandLineStr =  ui->gamsOptionCommandLine->getOptionString();

    if (!commandLineStr.endsWith(" "))
        commandLineStr.append(" ");

    bool gdxParam = commandLineStr.contains(QRegularExpression("gdx[= ]", QRegularExpression::CaseInsensitiveOption));
    bool actParam = commandLineStr.contains("ACTION=C",Qt::CaseInsensitive);

    if (state == RunActionState::RunWithGDXCreation && !gdxParam) {
       commandLineStr.append("GDX=default");
       ui->gamsRunToolButton->setDefaultAction( actionRun_with_GDX_Creation );

    } else if (state == RunActionState::Compile && !actParam) {
        commandLineStr.append("ACTION=C");
        ui->gamsRunToolButton->setDefaultAction( actionCompile );

    } else if (state == RunActionState::CompileWithGDXCreation) {
        if (!gdxParam) commandLineStr.append("GDX=default ");
        if (!actParam) commandLineStr.append("ACTION=C");
        ui->gamsRunToolButton->setDefaultAction( actionCompile_with_GDX_Creation );

    } else {
        ui->gamsRunToolButton->setDefaultAction( actionRun );
    }

    return commandLineStr.simplified();
}

void OptionWidget::on_interruptAction()
{
    ui->gamsInterruptToolButton->setDefaultAction( actionInterrupt );
}

void OptionWidget::on_stopAction()
{
    ui->gamsInterruptToolButton->setDefaultAction( actionStop );
}

void OptionWidget::updateOptionTableModel(QLineEdit *lineEdit, const QString &commandLineStr)
{
    Q_UNUSED(lineEdit);
    if (mExtendedEditor->isHidden()) return;

    emit optionTableModelChanged(commandLineStr);
}

void OptionWidget::updateCommandLineStr(const QList<OptionItem> &optionItems)
{
    if (ui->gamsOptionWidget->isHidden())
       return;

    emit commandLineOptionChanged(ui->gamsOptionCommandLine->lineEdit(), optionItems);
}

void OptionWidget::showOptionContextMenu(const QPoint &pos)
{
    QModelIndexList indexSelection = ui->gamsOptionTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->gamsOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    QModelIndexList selection = ui->gamsOptionTableView->selectionModel()->selectedRows();
    bool thereIsARowSelection = (selection.count() > 0);
    bool thereIsARow = (ui->gamsOptionTableView->model()->rowCount() > 0);

    QMenu menu(this);
    for(QAction* action : ui->gamsOptionTableView->actions()) {
        if (action->objectName().compare("actionInsert_option")==0) {
            action->setEnabled( !thereIsARow || thereIsARowSelection );
            menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionDelete_option")==0) {
            action->setEnabled( thereIsARowSelection );
            menu.addAction(action);
        } else if (action->objectName().compare("actionDeleteAll_option")==0) {
            action->setEnabled( ui->gamsOptionTableView->model()->rowCount() > 0 );
            menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionMoveUp_option")==0) {
            action->setEnabled( thereIsARowSelection && (selection.first().row() > 0) );
            menu.addAction(action);
        } else if (action->objectName().compare("actionMoveDown_option")==0) {
            action->setEnabled( thereIsARowSelection && (selection.last().row() < mOptionTableModel->rowCount()-1) );
            menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionSelect_all")==0) {
            action->setEnabled( thereIsARow );
            menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionShowDefinition_option")==0) {

            bool thereIsAnOptionSelection = false;
            for (QModelIndex s : selection) {
                QVariant data = ui->gamsOptionTableView->model()->headerData(s.row(), Qt::Vertical,  Qt::CheckStateRole);
                if (Qt::CheckState(data.toUInt())!=Qt::PartiallyChecked) {
                    thereIsAnOptionSelection = true;
                    break;
                }
            }
            action->setEnabled( thereIsAnOptionSelection );
            menu.addAction(action);
        } else if (action->objectName().compare("actionResize_columns")==0) {
            action->setEnabled( thereIsARow );
            menu.addAction(action);
            menu.addSeparator();
        }
    }
    menu.exec(ui->gamsOptionTableView->viewport()->mapToGlobal(pos));
}

void OptionWidget::showDefinitionContextMenu(const QPoint &pos)
{
    QModelIndexList selection = ui->gamsOptionTreeView->selectionModel()->selectedRows();
    if (selection.count() <= 0)
        return;

    bool hasSelectionBeenAdded = (selection.size()>0);
    // assume single selection
    for (QModelIndex idx : selection) {
        QModelIndex parentIdx = ui->gamsOptionTreeView->model()->parent(idx);
        QVariant data = (parentIdx.row() < 0) ? ui->gamsOptionTreeView->model()->data(idx, Qt::CheckStateRole)
                                              : ui->gamsOptionTreeView->model()->data(parentIdx, Qt::CheckStateRole);
        hasSelectionBeenAdded = (Qt::CheckState(data.toInt()) == Qt::Checked);
    }

    QMenu menu(this);
    for(QAction* action : ui->gamsOptionTreeView->actions()) {
        if (action->objectName().compare("actionFindThisOption")==0) {
            action->setEnabled(  hasSelectionBeenAdded );
            menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionAddThisOption")==0) {
            action->setEnabled( !hasSelectionBeenAdded );
            menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionDeleteThisOption")==0) {
            action->setEnabled( hasSelectionBeenAdded  );
            menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionResize_columns")==0) {
            action->setEnabled( ui->gamsOptionTreeView->model()->rowCount()>0 );
            menu.addAction(action);
            menu.addSeparator();
        }
    }

    menu.exec(ui->gamsOptionTreeView->viewport()->mapToGlobal(pos));

}

void OptionWidget::updateRunState(bool isRunnable, bool isRunning)
{
    bool activate = isRunnable && !isRunning;
    setRunActionsEnabled(activate);
    setInterruptActionsEnabled(isRunnable && isRunning);

    ui->gamsOptionWidget->setEnabled(activate);
    ui->gamsOptionCommandLine->setEnabled(activate && !isEditorExtended());
}

void OptionWidget::addOptionFromDefinition(const QModelIndex &index)
{
    QModelIndex parentIndex =  ui->gamsOptionTreeView->model()->parent(index);
    QModelIndex optionNameIndex = (parentIndex.row()<0) ? ui->gamsOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) :
                                                          ui->gamsOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ;
    QModelIndex synonymIndex = (parentIndex.row()<0) ? ui->gamsOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_SYNONYM) :
                                                       ui->gamsOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_SYNONYM) ;
    QModelIndex defValueIndex = (parentIndex.row()<0) ? ui->gamsOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_DEF_VALUE) :
                                                        ui->gamsOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_DEF_VALUE) ;
    QModelIndex entryNumberIndex = (parentIndex.row()<0) ? ui->gamsOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER) :
                                                           ui->gamsOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER) ;
    QModelIndex selectedValueIndex = (parentIndex.row()<0) ? defValueIndex : index ;

    QString optionNameData = ui->gamsOptionTreeView->model()->data(optionNameIndex).toString();
    QString synonymData = ui->gamsOptionTreeView->model()->data(synonymIndex).toString();
    QString selectedValueData = ui->gamsOptionTreeView->model()->data(selectedValueIndex).toString();
    QString entryNumberData = ui->gamsOptionTreeView->model()->data(entryNumberIndex).toString();

    mOptionTokenizer->getOption()->setModified(optionNameData, true);
    ui->gamsOptionTreeView->model()->setData(optionNameIndex, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);

    int i;
    for(i=0; i < ui->gamsOptionTableView->model()->rowCount(); ++i) {
        QModelIndex idx = ui->gamsOptionTableView->model()->index(i, 0, QModelIndex());
        QString optionName = ui->gamsOptionTableView->model()->data(idx, Qt::DisplayRole).toString();
        if ((QString::compare(optionNameData, optionName, Qt::CaseInsensitive)==0) ||
            (QString::compare(synonymData, optionName, Qt::CaseInsensitive)==0))
            break;
    }
    if (i < ui->gamsOptionTableView->model()->rowCount()) {
        ui->gamsOptionTableView->model()->setData( ui->gamsOptionTableView->model()->index(i, 1), selectedValueData, Qt::EditRole);
        ui->gamsOptionTableView->selectRow(i);
        return;
    }

    ui->gamsOptionTableView->model()->insertRows(ui->gamsOptionTableView->model()->rowCount(), 1, QModelIndex());
    QModelIndex insertKeyIndex = ui->gamsOptionTableView->model()->index(ui->gamsOptionTableView->model()->rowCount()-1, 0);
    QModelIndex insertValueIndex = ui->gamsOptionTableView->model()->index(ui->gamsOptionTableView->model()->rowCount()-1, 1);
    QModelIndex insertEntryIndex = ui->gamsOptionTableView->model()->index(ui->gamsOptionTableView->model()->rowCount()-1, 2);
    ui->gamsOptionTableView->model()->setData( insertKeyIndex, optionNameData, Qt::EditRole);
    ui->gamsOptionTableView->model()->setData( insertValueIndex, selectedValueData, Qt::EditRole);
    ui->gamsOptionTableView->model()->setData( insertEntryIndex, entryNumberData, Qt::EditRole);
    ui->gamsOptionTableView->selectionModel()->select( mOptionTableModel->index(ui->gamsOptionTableView->model()->rowCount()-1, 0),
                                                       QItemSelectionModel::Select|QItemSelectionModel::Rows );
    if (parentIndex.row()<0)
        ui->gamsOptionTableView->edit(insertValueIndex);
}

void OptionWidget::loadCommandLineOption(const QStringList &history)
{
    // disconnect
    disconnect(ui->gamsOptionCommandLine, &QComboBox::editTextChanged,
            ui->gamsOptionCommandLine, &CommandLineOption::validateChangedOption);
    disconnect(ui->gamsOptionCommandLine, &CommandLineOption::commandLineOptionChanged,
            mOptionTokenizer, &OptionTokenizer::formatTextLineEdit);
    disconnect(ui->gamsOptionCommandLine, &CommandLineOption::commandLineOptionChanged,
            this, &OptionWidget::updateOptionTableModel );
    disconnect(mOptionTableModel, &GamsOptionTableModel::optionModelChanged, this, static_cast<void(OptionWidget::*)(const QList<OptionItem> &)> (&OptionWidget::updateCommandLineStr));
    disconnect(this, static_cast<void(OptionWidget::*)(QLineEdit*, const QList<OptionItem> &)>(&OptionWidget::commandLineOptionChanged), mOptionTokenizer, &OptionTokenizer::formatItemLineEdit);

    ui->gamsOptionTreeView->clearSelection();
    ui->gamsOptionTreeView->collapseAll();
    ui->gamsOptionCommandLine->clear();
    for (QString str: history) {
        ui->gamsOptionCommandLine->insertItem(0, str );
    }

    connect(ui->gamsOptionCommandLine, &QComboBox::editTextChanged,
            ui->gamsOptionCommandLine, &CommandLineOption::validateChangedOption);
    connect(ui->gamsOptionCommandLine, &CommandLineOption::commandLineOptionChanged,
            mOptionTokenizer, &OptionTokenizer::formatTextLineEdit);
    connect(ui->gamsOptionCommandLine, &CommandLineOption::commandLineOptionChanged,
            this, &OptionWidget::updateOptionTableModel );
    connect(mOptionTableModel, &GamsOptionTableModel::optionModelChanged, this, static_cast<void(OptionWidget::*)(const QList<OptionItem> &)> (&OptionWidget::updateCommandLineStr));
    connect(this, static_cast<void(OptionWidget::*)(QLineEdit*, const QList<OptionItem> &)>(&OptionWidget::commandLineOptionChanged), mOptionTokenizer, &OptionTokenizer::formatItemLineEdit);

    if (history.isEmpty()) {
        ui->gamsOptionCommandLine->insertItem(0, " ");
        ui->gamsOptionTreeView->clearSelection();
        ui->gamsOptionTreeView->collapseAll();
    }
    ui->gamsOptionCommandLine->setCurrentIndex(0);
}

void OptionWidget::selectSearchField()
{
    ui->gamsOptionSearch->setFocus();
}

void OptionWidget::optionItemCommitted(QWidget *editor)
{
    Q_UNUSED(editor);
    if (mOptionCompleter->currentEditedIndex().isValid()) {
        ui->gamsOptionTableView->selectionModel()->select( mOptionCompleter->currentEditedIndex(), QItemSelectionModel::ClearAndSelect );
        ui->gamsOptionTableView->setCurrentIndex( mOptionCompleter->currentEditedIndex() );
        ui->gamsOptionTableView->setFocus();
    }
}

void OptionWidget::findAndSelectionOptionFromDefinition()
{
    ui->gamsOptionTableView->selectionModel()->clearSelection();
    QModelIndex index = ui->gamsOptionTreeView->selectionModel()->currentIndex();
    QModelIndex parentIndex =  ui->gamsOptionTreeView->model()->parent(index);

    QModelIndex idx = (parentIndex.row()<0) ? ui->gamsOptionTreeView->model()->index( index.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER )
                                            : ui->gamsOptionTreeView->model()->index( parentIndex.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER );
    QVariant data = ui->gamsOptionTreeView->model()->data( idx, Qt::DisplayRole );
    QModelIndexList indices = ui->gamsOptionTableView->model()->match(ui->gamsOptionTableView->model()->index(0, GamsOptionTableModel::COLUMN_ENTRY_NUMBER),
                                                                       Qt::DisplayRole,
                                                                       data, -1, Qt::MatchExactly|Qt::MatchRecursive);
    ui->gamsOptionTableView->clearSelection();
    QItemSelection selection;
    for(QModelIndex i :indices) {
        QModelIndex leftIndex  = ui->gamsOptionTableView->model()->index(i.row(), 0);
        QModelIndex rightIndex = ui->gamsOptionTableView->model()->index(i.row(), ui->gamsOptionTableView->model()->columnCount() -1);

        QItemSelection rowSelection(leftIndex, rightIndex);
        selection.merge(rowSelection, QItemSelectionModel::Select);
    }

    ui->gamsOptionTableView->selectionModel()->select(selection, QItemSelectionModel::Select);
}

void OptionWidget::showOptionDefinition()
{
   if (!mExtendedEditor->isVisible() || !ui->gamsOptionTableView->hasFocus())
       return;

    QModelIndexList indexSelection = ui->gamsOptionTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->gamsOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    QModelIndexList selection = ui->gamsOptionTableView->selectionModel()->selectedRows();

    if (selection.count() <= 0)
       return;

    ui->gamsOptionTreeView->selectionModel()->clearSelection();
    for (int i=0; i<selection.count(); i++) {
            QModelIndex index = selection.at(i);
            if (Qt::CheckState(ui->gamsOptionTableView->model()->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole).toUInt())==Qt::PartiallyChecked)
                continue;

            QVariant optionId = ui->gamsOptionTableView->model()->data( index.sibling(index.row(), ui->gamsOptionTableView->model()->columnCount()-1), Qt::DisplayRole);
            QModelIndexList indices = ui->gamsOptionTreeView->model()->match(ui->gamsOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_ENTRY_NUMBER),
                                                                               Qt::DisplayRole,
                                                                               optionId, 1, Qt::MatchExactly|Qt::MatchRecursive);
            for(QModelIndex idx : indices) {
                QModelIndex  parentIndex =  ui->gamsOptionTreeView->model()->parent(index);

                if (parentIndex.row() < 0 && !ui->gamsOptionTreeView->isExpanded(idx))
                    ui->gamsOptionTreeView->expand(idx);
                QItemSelection selection = ui->gamsOptionTreeView->selectionModel()->selection();
                selection.select(ui->gamsOptionTreeView->model()->index(idx.row(), 0),
                                 ui->gamsOptionTreeView->model()->index(idx.row(), ui->gamsOptionTreeView->model()->columnCount()-1));
                ui->gamsOptionTreeView->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
            }
            if (indices.size() > 0) {
                ui->gamsOptionTreeView->scrollTo(indices.first(), QAbstractItemView::EnsureVisible);
                const QRect r = ui->gamsOptionTreeView->visualRect(indices.first());
                ui->gamsOptionTreeView->horizontalScrollBar()->setValue(r.x());
            }
    }
}

void OptionWidget::deleteOption()
{
    if (!mExtendedEditor->isVisible())
        return;

     QModelIndexList indexSelection = ui->gamsOptionTableView->selectionModel()->selectedIndexes();
     for(QModelIndex index : indexSelection) {
         ui->gamsOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
     }

     QModelIndexList selection = ui->gamsOptionTableView->selectionModel()->selectedRows();

     if (selection.count() <= 0)
        return;

    QModelIndex index = selection.at(0);
    QModelIndex removeTableIndex = ui->gamsOptionTableView->model()->index(index.row(), 0);
    QVariant optionName = ui->gamsOptionTableView->model()->data(removeTableIndex, Qt::DisplayRole);

    QModelIndexList items = ui->gamsOptionTableView->model()->match(ui->gamsOptionTableView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     optionName, -1);
    QModelIndexList definitionItems = ui->gamsOptionTreeView->model()->match(ui->gamsOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     optionName, 1);

    ui->gamsOptionTableView->model()->removeRow(index.row(), QModelIndex());

    if (items.size() <= 1) {  // only set Unchecked if it's the only optionName in the table
        mOptionTokenizer->getOption()->setModified(optionName.toString(), false);
       for(QModelIndex item : definitionItems) {
           ui->gamsOptionTreeView->model()->setData(item, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole);
       }
    }

}

void OptionWidget::deleteAllOptions()
{
    if (!mExtendedEditor->isVisible() || !ui->gamsOptionTableView->hasFocus())
        return;

    mOptionTokenizer->getOption()->resetModficationFlag();

    QModelIndexList items = ui->gamsOptionTreeView->model()->match(ui->gamsOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::CheckStateRole,
                                                                     Qt::CheckState(Qt::Checked),
                                                                     ui->gamsOptionTreeView->model()->rowCount());
    for(QModelIndex item : items) {
        ui->gamsOptionTreeView->model()->setData(item, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole);
    }
    ui->gamsOptionTableView->model()->removeRows(0, ui->gamsOptionTableView->model()->rowCount(), QModelIndex());

    emit optionTableModelChanged("");
}

void OptionWidget::insertOption()
{
    if (!mExtendedEditor->isVisible())
        return;

    QModelIndexList indexSelection = ui->gamsOptionTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->gamsOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    QModelIndexList selection = ui->gamsOptionTableView->selectionModel()->selectedRows();

    if (mOptionTableModel->rowCount() <= 0 || selection.count() <= 0) {
        mOptionTableModel->insertRows(mOptionTableModel->rowCount(), 1, QModelIndex());
        QModelIndex index = mOptionTableModel->index( mOptionTableModel->rowCount()-1, GamsOptionTableModel::COLUMN_OPTION_KEY);
        ui->gamsOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
        ui->gamsOptionTableView->edit( index );

        ui->gamsOptionTableView->scrollTo(index, QAbstractItemView::EnsureVisible);
    } else if (selection.count() > 0) {
        QModelIndex index = selection.at(0);
        ui->gamsOptionTableView->model()->insertRows(index.row(), 1, QModelIndex());
        ui->gamsOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
        ui->gamsOptionTableView->edit( mOptionTableModel->index(index.row(), GamsOptionTableModel::COLUMN_OPTION_KEY) );

        ui->gamsOptionTableView->scrollTo(index, QAbstractItemView::EnsureVisible);
    }
}

void OptionWidget::moveOptionUp()
{
    QModelIndexList indexSelection = ui->gamsOptionTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->gamsOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    QModelIndexList selection = ui->gamsOptionTableView->selectionModel()->selectedRows();
    if (selection.count() <= 0 || (selection.first().row() <= 0))
       return;

    QModelIndex index = selection.at(0);
    ui->gamsOptionTableView->model()->moveRows(QModelIndex(), index.row(), 1, QModelIndex(), index.row()-1);
    ui->gamsOptionTableView->selectionModel()->select( mOptionTableModel->index(index.row()-1, 0),
                                                       QItemSelectionModel::Select|QItemSelectionModel::Rows );
}

void OptionWidget::moveOptionDown()
{
    QModelIndexList indexSelection = ui->gamsOptionTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->gamsOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    QModelIndexList selection = ui->gamsOptionTableView->selectionModel()->selectedRows();

    if (selection.count() <= 0 || (selection.last().row() >= mOptionTableModel->rowCount()-1) )
        return;

    QModelIndex index = selection.at(0);
    ui->gamsOptionTableView->model()->moveRows(QModelIndex(), index.row(), 1, QModelIndex(), index.row()+2);
    ui->gamsOptionTableView->selectionModel()->select( mOptionTableModel->index(index.row()+1, 0),
                                                       QItemSelectionModel::Select|QItemSelectionModel::Rows );
}

void OptionWidget::setEditorExtended(bool extended)
{
    if (extended) {
        ui->gamsOptionTreeView->clearSelection();
        ui->gamsOptionTreeView->collapseAll();
        emit optionTableModelChanged(ui->gamsOptionCommandLine->currentText());
    }
    mExtendedEditor->setVisible(extended);
    main->updateRunState();
    ui->gamsOptionCommandLine->setEnabled(!extended);
}

bool OptionWidget::isEditorExtended()
{
    return mExtendedEditor->isVisible();
}

void OptionWidget::on_newTableRowDropped(const QModelIndex &index)
{
    ui->gamsOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );

    QString optionName = ui->gamsOptionTableView->model()->data(index, Qt::DisplayRole).toString();
    QModelIndexList definitionItems = ui->gamsOptionTreeView->model()->match(ui->gamsOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     optionName, 1);
    mOptionTokenizer->getOption()->setModified(optionName, true);
    for(QModelIndex item : definitionItems) {
        ui->gamsOptionTreeView->model()->setData(item, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);
    }

    if (mOptionTokenizer->getOption()->getOptionType(optionName) != optTypeEnumStr &&
        mOptionTokenizer->getOption()->getOptionType(optionName) != optTypeEnumInt &&
        mOptionTokenizer->getOption()->getOptionSubType(optionName) != optsubNoValue)
        ui->gamsOptionTableView->edit( mOptionTableModel->index(index.row(), GamsOptionTableModel::COLUMN_OPTION_VALUE) );
}

void OptionWidget::on_optionTableNameChanged(const QString &from, const QString &to)
{
    if (QString::compare(from, to, Qt::CaseInsensitive)==0)
        return;

    QModelIndexList fromDefinitionItems = ui->gamsOptionTreeView->model()->match(ui->gamsOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     from, 1);
    if (fromDefinitionItems.size() <= 0) {
        fromDefinitionItems = ui->gamsOptionTreeView->model()->match(ui->gamsOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_SYNONYM),
                                                                         Qt::DisplayRole,
                                                                         from, 1);
    }
    for(QModelIndex item : fromDefinitionItems) {
        QModelIndex index = ui->gamsOptionTreeView->model()->index(item.row(), OptionDefinitionModel::COLUMN_OPTION_NAME);
        ui->gamsOptionTreeView->model()->setData(index, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole);
    }

    QModelIndexList toDefinitionItems = ui->gamsOptionTreeView->model()->match(ui->gamsOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     to, 1);
    if (toDefinitionItems.size() <= 0) {
        toDefinitionItems = ui->gamsOptionTreeView->model()->match(ui->gamsOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_SYNONYM),
                                                                         Qt::DisplayRole,
                                                                         to, 1);
    }
    for(QModelIndex item : toDefinitionItems) {
        QModelIndex index = ui->gamsOptionTreeView->model()->index(item.row(), OptionDefinitionModel::COLUMN_OPTION_NAME);
        ui->gamsOptionTreeView->model()->setData(index, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);
    }
}

void OptionWidget::on_optionTableModelChanged(const QString &commandLineStr)
{
    disconnect(ui->gamsOptionCommandLine, &CommandLineOption::commandLineOptionChanged,
            this, &OptionWidget::updateOptionTableModel );
    disconnect(mOptionTableModel, &GamsOptionTableModel::optionModelChanged, this, static_cast<void(OptionWidget::*)(const QList<OptionItem> &)> (&OptionWidget::updateCommandLineStr));
    disconnect(this, static_cast<void(OptionWidget::*)(QLineEdit*, const QList<OptionItem> &)>(&OptionWidget::commandLineOptionChanged), mOptionTokenizer, &OptionTokenizer::formatItemLineEdit);

    mOptionTableModel->on_optionTableModelChanged(commandLineStr);

    connect(ui->gamsOptionCommandLine, &CommandLineOption::commandLineOptionChanged,
            this, &OptionWidget::updateOptionTableModel );
    connect(mOptionTableModel, &GamsOptionTableModel::optionModelChanged, this, static_cast<void(OptionWidget::*)(const QList<OptionItem> &)> (&OptionWidget::updateCommandLineStr));
    connect(this, static_cast<void(OptionWidget::*)(QLineEdit*, const QList<OptionItem> &)>(&OptionWidget::commandLineOptionChanged), mOptionTokenizer, &OptionTokenizer::formatItemLineEdit);
}

void OptionWidget::resizeColumnsToContents()
{
    if (ui->gamsOptionTableView->hasFocus()) {
        if (ui->gamsOptionTableView->model()->rowCount()<=0)
            return;
        ui->gamsOptionTableView->resizeColumnToContents(GamsOptionTableModel::COLUMN_OPTION_KEY);
        ui->gamsOptionTableView->resizeColumnToContents(GamsOptionTableModel::COLUMN_OPTION_VALUE);
    } else  if (ui->gamsOptionTreeView->hasFocus()) {
        ui->gamsOptionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_OPTION_NAME);
        ui->gamsOptionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_SYNONYM);
        ui->gamsOptionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_DEF_VALUE);
        ui->gamsOptionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_RANGE);
        ui->gamsOptionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_TYPE);
    }
}

void OptionWidget::setRunsActionGroup(QAction *aRun, QAction *aRunGDX, QAction *aCompile, QAction *aCompileGDX)
{
    actionRun = aRun;
    actionCompile = aCompile;
    actionRun_with_GDX_Creation = aRunGDX;
    actionCompile_with_GDX_Creation = aCompileGDX;

    QMenu* runMenu = new QMenu;
    runMenu->addAction(actionRun);
    runMenu->addAction(actionRun_with_GDX_Creation);
    runMenu->addSeparator();
    runMenu->addAction(actionCompile);
    runMenu->addAction(actionCompile_with_GDX_Creation);
    actionRun->setShortcutVisibleInContextMenu(true);
    actionRun_with_GDX_Creation->setShortcutVisibleInContextMenu(true);
    actionCompile->setShortcutVisibleInContextMenu(true);
    actionCompile_with_GDX_Creation->setShortcutVisibleInContextMenu(true);

    ui->gamsRunToolButton->setMenu(runMenu);
    ui->gamsRunToolButton->setDefaultAction(actionRun);
}

void OptionWidget::setInterruptActionGroup(QAction *aInterrupt, QAction *aStop)
{
    actionInterrupt = aInterrupt;
    actionInterrupt->setShortcutVisibleInContextMenu(true);
    actionStop = aStop;
    actionStop->setShortcutVisibleInContextMenu(true);

    QMenu* interruptMenu = new QMenu();
    interruptMenu->addAction(actionInterrupt);
    interruptMenu->addAction(actionStop);

    ui->gamsInterruptToolButton->setMenu(interruptMenu);
    ui->gamsInterruptToolButton->setDefaultAction(actionInterrupt);
}

void OptionWidget::setRunActionsEnabled(bool enable)
{
    actionRun->setEnabled(enable);
    actionRun_with_GDX_Creation->setEnabled(enable);
    actionCompile->setEnabled(enable);
    actionCompile_with_GDX_Creation->setEnabled(enable);
    ui->gamsRunToolButton->menu()->setEnabled(enable);
}

void OptionWidget::setInterruptActionsEnabled(bool enable)
{
    actionInterrupt->setEnabled(enable);
    actionStop->setEnabled(enable);
    ui->gamsInterruptToolButton->menu()->setEnabled(enable);
}

void OptionWidget::addActions()
{
//    QAction* selectAll = mContextMenu.addAction("Select All", [this]() { ui->gamsOptionTableView->selectAll(); });
//    selectAll->setObjectName("actionSelect_all");
//    selectAll->setShortcut( QKeySequence("Ctrl+A") );
//    selectAll->setShortcutVisibleInContextMenu(true);
//    selectAll->setShortcutContext(Qt::WidgetShortcut);
//    ui->gamsOptionTableView->addAction(selectAll);

    QAction* insertOptionAction = mContextMenu.addAction(QIcon(":/img/insert"), "insert new Option", [this]() { insertOption(); });
    insertOptionAction->setObjectName("actionInsert_option");
    insertOptionAction->setShortcut( QKeySequence("Ctrl+Insert") );
    insertOptionAction->setShortcutVisibleInContextMenu(true);
    insertOptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsOptionTableView->addAction(insertOptionAction);

    QAction* deleteAction = mContextMenu.addAction(QIcon(":/img/delete-all"), "delete Selection", [this]() { deleteOption(); });
    deleteAction->setObjectName("actionDelete_option");
    deleteAction->setShortcut( QKeySequence("Ctrl+Delete") );
    deleteAction->setShortcutVisibleInContextMenu(true);
    deleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsOptionTableView->addAction(deleteAction);

    QAction* deleteAllAction = mContextMenu.addAction(QIcon(":/img/delete-all"), "delete All Options", [this]() { deleteAllOptions(); });
    deleteAllAction->setObjectName("actionDeleteAll_option");
    deleteAllAction->setShortcut( QKeySequence("Alt+Delete") );
    deleteAllAction->setShortcutVisibleInContextMenu(true);
    deleteAllAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsOptionTableView->addAction(deleteAllAction);

    QAction* moveUpAction = mContextMenu.addAction(QIcon(":/img/move-up"), "move Up", [this]() { moveOptionUp(); });
    moveUpAction->setObjectName("actionMoveUp_option");
    moveUpAction->setShortcut( QKeySequence("Ctrl+Up") );
    moveUpAction->setShortcutVisibleInContextMenu(true);
    moveUpAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsOptionTableView->addAction(moveUpAction);

    QAction* moveDownAction = mContextMenu.addAction(QIcon(":/img/move-down"), "move Down", [this]() { moveOptionDown(); });
    moveDownAction->setObjectName("actionMoveDown_option");
    moveDownAction->setShortcut( QKeySequence("Ctrl+Down") );
    moveDownAction->setShortcutVisibleInContextMenu(true);
    moveDownAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsOptionTableView->addAction(moveDownAction);

    QAction* showDefinitionAction = mContextMenu.addAction("show Option Definition", [this]() { showOptionDefinition(); });
    showDefinitionAction->setObjectName("actionShowDefinition_option");
    showDefinitionAction->setShortcut( QKeySequence("Ctrl+F1") );
    showDefinitionAction->setShortcutVisibleInContextMenu(true);
    showDefinitionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsOptionTableView->addAction(showDefinitionAction);

    QAction* findThisOptionAction = mContextMenu.addAction("show Selection of This Option", [this]() {
        findAndSelectionOptionFromDefinition();
    });
    findThisOptionAction->setObjectName("actionFindThisOption");
    findThisOptionAction->setShortcut( QKeySequence("Ctrl+Shift+F1") );
    findThisOptionAction->setShortcutVisibleInContextMenu(true);
    findThisOptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsOptionTreeView->addAction(findThisOptionAction);

    QAction* addThisOptionAction = mContextMenu.addAction(QIcon(":/img/plus"), "add This Option", [this]() {
        QModelIndexList selection = ui->gamsOptionTreeView->selectionModel()->selectedRows();
        if (selection.size()>0)
            addOptionFromDefinition(selection.at(0));
    });
    addThisOptionAction->setObjectName("actionAddThisOption");
    addThisOptionAction->setShortcut( QKeySequence("Ctrl+Shift+Insert") );
    addThisOptionAction->setShortcutVisibleInContextMenu(true);
    addThisOptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsOptionTreeView->addAction(addThisOptionAction);

    QAction* deleteThisOptionAction = mContextMenu.addAction(QIcon(":/img/delete-all"), "remove This Option", [this]() {
        findAndSelectionOptionFromDefinition();
        deleteOption();
    });
    deleteThisOptionAction->setObjectName("actionDeleteThisOption");
    deleteThisOptionAction->setShortcut( QKeySequence("Ctrl+Shift+Delete") );
    deleteThisOptionAction->setShortcutVisibleInContextMenu(true);
    deleteThisOptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsOptionTreeView->addAction(deleteThisOptionAction);

    QAction* resizeColumns = mContextMenu.addAction("Resize columns to contents", [this]() { resizeColumnsToContents(); });
    resizeColumns->setObjectName("actionResize_columns");
    resizeColumns->setShortcut( QKeySequence("Ctrl+R") );
    resizeColumns->setShortcutVisibleInContextMenu(true);
    resizeColumns->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsOptionTableView->addAction(resizeColumns);
    ui->gamsOptionTreeView->addAction(resizeColumns);
}

QDockWidget* OptionWidget::extendedEditor() const
{
    return mExtendedEditor;
}

OptionTokenizer *OptionWidget::getOptionTokenizer() const
{
    return mOptionTokenizer;
}

bool OptionWidget::isAnOptionWidgetFocused(QWidget *focusWidget) const
{
    return (focusWidget==ui->gamsOptionTableView || focusWidget==ui->gamsOptionTreeView || focusWidget==ui->gamsOptionCommandLine);
}

bool OptionWidget::isAnOptionTableFocused(QWidget *focusWidget) const
{
      return (focusWidget==ui->gamsOptionTableView);
}

QString OptionWidget::getSelectedOptionName(QWidget *widget) const
{
    QString selectedOptions = "";
    if (widget == ui->gamsOptionTableView) {
        QModelIndexList selection = ui->gamsOptionTableView->selectionModel()->selectedRows();
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            QVariant headerData = ui->gamsOptionTableView->model()->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole);
            if (Qt::CheckState(headerData.toUInt())==Qt::Checked) {
                return "";
            }
            QVariant data = ui->gamsOptionTableView->model()->data( index.sibling(index.row(),0) );
            if (mOptionTokenizer->getOption()->isDoubleDashedOption(data.toString())) {
               return "";
            } else if (mOptionTokenizer->getOption()->isASynonym(data.toString())) {
                return mOptionTokenizer->getOption()->getNameFromSynonym(data.toString());
            }
            return data.toString();
        }
    } else if (widget == ui->gamsOptionTreeView) {
        QModelIndexList selection = ui->gamsOptionTreeView->selectionModel()->selectedRows();
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            QModelIndex  parentIndex =  ui->gamsOptionTreeView->model()->parent(index);
            if (parentIndex.row() >= 0) {
                return ui->gamsOptionTreeView->model()->data( parentIndex.sibling(parentIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ).toString();
            } else {
                return ui->gamsOptionTreeView->model()->data( index.sibling(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ).toString();
            }
        }
    }
    return selectedOptions;
}

QString OptionWidget::getCurrentCommandLineData() const
{
    return ui->gamsOptionCommandLine->getOptionString();
}

void OptionWidget::focus()
{
    if (isEditorExtended())
        if (ui->gamsOptionTableView->hasFocus())
            ui->gamsOptionSearch->setFocus(Qt::ShortcutFocusReason);
        else if (ui->gamsOptionSearch->hasFocus())
                ui->gamsOptionTreeView->setFocus(Qt::ShortcutFocusReason);
        else
            ui->gamsOptionTableView->setFocus(Qt::TabFocusReason);
    else
        ui->gamsOptionCommandLine->setFocus(Qt::ShortcutFocusReason);
}

}
}
}
