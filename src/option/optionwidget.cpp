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
#include "optioncompleterdelegate.h"
#include "optionsortfilterproxymodel.h"
#include "gamsoptiondefinitionmodel.h"
#include "gamsoptiontablemodel.h"
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

    mOptionTokenizer = new OptionTokenizer(QString("optgams.def"));
//    mCommandLineHistory = new CommandLineHistory(this);

    setRunsActionGroup(actionRun, actionRun_with_GDX_Creation, actionCompile, actionCompile_with_GDX_Creation);
    setInterruptActionGroup(aInterrupt, actionStop);

    ui->gamsOptionWidget->hide();
    connect(ui->gamsOptionCommandLine, &CommandLineOption::optionRunChanged, main, &MainWindow::optionRunChanged);
    connect(ui->gamsOptionCommandLine, &QComboBox::editTextChanged, ui->gamsOptionCommandLine, &CommandLineOption::validateChangedOption);
    connect(ui->gamsOptionCommandLine, &CommandLineOption::commandLineOptionChanged, mOptionTokenizer, &OptionTokenizer::formatTextLineEdit);
    connect(ui->gamsOptionCommandLine, &CommandLineOption::commandLineOptionChanged, this, &OptionWidget::updateOptionTableModel );

    QList<OptionItem> optionItem = mOptionTokenizer->tokenize(ui->gamsOptionCommandLine->lineEdit()->text());
    QString normalizedText = mOptionTokenizer->normalize(optionItem);
    GamsOptionTableModel* optionTableModel = new GamsOptionTableModel(normalizedText, mOptionTokenizer,  this);
    ui->gamsOptionTableView->setModel( optionTableModel );
    connect(optionTableModel, &GamsOptionTableModel::optionModelChanged, this, static_cast<void(OptionWidget::*)(const QList<OptionItem> &)> (&OptionWidget::updateCommandLineStr));
    connect(this, static_cast<void(OptionWidget::*)(QLineEdit*, const QList<OptionItem> &)>(&OptionWidget::commandLineOptionChanged), mOptionTokenizer, &OptionTokenizer::formatItemLineEdit);

    ui->gamsOptionTableView->setItemDelegate( new OptionCompleterDelegate(mOptionTokenizer, ui->gamsOptionTableView) );
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
    ui->gamsOptionTableView->setAcceptDrops(true);
    ui->gamsOptionTableView->setDropIndicatorShown(true);
    ui->gamsOptionTableView->setDragDropMode(QAbstractItemView::DragDrop);
//    ui->gamsOptionTableView->setDragDropOverwriteMode(true);

    AddOptionHeaderView* headerView = new AddOptionHeaderView(Qt::Horizontal, ui->gamsOptionTableView);
    headerView->setSectionResizeMode(QHeaderView::Stretch);
    ui->gamsOptionTableView->setHorizontalHeader(headerView);
    ui->gamsOptionTableView->horizontalHeader()->setStretchLastSection(true);
    ui->gamsOptionTableView->resizeColumnToContents(0);
    ui->gamsOptionTableView->resizeColumnToContents(1);
    ui->gamsOptionTableView->setColumnHidden(2, true);

    ui->gamsOptionTableView->horizontalHeader()->setHighlightSections(false);
    ui->gamsOptionTableView->verticalHeader()->setDefaultSectionSize(ui->gamsOptionTableView->verticalHeader()->minimumSectionSize());

    connect(ui->gamsOptionTableView, &QTableView::customContextMenuRequested,this, &OptionWidget::showOptionContextMenu);
    connect(this, &OptionWidget::optionTableModelChanged, optionTableModel, &GamsOptionTableModel::on_optionTableModelChanged);
    connect(optionTableModel, &GamsOptionTableModel::newTableRowDropped, this, &OptionWidget::on_newTableRowDropped);
    connect(optionTableModel, &GamsOptionTableModel::optionNameChanged, this, &OptionWidget::on_optionTableNameChanged);

    QSortFilterProxyModel* proxymodel = new OptionSortFilterProxyModel(this);
    GamsOptionDefinitionModel* optdefmodel =  new GamsOptionDefinitionModel(mOptionTokenizer->getOption(), 0, this);
    proxymodel->setFilterKeyColumn(-1);
    proxymodel->setSourceModel( optdefmodel );
    proxymodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxymodel->setSortCaseSensitivity(Qt::CaseInsensitive);
    connect(ui->gamsOptionSearch, &QLineEdit::textChanged,
            proxymodel, static_cast<void(QSortFilterProxyModel::*)(const QString &)>(&QSortFilterProxyModel::setFilterRegExp));

    ui->gamsOptionTreeView->setModel( proxymodel );
    ui->gamsOptionTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->gamsOptionTreeView->setDragEnabled(true);
    ui->gamsOptionTreeView->setDragDropMode(QAbstractItemView::DragOnly);

    ui->gamsOptionTreeView->setItemsExpandable(true);
    ui->gamsOptionTreeView->setSortingEnabled(true);
    ui->gamsOptionTreeView->sortByColumn(0, Qt::AscendingOrder);
    ui->gamsOptionTreeView->resizeColumnToContents(0);
    ui->gamsOptionTreeView->resizeColumnToContents(2);
    ui->gamsOptionTreeView->resizeColumnToContents(3);
    ui->gamsOptionTreeView->setAlternatingRowColors(true);
    ui->gamsOptionTreeView->setExpandsOnDoubleClick(false);
    ui->gamsOptionTreeView->setColumnHidden(OptionDefinitionModel::COLUMN_ENTRY_NUMBER, true); // false);

    connect(ui->gamsOptionTreeView, &QAbstractItemView::doubleClicked, this, &OptionWidget::addOptionFromDefinition);
    connect(optionTableModel, &GamsOptionTableModel::optionModelChanged, optdefmodel, &GamsOptionDefinitionModel::modifyOptionDefinition);

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

    QMenu menu(this);
    QAction* addAction = menu.addAction(QIcon(":/img/plus"), "Add new option");
    QAction* insertAction = menu.addAction(QIcon(":/img/insert"), "Insert new option");
    menu.addSeparator();
    QAction* moveUpAction = menu.addAction(QIcon(":/img/move-up"), "move Up");
    QAction* moveDownAction = menu.addAction(QIcon(":/img/move-down"), "move Dwn");
    menu.addSeparator();
    QAction* deleteAction = menu.addAction(QIcon(":/img/delete"), "Remove selected option");
    menu.addSeparator();
    QAction* deleteAllActions = menu.addAction(QIcon(":/img/delete-all"), "Remove all options");

    if (ui->gamsOptionTableView->model()->rowCount() <= 0) {
        deleteAllActions->setVisible(false);
    }
    if (selection.count() <= 0) {
        insertAction->setVisible(false);
        deleteAction->setVisible(false);
        moveUpAction->setVisible(false);
        moveDownAction->setVisible(false);
    } else {
        QModelIndex index = selection.at(0);
        if (index.row()==0)
            moveUpAction->setVisible(false);
        if (index.row()+1 == ui->gamsOptionTableView->model()->rowCount())
            moveDownAction->setVisible(false);
    }

    QAction* action = menu.exec(ui->gamsOptionTableView->viewport()->mapToGlobal(pos));
    if (action == addAction) {
       ui->gamsOptionTableView->model()->insertRows(ui->gamsOptionTableView->model()->rowCount(), 1, QModelIndex());
       ui->gamsOptionTableView->selectRow(ui->gamsOptionTableView->model()->rowCount()-1);
    } else if (action == insertAction) {
            if (selection.count() > 0) {
                QModelIndex index = selection.at(0);
                ui->gamsOptionTableView->model()->insertRows(index.row(), 1, QModelIndex());
                ui->gamsOptionTableView->selectRow(index.row());
            }
    } else if (action == moveUpAction) {
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            ui->gamsOptionTableView->model()->moveRows(QModelIndex(), index.row(), 1, QModelIndex(), index.row()-1);
            ui->gamsOptionTableView->selectRow(index.row()-1);
        }

    } else if (action == moveDownAction) {
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            ui->gamsOptionTableView->model()->moveRows(QModelIndex(), index.row(), 1, QModelIndex(), index.row()+2);
            ui->gamsOptionTableView->selectRow(index.row()+1);
        }
    } else if (action == deleteAction) {
             if (selection.count() > 0) {
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
    } else if (action == deleteAllActions) {
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
    ui->gamsOptionTableView->selectRow(ui->gamsOptionTableView->model()->rowCount()-1);
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

    if (history.isEmpty())
        ui->gamsOptionCommandLine->insertItem(0, " ");
    ui->gamsOptionCommandLine->setCurrentIndex(0);
}

void OptionWidget::selectSearchField()
{
    ui->gamsOptionSearch->setFocus();
}

void OptionWidget::setEditorExtended(bool extended)
{
    if (extended) emit optionTableModelChanged(ui->gamsOptionCommandLine->currentText());
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
    QString optionName = ui->gamsOptionTableView->model()->data(index, Qt::DisplayRole).toString();
    QModelIndexList definitionItems = ui->gamsOptionTreeView->model()->match(ui->gamsOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     optionName, 1);
    mOptionTokenizer->getOption()->setModified(optionName, true);
    for(QModelIndex item : definitionItems) {
        ui->gamsOptionTreeView->model()->setData(item, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);
    }
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
    ui->gamsOptionCommandLine->setFocus(Qt::ShortcutFocusReason);
}

}
}
}
