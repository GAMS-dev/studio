/**
 * GAMS Model Instance Inspector (MII)
 *
 * Copyright (c) 2023-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2023-2024 GAMS Development Corp. <support@gams.com>
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
﻿/*
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
#include <QStandardItemModel>
#include <QShortcut>
#include <QMessageBox>
#include <QFileDialog>
#include <QClipboard>

#include "solveroptionwidget.h"
#include "ui_solveroptionwidget.h"

#include "addoptionheaderview.h"
#include "definitionitemdelegate.h"
#include "optionsortfilterproxymodel.h"
#include "solveroptiondefinitionmodel.h"
#include "headerviewproxy.h"
#include "mainwindow.h"
#include "editors/systemlogedit.h"
#include "settings.h"
#include "exception.h"

namespace gams {
namespace studio {
namespace option {

SolverOptionWidget::SolverOptionWidget(const QString &solverName, const QString &optionFilePath, const QString &optDefFileName,
                                       const FileId &id, const QString &encodingName, QWidget *parent) :
    AbstractView(parent),
    ui(new Ui::SolverOptionWidget),
    mFileId(id),
    mLocation(optionFilePath),
    mSolverName(solverName)
{
    ui->setupUi(this);
    mEncoding = encodingName.isEmpty() ? "UTF-8" : encodingName;
    setFocusProxy(ui->solverOptionTableView);
    addActions();

    init(optDefFileName);
}

SolverOptionWidget::~SolverOptionWidget()
{
    delete ui;
    delete mOptionTokenizer;
    delete mOptionCompleter;
    delete mOptionTableModel;
}

bool SolverOptionWidget::init(const QString &optDefFileName)
{
    mOptionTokenizer = new OptionTokenizer(optDefFileName);
    if (!mOptionTokenizer->getOption()->available())
       EXCEPT() << "Could not find or load OPT library for opening '" << mLocation << "'. Please check your GAMS installation.";

    SystemLogEdit* logEdit = new SystemLogEdit(this);
    mOptionTokenizer->provideLogger(logEdit);
    ui->solverOptionTabWidget->addTab( logEdit, "Messages" );

    QList<SolverOptionItem *> optionItem = mOptionTokenizer->readOptionFile(mLocation, mEncoding);
    mOptionTableModel = new SolverOptionTableModel(optionItem, mOptionTokenizer,  this);
    ui->solverOptionTableView->setModel( mOptionTableModel );
    updateTableColumnSpan();

    mOptionCompleter = new OptionCompleterDelegate(mOptionTokenizer, ui->solverOptionTableView);
    ui->solverOptionTableView->setItemDelegate( mOptionCompleter );
    ui->solverOptionTableView->setEditTriggers(QAbstractItemView::DoubleClicked
                       | QAbstractItemView::SelectedClicked
                       | QAbstractItemView::EditKeyPressed
                       | QAbstractItemView::AnyKeyPressed );
    ui->solverOptionTableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->solverOptionTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->solverOptionTableView->setAutoScroll(true);
    ui->solverOptionTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->solverOptionTableView->setSortingEnabled(false);

    ui->solverOptionTableView->viewport()->setAcceptDrops(true);
    ui->solverOptionTableView->setDropIndicatorShown(true);
    ui->solverOptionTableView->setDragDropMode(QAbstractItemView::DropOnly);
    ui->solverOptionTableView->setDragDropOverwriteMode(true);
    ui->solverOptionTableView->setDefaultDropAction(Qt::CopyAction);

    AddOptionHeaderView* headerView = new AddOptionHeaderView(Qt::Horizontal, ui->solverOptionTableView);
    headerView->setSectionResizeMode(QHeaderView::Interactive);
    QFontMetrics fm(QFont("times", 16));
    headerView->setMinimumSectionSize(fm.horizontalAdvance("Comment"));

    ui->solverOptionTableView->setHorizontalHeader(headerView);
    ui->solverOptionTableView->setColumnHidden( mOptionTableModel->getColumnEntryNumber(), true);
    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->solverOptionTableView->horizontalHeader()->setStyle(HeaderViewProxy::instance());

    ui->solverOptionTableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));
    if (ui->solverOptionTableView->model()->rowCount()<=0) {
        ui->solverOptionTableView->horizontalHeader()->setDefaultSectionSize( ui->solverOptionTableView->sizeHint().width()/(ui->solverOptionTableView->model()->columnCount()-1) );
    } else {
        ui->solverOptionTableView->resizeColumnsToContents();
    }
    ui->solverOptionTableView->horizontalHeader()->setStretchLastSection(true);
    ui->solverOptionTableView->horizontalHeader()->setHighlightSections(false);
    headerRegister(ui->solverOptionTableView->horizontalHeader());

    QList<OptionGroup> optionGroupList = mOptionTokenizer->getOption()->getOptionGroupList();
    int groupsize = 0;
    for(const OptionGroup &group : std::as_const(optionGroupList)) {
        if (group.hidden || group.name.compare("deprecated", Qt::CaseInsensitive)==0)
            continue;
        else
            ++groupsize;
    }

    QStandardItemModel* groupModel = new QStandardItemModel(groupsize+1, 3);
    int i = 0;
    groupModel->setItem(0, 0, new QStandardItem("--- All Options ---"));
    groupModel->setItem(0, 1, new QStandardItem("0"));
    groupModel->setItem(0, 2, new QStandardItem("All Options"));
    for(const OptionGroup &group : std::as_const(optionGroupList)) {
        if (group.hidden || group.name.compare("deprecated", Qt::CaseInsensitive)==0)
            continue;
        ++i;
        groupModel->setItem(i, 0, new QStandardItem(group.description));
        groupModel->setItem(i, 1, new QStandardItem(QString::number(group.number)));
        groupModel->setItem(i, 2, new QStandardItem(group.name));
    }
    ui->solverOptionGroup->setModel(groupModel);
    ui->solverOptionGroup->setModelColumn(0);

    OptionSortFilterProxyModel* proxymodel = new OptionSortFilterProxyModel(this);
    SolverOptionDefinitionModel* optdefmodel =  new SolverOptionDefinitionModel(mOptionTokenizer->getOption(), 0, this);
    proxymodel->setFilterKeyColumn(-1);
    proxymodel->setSourceModel( optdefmodel );
    proxymodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxymodel->setSortCaseSensitivity(Qt::CaseInsensitive);

    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->solverOptionTreeView->header()->setStyle(HeaderViewProxy::instance());
    ui->solverOptionTreeView->setModel( proxymodel );
    ui->solverOptionTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->solverOptionTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->solverOptionTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->solverOptionTreeView->setDragEnabled(true);
    ui->solverOptionTreeView->setDragDropMode(QAbstractItemView::DragOnly);

    ui->solverOptionTreeView->setItemDelegate( new DefinitionItemDelegate(ui->solverOptionTreeView) );
    ui->solverOptionTreeView->setItemsExpandable(true);
    ui->solverOptionTreeView->setSortingEnabled(true);
    ui->solverOptionTreeView->sortByColumn(0, Qt::AscendingOrder);
    ui->solverOptionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_OPTION_NAME);
    ui->solverOptionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_SYNONYM);
    ui->solverOptionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_DEF_VALUE);
    ui->solverOptionTreeView->setExpandsOnDoubleClick(false);
    if (!mOptionTokenizer->getOption()->isSynonymDefined())
        ui->solverOptionTreeView->setColumnHidden( 1, true);
    ui->solverOptionTreeView->setColumnHidden(OptionDefinitionModel::COLUMN_ENTRY_NUMBER, true);
    headerRegister(ui->solverOptionTreeView->header());

    ui->solverOptionHSplitter->setSizes(QList<int>({25, 75}));
    ui->solverOptionVSplitter->setSizes(QList<int>({75, 25}));

    setModified(false);

    ui->solverOptionTableView->setTabKeyNavigation(true);

    setTabOrder(ui->solverOptionGroup, ui->solverOptionSearch);
    setTabOrder(ui->solverOptionSearch, ui->solverOptionTreeView);
    setTabOrder(ui->solverOptionTreeView, ui->compactViewCheckBox);
    setTabOrder(ui->compactViewCheckBox, ui->openAsTextButton);
    setTabOrder(ui->openAsTextButton, ui->SolverOptionMessageWidget);

    if (!mOptionTokenizer->getOption()->available())  {
        ui->solverOptionSearch->setReadOnly(true);
        ui->compactViewCheckBox->setEnabled(false);

        QString msg1 = QString("Unable to open %1 Properly").arg(mLocation);
        QString msg2 = QString("'%1' is not a valid solver name").arg(mSolverName);
        mOptionTokenizer->logger()->append(QString("%1. %2").arg(msg1, msg2), LogMsgType::Error);
        mOptionTokenizer->logger()->append(QString("An operation on the file contents might not be saved. Try 'Save As' or 'Open As Text' instead."), LogMsgType::Warning);

        return false;
    }
    else {
        connect(ui->solverOptionTableView->verticalHeader(), &QHeaderView::sectionClicked, this, &SolverOptionWidget::on_selectAndToggleRow, Qt::UniqueConnection);
        connect(ui->solverOptionTableView, &QTableView::customContextMenuRequested, this, &SolverOptionWidget::showOptionContextMenu, Qt::UniqueConnection);
        connect(mOptionTableModel, &SolverOptionTableModel::newTableRowDropped, this, &SolverOptionWidget::on_newTableRowDropped, Qt::UniqueConnection);

        connect(ui->solverOptionSearch, &FilterLineEdit::regExpChanged, this, [this, proxymodel]() {
            proxymodel->setFilterRegularExpression(ui->solverOptionSearch->regExp());
            selectSearchField();
        });

        connect(ui->solverOptionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SolverOptionWidget::findAndSelectionOptionFromDefinition, Qt::UniqueConnection);
        connect(ui->solverOptionTreeView, &QAbstractItemView::doubleClicked, this, &SolverOptionWidget::addOptionFromDefinition);
        connect(ui->solverOptionTreeView, &QTreeView::customContextMenuRequested, this, &SolverOptionWidget::showDefinitionContextMenu, Qt::UniqueConnection);

        connect(ui->solverOptionGroup, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int index) {
            optdefmodel->loadOptionFromGroup( groupModel->data(groupModel->index(index, 1)).toInt() );
            mOptionTableModel->on_groupDefinitionReloaded();
        });

        connect(mOptionTableModel, &QAbstractTableModel::dataChanged, this, &SolverOptionWidget::on_dataItemChanged, Qt::UniqueConnection);
        connect(mOptionTableModel, &QAbstractTableModel::dataChanged, mOptionTableModel, &SolverOptionTableModel::on_updateSolverOptionItem, Qt::UniqueConnection);
        connect(mOptionTableModel, &SolverOptionTableModel::solverOptionItemModelChanged, mOptionTableModel, &SolverOptionTableModel::updateRecurrentStatus, Qt::UniqueConnection);
        connect(mOptionTableModel, &SolverOptionTableModel::solverOptionModelChanged, optdefmodel, &SolverOptionDefinitionModel::modifyOptionDefinition, Qt::UniqueConnection);
        connect(mOptionTableModel, &SolverOptionTableModel::solverOptionItemModelChanged, optdefmodel, &SolverOptionDefinitionModel::modifyOptionDefinitionItem, Qt::UniqueConnection);
        connect(mOptionTableModel, &SolverOptionTableModel::solverOptionItemRemoved, mOptionTableModel, &SolverOptionTableModel::on_removeSolverOptionItem, Qt::UniqueConnection);

        connect( mOptionCompleter, &OptionCompleterDelegate::closeEditor, this, &SolverOptionWidget::completeEditingOption, Qt::UniqueConnection );

        connect(this, &SolverOptionWidget::compactViewChanged, optdefmodel, &SolverOptionDefinitionModel::on_compactViewChanged, Qt::UniqueConnection);

        mOptionTokenizer->logger()->append(QString("Loading options from %1").arg(mLocation), LogMsgType::Info);
        return true;
    }
}

FileId SolverOptionWidget::fileId() const
{
    return mFileId;
}

bool SolverOptionWidget::isModified() const
{
    return mModified;
}

void SolverOptionWidget::showOptionContextMenu(const QPoint &pos)
{
    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();

    for(const QModelIndex &index : std::as_const(indexSelection)) {
        ui->solverOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }
    QModelIndexList selection = ui->solverOptionTableView->selectionModel()->selectedRows();
    bool thereIsARowSelection = isThereARowSelection();
    bool viewIsCompact = isViewCompact();
    bool thereIsAnOptionSelection = false;
    for (const QModelIndex &s : std::as_const(selection)) {
        QVariant data = ui->solverOptionTableView->model()->headerData(s.row(), Qt::Vertical,  Qt::CheckStateRole);
        if (Qt::CheckState(data.toUInt())!=Qt::PartiallyChecked) {
            thereIsAnOptionSelection = true;
            break;
        }
    }

    QMenu menu(this);
    if ( thereIsARowSelection ) {
        QList<QAction*> ret;
        getMainWindow()->getAdvancedActions(&ret);
        for(QAction *action : std::as_const(ret)) {
            if (action->objectName().compare("actionComment")==0) {
                menu.addAction(action);
                menu.addSeparator();
                break;
            }
        }
    }
    const auto actions = ui->solverOptionTableView->actions();
    for(QAction* action : actions) {
        if (action->objectName().compare("actionInsert_option")==0) {
            if ( !viewIsCompact && (!isThereARow() || isThereARowSelection()) )
                menu.addAction(action);
        } else if (action->objectName().compare("actionInsert_comment")==0) {
            if ( !viewIsCompact && (!isThereARow() || isThereARowSelection()) )
                menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionDelete_option")==0) {
            if ( thereIsARowSelection )
                menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionMoveUp_option")==0) {
            if ( !viewIsCompact && thereIsARowSelection && (selection.first().row() > 0) )
                menu.addAction(action);
        } else if (action->objectName().compare("actionMoveDown_option")==0) {
            if ( !viewIsCompact && thereIsARowSelection && (selection.last().row() < mOptionTableModel->rowCount()-1) )
                menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionSelect_all")==0) {
            if ( !viewIsCompact && isThereARow() )
                menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionShowDefinition_option")==0) {
            if ( thereIsAnOptionSelection )
                menu.addAction(action);
        } else if (action->objectName().compare("actionShowRecurrence_option")==0) {
            if ( indexSelection.size()>=1 && thereIsAnOptionSelection && getRecurrentOption(indexSelection.at(0)).size()>0 )
                menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionResize_columns")==0) {
            if ( isThereARow() )
                menu.addAction(action);
        }
    }

    menu.exec(ui->solverOptionTableView->viewport()->mapToGlobal(pos));
}

void SolverOptionWidget::showDefinitionContextMenu(const QPoint &pos)
{
    QModelIndexList selection = ui->solverOptionTreeView->selectionModel()->selectedRows();
    if (selection.count() <= 0)
        return;

    bool hasSelectionBeenAdded = (selection.size()>0);
    // assume single selection
    for (const QModelIndex &idx : std::as_const(selection)) {
        QModelIndex parentIdx = ui->solverOptionTreeView->model()->parent(idx);
        QVariant data = (parentIdx.row() < 0) ?  ui->solverOptionTreeView->model()->data(idx, Qt::CheckStateRole)
                                              : ui->solverOptionTreeView->model()->data(parentIdx, Qt::CheckStateRole);
        hasSelectionBeenAdded = (Qt::CheckState(data.toInt()) == Qt::Checked);
    }

    QMenu menu(this);
    const auto actions = ui->solverOptionTreeView->actions();
    for(QAction* action : actions) {
        if (action->objectName().compare("actionAddThisOption")==0) {
            if ( !hasSelectionBeenAdded && ui->solverOptionTableView->selectionModel()->selectedRows().size() <= 0 )
                menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionDeleteThisOption")==0) {
                  if ( hasSelectionBeenAdded && ui->solverOptionTableView->selectionModel()->selectedRows().size() > 0)
                      menu.addAction(action);
                  menu.addSeparator();
        } else if (action->objectName().compare("actionCopyDefinitionOptionName")==0) {
                  menu.addAction(action);
        } else if (action->objectName().compare("actionCopyDefinitionOptionDescription")==0) {
                  menu.addAction(action);
        } else if (action->objectName().compare("actionCopyDefinitionText")==0) {
                  menu.addAction(action);
                  menu.addSeparator();
        }  else if (action->objectName().compare("actionResize_columns")==0) {
                if ( ui->solverOptionTreeView->model()->rowCount()>0 )
                   menu.addAction(action);
        }
    }

    menu.exec(ui->solverOptionTreeView->viewport()->mapToGlobal(pos));
}

void SolverOptionWidget::addOptionFromDefinition(const QModelIndex &index)
{
    setModified(true);

    QModelIndex parentIndex =  ui->solverOptionTreeView->model()->parent(index);
    QModelIndex optionNameIndex = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) :
                                                          ui->solverOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ;
    QModelIndex defValueIndex = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_DEF_VALUE) :
                                                        ui->solverOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_DEF_VALUE) ;
    QModelIndex selectedValueIndex = (parentIndex.row()<0) ? defValueIndex :
                                                             ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex) ;

    disconnect(mOptionTableModel, &QAbstractTableModel::dataChanged, mOptionTableModel, &SolverOptionTableModel::on_updateSolverOptionItem);

    bool replaceExistingEntry = false;
    QString optionNameData = ui->solverOptionTreeView->model()->data(optionNameIndex).toString();
    QModelIndex optionIdIndex = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER) :
                                                        ui->solverOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER) ;
    QVariant optionIdData = ui->solverOptionTreeView->model()->data(optionIdIndex);

    int rowToBeAdded = ui->solverOptionTableView->model()->rowCount();
    Settings* settings = Settings::settings();
    if (settings && settings->toBool(skSoOverrideExisting)) {
        QModelIndexList indices = ui->solverOptionTableView->model()->match(ui->solverOptionTableView->model()->index(0, mOptionTableModel->getColumnEntryNumber()),
                                                                            Qt::DisplayRole,
                                                                            optionIdData, -1, Qt::MatchExactly|Qt::MatchRecursive);
        ui->solverOptionTableView->clearSelection();
        QItemSelection selection;
        for(const QModelIndex &idx: std::as_const(indices)) {
            QModelIndex leftIndex  = ui->solverOptionTableView->model()->index(idx.row(), GamsParameterTableModel::COLUMN_OPTION_KEY);
            QModelIndex rightIndex = ui->solverOptionTableView->model()->index(idx.row(), GamsParameterTableModel::COLUMN_ENTRY_NUMBER);
            QItemSelection rowSelection(leftIndex, rightIndex);
            selection.merge(rowSelection, QItemSelectionModel::Select);
        }
        ui->solverOptionTableView->selectionModel()->select(selection, QItemSelectionModel::Select);

        bool singleEntryExisted = (indices.size()==1);
        bool multipleEntryExisted = (indices.size()>1);
        if (singleEntryExisted ) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Option Entry exists");
            msgBox.setText("Option '" + optionNameData+ "' already exists.");
            msgBox.setInformativeText("How do you want to proceed?");
            msgBox.setDetailedText(QString("Entry:  '%1'\nDescription:  %2 %3").arg(getOptionTableEntry(indices.at(0).row()))
                    .arg("When a solver option file contains multiple entries of the same options, only the value of the last entry will be utilized by the solver.",
                         "The value of all other entries except the last entry will be ignored."));
            msgBox.setStandardButtons(QMessageBox::Abort);
            msgBox.addButton("Replace existing entry", QMessageBox::ActionRole);
            msgBox.addButton("Add new entry", QMessageBox::ActionRole);

            switch(msgBox.exec()) {
            case 3: // replace
                if (settings && settings->toBool(skSoDeleteCommentsAbove) && indices.size()>0) {
                    disconnect(mOptionTableModel, &SolverOptionTableModel::solverOptionItemRemoved, mOptionTableModel, &SolverOptionTableModel::on_removeSolverOptionItem);
                    deleteCommentsBeforeOption(indices.at(0).row());
                    connect(mOptionTableModel, &SolverOptionTableModel::solverOptionItemRemoved, mOptionTableModel, &SolverOptionTableModel::on_removeSolverOptionItem, Qt::UniqueConnection);
                }
                replaceExistingEntry = true;
                indices = ui->solverOptionTableView->model()->match(ui->solverOptionTableView->model()->index(0, mOptionTableModel->getColumnEntryNumber()),
                                                                    Qt::DisplayRole,
                                                                    optionIdData, -1, Qt::MatchExactly|Qt::MatchRecursive);
                rowToBeAdded = (indices.size()>0) ? indices.at(0).row() : 0;
                break;
            case 4: // add
                break;
            case QMessageBox::Abort:
                return;
            }
        } else if (multipleEntryExisted) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Multiple Option Entries exist");
            msgBox.setText(QString("%1 entries of Option '%2' already exist.").arg(indices.size()).arg(optionNameData));
            msgBox.setInformativeText("How do you want to proceed?");
            QString entryDetailedText = QString("Entries:\n");
            int i = 0;
            for (const QModelIndex &idx : std::as_const(indices))
                entryDetailedText.append(QString("   %1. '%2'\n").arg(++i).arg(getOptionTableEntry(idx.row())));
            msgBox.setDetailedText(QString("%1Description:  %2 %3").arg(entryDetailedText)
                    .arg("When a solver option file contains multiple entries of the same options, only the value of the last entry will be utilized by the solver.",
                         "The value of all other entries except the last entry will be ignored."));
            msgBox.setText("Multiple entries of Option '" + optionNameData + "' already exist.");
            msgBox.setInformativeText("How do you want to proceed?");
            msgBox.setStandardButtons(QMessageBox::Abort);
            msgBox.addButton("Replace first entry and delete other entries", QMessageBox::ActionRole);
            msgBox.addButton("Add new entry", QMessageBox::ActionRole);

            switch(msgBox.exec()) {
            case 3: // delete and replace
                disconnect(mOptionTableModel, &SolverOptionTableModel::solverOptionItemRemoved, mOptionTableModel, &SolverOptionTableModel::on_removeSolverOptionItem);
                ui->solverOptionTableView->selectionModel()->clearSelection();
                for(int i=1; i<indices.size(); i++) {
                    ui->solverOptionTableView->selectionModel()->select( indices.at(i), QItemSelectionModel::Select|QItemSelectionModel::Rows );
                }
                deleteOption();
                deleteCommentsBeforeOption(indices.at(0).row());
                connect(mOptionTableModel, &SolverOptionTableModel::solverOptionItemRemoved, mOptionTableModel, &SolverOptionTableModel::on_removeSolverOptionItem, Qt::UniqueConnection);
                replaceExistingEntry = true;
                indices = ui->solverOptionTableView->model()->match(ui->solverOptionTableView->model()->index(0, mOptionTableModel->getColumnEntryNumber()),
                                                                    Qt::DisplayRole,
                                                                    optionIdData, -1, Qt::MatchExactly|Qt::MatchRecursive);
                rowToBeAdded = (indices.size()>0) ? indices.at(0).row() : 0;
                break;
            case 4: // add
                break;
            case QMessageBox::Abort:
                return;
            }

        } // else entry not exist
    }
    ui->solverOptionTableView->selectionModel()->clearSelection();
//    QString synonymData = ui->solverOptionTreeView->model()->data(synonymIndex).toString();
    QString selectedValueData = ui->solverOptionTreeView->model()->data(selectedValueIndex).toString();
    mOptionTokenizer->getOption()->setModified(optionNameData, true);
    ui->solverOptionTreeView->model()->setData(optionNameIndex, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);

    if (settings && settings->toBool(skSoAddCommentAbove)) { // insert comment description row
        int indexRow = index.row();
        int parentIndexRow = parentIndex.row();
        QModelIndex descriptionIndex = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index(indexRow, OptionDefinitionModel::COLUMN_DESCIPTION):
                                                               ui->solverOptionTreeView->model()->index(parentIndexRow, OptionDefinitionModel::COLUMN_DESCIPTION);
        QString descriptionData = ui->solverOptionTreeView->model()->data(descriptionIndex).toString();

        ui->solverOptionTableView->model()->insertRows(rowToBeAdded, 1, QModelIndex());

        QModelIndex insertKeyIndex = ui->solverOptionTableView->model()->index(rowToBeAdded, SolverOptionTableModel::COLUMN_OPTION_KEY);
        QModelIndex insertValueIndex = ui->solverOptionTableView->model()->index(rowToBeAdded, SolverOptionTableModel::COLUMN_OPTION_VALUE);
        QModelIndex insertNumberIndex = ui->solverOptionTableView->model()->index(rowToBeAdded, mOptionTableModel->getColumnEntryNumber());

        ui->solverOptionTableView->model()->setHeaderData( insertKeyIndex.row(), Qt::Vertical,
                                                           Qt::CheckState(Qt::PartiallyChecked),
                                                           Qt::CheckStateRole );
        ui->solverOptionTableView->model()->setData( insertKeyIndex, descriptionData, Qt::EditRole);
        ui->solverOptionTableView->model()->setData( insertValueIndex, "", Qt::EditRole);
        ui->solverOptionTableView->model()->setData( insertNumberIndex, -1, Qt::EditRole);

        rowToBeAdded++;
        if (parentIndex.row() >= 0) {  // insert enum comment description row
            descriptionIndex = ui->solverOptionTreeView->model()->index(indexRow, OptionDefinitionModel::COLUMN_DESCIPTION, parentIndex);
            QString strData =  selectedValueData;
            strData.append( " - " );
            strData.append( ui->solverOptionTreeView->model()->data(descriptionIndex).toString() );

            ui->solverOptionTableView->model()->insertRows(rowToBeAdded, 1, QModelIndex());

            insertKeyIndex = ui->solverOptionTableView->model()->index(rowToBeAdded, SolverOptionTableModel::COLUMN_OPTION_KEY);
            insertValueIndex = ui->solverOptionTableView->model()->index(rowToBeAdded, SolverOptionTableModel::COLUMN_OPTION_VALUE);
            insertNumberIndex = ui->solverOptionTableView->model()->index(rowToBeAdded, mOptionTableModel->getColumnEntryNumber());

            ui->solverOptionTableView->model()->setHeaderData( insertKeyIndex.row(), Qt::Vertical,
                                                               Qt::CheckState(Qt::PartiallyChecked),
                                                               Qt::CheckStateRole );
            ui->solverOptionTableView->model()->setData( insertKeyIndex, strData, Qt::EditRole);
            ui->solverOptionTableView->model()->setData( insertValueIndex, "", Qt::EditRole);
            ui->solverOptionTableView->model()->setData( insertNumberIndex, -1, Qt::EditRole);

            rowToBeAdded++;
        }
    }
    // insert option row
    if (rowToBeAdded == ui->solverOptionTableView->model()->rowCount()) {
        ui->solverOptionTableView->model()->insertRows(rowToBeAdded, 1, QModelIndex());
    }
    QModelIndex insertKeyIndex = ui->solverOptionTableView->model()->index(rowToBeAdded, SolverOptionTableModel::COLUMN_OPTION_KEY);
    QModelIndex insertValueIndex = ui->solverOptionTableView->model()->index(rowToBeAdded, SolverOptionTableModel::COLUMN_OPTION_VALUE);
    QModelIndex insertNumberIndex = ui->solverOptionTableView->model()->index(rowToBeAdded, mOptionTableModel->getColumnEntryNumber());
    ui->solverOptionTableView->model()->setData( insertKeyIndex, optionNameData, Qt::EditRole);
    ui->solverOptionTableView->model()->setData( insertValueIndex, selectedValueData, Qt::EditRole);
    if (settings && settings->toBool(skSoAddEOLComment)) {
        QModelIndex commentIndex = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_DESCIPTION)
                                                         : ui->solverOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_DESCIPTION);
        QString commentData = ui->solverOptionTreeView->model()->data(commentIndex).toString();
        QModelIndex insertEOLCommentIndex = ui->solverOptionTableView->model()->index(rowToBeAdded, SolverOptionTableModel::COLUMN_EOL_COMMENT);
        ui->solverOptionTableView->model()->setData( insertEOLCommentIndex, commentData, Qt::EditRole);
    }
    int optionEntryNumber = mOptionTokenizer->getOption()->getOptionDefinition(optionNameData).number;
    ui->solverOptionTableView->model()->setData( insertNumberIndex, optionEntryNumber, Qt::EditRole);
    ui->solverOptionTableView->model()->setHeaderData( rowToBeAdded, Qt::Vertical, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole );
    ui->solverOptionTableView->selectRow(rowToBeAdded);
    selectAnOption();

    QString text = mOptionTableModel->getOptionTableEntry(insertNumberIndex.row());
    if (replaceExistingEntry)
        mOptionTokenizer->logger()->append(QString("Option entry '%1' has been replaced").arg(text), LogMsgType::Info);
    else
        mOptionTokenizer->logger()->append(QString("Option entry '%1' has been added").arg(text), LogMsgType::Info);

    int lastColumn = ui->solverOptionTableView->model()->columnCount()-1;
    int lastRow = rowToBeAdded;
    int firstRow = lastRow;
    if (settings && settings->toBool(skSoAddCommentAbove)) {
        firstRow--;
        if (parentIndex.row() >=0)
            firstRow--;
    }
    if (firstRow<0)
        firstRow = 0;
    mOptionTableModel->on_updateSolverOptionItem( ui->solverOptionTableView->model()->index(firstRow, lastColumn),
                                                  ui->solverOptionTableView->model()->index(lastRow, lastColumn),
                                                  {Qt::EditRole});

    connect(mOptionTableModel, &QAbstractTableModel::dataChanged, mOptionTableModel, &SolverOptionTableModel::on_updateSolverOptionItem, Qt::UniqueConnection);
    updateTableColumnSpan();
    if (isViewCompact())
        refreshOptionTableModel(true);
    showOptionDefinition(true);

    emit itemCountChanged(ui->solverOptionTableView->model()->rowCount());

    if (parentIndex.row()<0) {
        if (mOptionTokenizer->getOption()->getOptionSubType(optionNameData) != optsubNoValue)
            ui->solverOptionTableView->edit(insertValueIndex);
   }
}


void SolverOptionWidget::on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(topLeft)
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)
    setModified(true);

    QModelIndexList toDefinitionItems = ui->solverOptionTreeView->model()->match(ui->solverOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     ui->solverOptionTableView->model()->data( topLeft, Qt::DisplayRole), 1);
    if (toDefinitionItems.size() <= 0) {
        toDefinitionItems = ui->solverOptionTreeView->model()->match(ui->solverOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_SYNONYM),
                                                                         Qt::DisplayRole,
                                                                         ui->solverOptionTableView->model()->data( topLeft, Qt::DisplayRole), 1);
    }

    for(const QModelIndex &item : std::as_const(toDefinitionItems)) {
        if (Qt::CheckState(ui->solverOptionTableView->model()->headerData(item.row(), Qt::Vertical, Qt::CheckStateRole).toUInt())==Qt::PartiallyChecked)
            continue;
        ui->solverOptionTreeView->selectionModel()->select(
                    QItemSelection (
                        ui->solverOptionTreeView->model ()->index (item.row() , 0),
                        ui->solverOptionTreeView->model ()->index (item.row(), ui->solverOptionTreeView->model ()->columnCount () - 1)),
                    QItemSelectionModel::Select);
        ui->solverOptionTreeView->scrollTo(toDefinitionItems.first(), QAbstractItemView::EnsureVisible);
    }
    ui->solverOptionTableView->selectionModel()->select(topLeft, QItemSelectionModel::Select);
}

bool SolverOptionWidget::saveOptionFile(const QString &location)
{
    return saveAs(location);
}

void SolverOptionWidget::on_reloadSolverOptionFile(const QString &encodingName)
{
    if (mEncoding != encodingName) {
        mEncoding = encodingName.isEmpty() ? "UTF-8" : encodingName;
        mOptionTokenizer->logger()->append(QString("Loading options from %1 with %2 encoding")
                                               .arg(mLocation, encodingName), LogMsgType::Info);
    } else if (mFileHasChangedExtern)
        mOptionTokenizer->logger()->append(QString("Loading options from %1").arg(mLocation), LogMsgType::Info);
     else
         return;
     mOptionTableModel->reloadSolverOptionModel( mOptionTokenizer->readOptionFile(mLocation, mEncoding) );
     mFileHasChangedExtern = false;
     setModified(false);
}

void SolverOptionWidget::on_selectRow(int logicalIndex)
{
    if (ui->solverOptionTableView->model()->rowCount() <= 0)
        return;

    QItemSelectionModel *selectionModel = ui->solverOptionTableView->selectionModel();
    QModelIndex topLeft = ui->solverOptionTableView->model()->index(logicalIndex, SolverOptionTableModel::COLUMN_OPTION_KEY, QModelIndex());
    QModelIndex  bottomRight = ui->solverOptionTableView->model()->index(logicalIndex, mOptionTableModel->getColumnEntryNumber(), QModelIndex());
    QItemSelection selection( topLeft, bottomRight);
    selectionModel->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

void SolverOptionWidget::on_selectAndToggleRow(int logicalIndex)
{
    on_selectRow(logicalIndex);
    on_toggleRowHeader(logicalIndex);
}

void SolverOptionWidget::on_toggleRowHeader(int logicalIndex)
{
    if (isEditing()) {
        QLineEdit* editor = qobject_cast<QLineEdit *>(mOptionCompleter->lastEditor());
        if (editor) emit editor->editingFinished();
    }
    mOptionTableModel->on_toggleRowHeader(logicalIndex);
    updateTableColumnSpan();
    setModified(true);

    if (ui->compactViewCheckBox->isChecked())
        on_compactViewCheckBox_stateChanged(Qt::Checked);
}

void SolverOptionWidget::on_compactViewCheckBox_stateChanged(int checkState)
{
    bool isViewCompact = (Qt::CheckState(checkState) == Qt::Checked);
    refreshOptionTableModel(isViewCompact);
    if (isViewCompact) {
        mOptionTokenizer->logger()->append(QString("activated Compact View, comments are hidden and actions related to comments are either not visible or forbidden"), LogMsgType::Info);
    } else {
        mOptionTokenizer->logger()->append(QString("deactivated Compact View, comments are now visible and all actions are allowed"), LogMsgType::Info);
    }
    emit compactViewChanged(isViewCompact);
}

void SolverOptionWidget::on_messageViewCheckBox_stateChanged(int checkState)
{
    if (Qt::CheckState(checkState) == Qt::Checked)
        ui->solverOptionVSplitter->setSizes(QList<int>({75, 25}));
    else
        ui->solverOptionVSplitter->setSizes(QList<int>({100, 0}));
    ui->solverOptionTabWidget->setVisible(Qt::CheckState(checkState) == Qt::Checked);
}

void SolverOptionWidget::on_openAsTextButton_clicked(bool checked)
{
    Q_UNUSED(checked)
    MainWindow* main = getMainWindow();
    if (!main) return;

    emit main->projectRepo()->closeFileEditors(fileId());

    FileMeta* fileMeta = main->fileRepo()->fileMeta(fileId());
    PExFileNode* fileNode = main->projectRepo()->findFileNode(this);
    PExProjectNode* project = (fileNode ? fileNode->assignedProject() : nullptr);

    emit main->projectRepo()->openFile(fileMeta, true, project, -1, true);
}

void SolverOptionWidget::copyAction()
{
    copyDefinitionToClipboard(SolverOptionDefinitionModel::COLUMN_OPTION_NAME);
}

void SolverOptionWidget::showOptionDefinition(bool selectRow)
{
    if (ui->solverOptionTableView->model()->rowCount() <= 0)
        return;

    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();
    if (indexSelection.count() <= 0)
        return;

    disconnect(ui->solverOptionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SolverOptionWidget::findAndSelectionOptionFromDefinition);

    ui->solverOptionSearch->clear();

    QModelIndexList selection;
    if (selectRow) {
       selectAnOption();
       selection = ui->solverOptionTableView->selectionModel()->selectedRows();
       ui->solverOptionTableView->selectionModel()->setCurrentIndex ( indexSelection.first(), QItemSelectionModel::Current ); //Select );
    } else {
         selection = indexSelection;
         ui->solverOptionTableView->selectionModel()->setCurrentIndex ( indexSelection.first(), QItemSelectionModel::Current ); //Select );
    }

    QModelIndexList selectIndices;
    for (int i=0; i<selection.count(); i++) {
         QModelIndex index = selection.at(i);
         if (Qt::CheckState(ui->solverOptionTableView->model()->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole).toUInt())==Qt::PartiallyChecked)
                continue;

         QString value = ui->solverOptionTableView->model()->data( index.sibling(index.row(), SolverOptionTableModel::COLUMN_OPTION_VALUE), Qt::DisplayRole).toString();
         QVariant optionId = ui->solverOptionTableView->model()->data( index.sibling(index.row(), mOptionTableModel->getColumnEntryNumber()), Qt::DisplayRole);
         QModelIndexList indices = ui->solverOptionTreeView->model()->match(ui->solverOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_ENTRY_NUMBER),
                                                                               Qt::DisplayRole,
                                                                               optionId, 1, Qt::MatchExactly|Qt::MatchRecursive);
         for(const QModelIndex &idx : std::as_const(indices)) {
             QModelIndex  parentIndex =  ui->solverOptionTreeView->model()->parent(idx);
             QModelIndex optionIdx = ui->solverOptionTreeView->model()->index(idx.row(), OptionDefinitionModel::COLUMN_OPTION_NAME);

             if (parentIndex.row() < 0) {
                if (ui->solverOptionTreeView->model()->hasChildren(optionIdx) && !ui->solverOptionTreeView->isExpanded(optionIdx))
                    ui->solverOptionTreeView->expand(optionIdx);
            }
            bool found = false;
            for(int r=0; r <ui->solverOptionTreeView->model()->rowCount(optionIdx); ++r) {
                QModelIndex i = ui->solverOptionTreeView->model()->index(r, OptionDefinitionModel::COLUMN_OPTION_NAME, optionIdx);
                QString enumValue = ui->solverOptionTreeView->model()->data(i, Qt::DisplayRole).toString();
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
    ui->solverOptionTreeView->selectionModel()->clearSelection();
    for(const QModelIndex &idx : std::as_const(selectIndices)) {
        QItemSelection selection = ui->solverOptionTreeView->selectionModel()->selection();
        QModelIndex  parentIdx =  ui->solverOptionTreeView->model()->parent(idx);
        if (parentIdx.row() < 0) {
            selection.select(ui->solverOptionTreeView->model()->index(idx.row(), OptionDefinitionModel::COLUMN_OPTION_NAME),
                             ui->solverOptionTreeView->model()->index(idx.row(), ui->solverOptionTreeView->model()->columnCount()-1));
        } else  {
            selection.select(ui->solverOptionTreeView->model()->index(idx.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIdx),
                             ui->solverOptionTreeView->model()->index(idx.row(), ui->solverOptionTreeView->model()->columnCount()-1, parentIdx));
        }
        ui->solverOptionTreeView->selectionModel()->select(selection, QItemSelectionModel::Select);
    }
    if (selectIndices.size() > 0) {
        QModelIndex parentIndex = ui->solverOptionTreeView->model()->parent(selectIndices.first());
        QModelIndex scrollToIndex = (parentIndex.row() < 0  ? ui->solverOptionTreeView->model()->index(selectIndices.first().row(), OptionDefinitionModel::COLUMN_OPTION_NAME)
                                                            : ui->solverOptionTreeView->model()->index(selectIndices.first().row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex));
        ui->solverOptionTreeView->scrollTo(scrollToIndex, QAbstractItemView::EnsureVisible);
        if (parentIndex.row() >= 0) {
            ui->solverOptionTreeView->scrollTo(parentIndex, QAbstractItemView::EnsureVisible);
            const QRect r = ui->solverOptionTreeView->visualRect(parentIndex);
            ui->solverOptionTreeView->horizontalScrollBar()->setValue(r.x());
        } else {
            const QRect r = ui->solverOptionTreeView->visualRect(scrollToIndex);
            ui->solverOptionTreeView->horizontalScrollBar()->setValue(r.x());
        }
    }

    connect(ui->solverOptionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SolverOptionWidget::findAndSelectionOptionFromDefinition, Qt::UniqueConnection);
}

void SolverOptionWidget::showOptionRecurrence()
{
    if (!isInFocus(focusWidget()))
        return;

    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();
    if (indexSelection.size() <= 0) {
        showOptionDefinition();
        return;
    }

    QItemSelection selection = ui->solverOptionTableView->selectionModel()->selection();
    selection.select(ui->solverOptionTableView->model()->index(indexSelection.at(0).row(), 0),
                     ui->solverOptionTableView->model()->index(indexSelection.at(0).row(), GamsParameterTableModel::COLUMN_ENTRY_NUMBER));
    ui->solverOptionTableView->selectionModel()->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows );

    QList<int> rowList = getRecurrentOption(indexSelection.at(0));
    if (rowList.size() <= 0) {
        showOptionDefinition();
        return;
    }

    for(int row : std::as_const(rowList)) {
        QItemSelection rowSelection = ui->solverOptionTableView->selectionModel()->selection();
        rowSelection.select(ui->solverOptionTableView->model()->index(row, 0),
                            ui->solverOptionTableView->model()->index(row, GamsParameterTableModel::COLUMN_ENTRY_NUMBER));
        ui->solverOptionTableView->selectionModel()->select(rowSelection, QItemSelectionModel::Select | QItemSelectionModel::Rows );
    }

    showOptionDefinition();
}

void SolverOptionWidget::copyDefinitionToClipboard(int column)
{
    if (ui->solverOptionTreeView->selectionModel()->selectedRows().count() <= 0)
        return;

    QModelIndex index = ui->solverOptionTreeView->selectionModel()->selectedRows().at(0);
    if (!index.isValid())
        return;

    QString text = "";
    QModelIndex parentIndex = ui->solverOptionTreeView->model()->parent(index);
    if (column == -1) { // copy all
        QStringList strList;
        if (parentIndex.isValid()) {
            QModelIndex idx = ui->solverOptionTreeView->model()->index(index.row(), SolverOptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex);
            strList << ui->solverOptionTreeView->model()->data(idx, Qt::DisplayRole).toString();
            idx = ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_DESCIPTION, parentIndex);
            strList << ui->solverOptionTreeView->model()->data(idx, Qt::DisplayRole).toString();
            text = strList.join(", ");
        } else {
           for (int j=0; j<ui->solverOptionTreeView->model()->columnCount(); j++) {
               if (j==SolverOptionDefinitionModel::COLUMN_ENTRY_NUMBER)
                  continue;
               QModelIndex columnindex = ui->solverOptionTreeView->model()->index(index.row(), j);
               strList << ui->solverOptionTreeView->model()->data(columnindex, Qt::DisplayRole).toString();
           }
           text = strList.join(", ");
        }
    } else {
        if (parentIndex.isValid()) {
            QModelIndex idx = ui->solverOptionTreeView->model()->index(index.row(), column, parentIndex);
            text = ui->solverOptionTreeView->model()->data( idx, Qt::DisplayRole ).toString();
        } else {
            text = ui->solverOptionTreeView->model()->data( ui->solverOptionTreeView->model()->index(index.row(), column), Qt::DisplayRole ).toString();
        }
    }
    QClipboard* clip = QApplication::clipboard();
    clip->setText( text );
}

void SolverOptionWidget::findAndSelectionOptionFromDefinition()
{
    if (ui->solverOptionTableView->model()->rowCount() <= 0)
        return;

    QModelIndex index = ui->solverOptionTreeView->selectionModel()->currentIndex();
    QModelIndex parentIndex =  ui->solverOptionTreeView->model()->parent(index);

    QModelIndex idx = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index( index.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER )
                                            : ui->solverOptionTreeView->model()->index( parentIndex.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER );
    QVariant data = ui->solverOptionTreeView->model()->data( idx, Qt::DisplayRole );
    QModelIndexList indices = ui->solverOptionTableView->model()->match(ui->solverOptionTableView->model()->index(0, mOptionTableModel->getColumnEntryNumber()),
                                                                       Qt::DisplayRole,
                                                                       data.toString(), -1, Qt::MatchExactly|Qt::MatchRecursive);
    ui->solverOptionTableView->clearSelection();
    ui->solverOptionTableView->clearFocus();
    QItemSelection selection;
    for(const QModelIndex &i :std::as_const(indices)) {
        QModelIndex valueIndex = ui->solverOptionTableView->model()->index(i.row(), SolverOptionTableModel::COLUMN_OPTION_VALUE);
        QString value =  ui->solverOptionTableView->model()->data( valueIndex, Qt::DisplayRole).toString();
        bool selected = false;
        if (parentIndex.row() < 0) {
            selected = true;
        } else {
            QModelIndex enumIndex = ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex);
            QString enumValue = ui->solverOptionTreeView->model()->data( enumIndex, Qt::DisplayRole).toString();
            if (QString::compare(value, enumValue, Qt::CaseInsensitive)==0)
                selected = true;
        }
        if (selected) {
            QModelIndex leftIndex  = ui->solverOptionTableView->model()->index(i.row(), 0);
            QModelIndex rightIndex = ui->solverOptionTableView->model()->index(i.row(), ui->solverOptionTableView->model()->columnCount() -1);

            QItemSelection rowSelection(leftIndex, rightIndex);
            selection.merge(rowSelection, QItemSelectionModel::Select);
        }
    }

    ui->solverOptionTableView->selectionModel()->select(selection, QItemSelectionModel::Select);
    ui->solverOptionTreeView->setFocus();
}

void SolverOptionWidget::toggleCommentOption()
{
    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex &index : std::as_const(indexSelection)) {
        ui->solverOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if ( isThereARow() && !isThereARowSelection() && !isEverySelectionARow() )
        return;

    bool modified = false;
    QModelIndexList selection = ui->solverOptionTableView->selectionModel()->selectedRows();
    for(int i=0; i<selection.count(); ++i) {
        on_toggleRowHeader( selection.at(i).row() );
        modified = true;
    }
    for(int i=0; i<selection.count(); ++i) {
         on_selectRow( selection.at(i).row() );
    }

    if (!indexSelection.isEmpty())
        ui->solverOptionTableView->selectionModel()->setCurrentIndex( indexSelection.first(), QItemSelectionModel::Current );
    ui->solverOptionTableView->setFocus();

    if (modified) {
        setModified(modified);
    }
}

void SolverOptionWidget::selectAllOptions()
{
    if (isViewCompact()) return;

    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();

    ui->solverOptionTableView->setFocus();
    ui->solverOptionTableView->selectAll();

    if (!indexSelection.isEmpty())
        ui->solverOptionTableView->selectionModel()->setCurrentIndex( indexSelection.first(), QItemSelectionModel::Current );
}

void SolverOptionWidget::deSelectOptions()
{
    if (ui->solverOptionTableView->hasFocus() && ui->solverOptionTableView->selectionModel()->hasSelection()) {
        QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();
        ui->solverOptionTreeView->selectionModel()->clearSelection();
        ui->solverOptionTableView->selectionModel()->clearSelection();
        if (!indexSelection.isEmpty())
             ui->solverOptionTableView->selectionModel()->setCurrentIndex( indexSelection.first(), QItemSelectionModel::Current );
        ui->solverOptionTableView->setFocus();
    } else if (ui->solverOptionTreeView->hasFocus() && ui->solverOptionTreeView->selectionModel()->hasSelection()) {
             ui->solverOptionTreeView->selectionModel()->clearSelection();
             ui->solverOptionTreeView->setFocus();
    } else {
        this->focusNextChild();
    }
}

void SolverOptionWidget::completeEditingOption(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    Q_UNUSED(editor)
    Q_UNUSED(hint)
    showOptionDefinition(false);
}

void SolverOptionWidget::selectAnOption()
{
    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();
    if (indexSelection.empty())
        indexSelection <<  ui->solverOptionTableView->selectionModel()->currentIndex();

    QList<int> rowIndex;
    for(int i=0; i<indexSelection.count(); ++i) {
        if (!rowIndex.contains(i)) {
            rowIndex << i;
            on_selectRow( indexSelection.at(i).row() );
        }
    }
    ui->solverOptionTableView->selectionModel()->clearCurrentIndex();
    ui->solverOptionTableView->selectionModel()->setCurrentIndex( indexSelection.first(), QItemSelectionModel::Current );
}

void SolverOptionWidget::insertOption()
{
    if (isViewCompact())
        return;

    if (isEditing()) {
        QLineEdit* editor = qobject_cast<QLineEdit *>(mOptionCompleter->lastEditor());
        if (editor) emit editor->editingFinished();
    }

    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex &index : std::as_const(indexSelection)) {
        if (mOptionCompleter->currentEditedIndex().isValid() && mOptionCompleter->currentEditedIndex().row()==index.row())
            return;
        ui->solverOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    disconnect(mOptionTableModel, &QAbstractTableModel::dataChanged, mOptionTableModel, &SolverOptionTableModel::on_updateSolverOptionItem);
    int rowToBeInserted = -1;
    if (isThereARowSelection()) {
        QList<int> rows;
        const auto indexes = ui->solverOptionTableView->selectionModel()->selectedRows();
        for(const QModelIndex &idx : indexes) {
            rows.append( idx.row() );
        }
        std::sort(rows.begin(), rows.end());
        rowToBeInserted = rows.at(0);
        ui->solverOptionTableView->model()->insertRows(rowToBeInserted, 1, QModelIndex());
        QModelIndex insertKeyIndex = ui->solverOptionTableView->model()->index(rowToBeInserted, SolverOptionTableModel::COLUMN_OPTION_KEY);
        QModelIndex insertValueIndex = ui->solverOptionTableView->model()->index(rowToBeInserted, SolverOptionTableModel::COLUMN_OPTION_VALUE);
        QModelIndex insertNumberIndex = ui->solverOptionTableView->model()->index(rowToBeInserted, mOptionTableModel->getColumnEntryNumber());

        ui->solverOptionTableView->model()->setHeaderData(rowToBeInserted, Qt::Vertical,
                                                          Qt::CheckState(Qt::Checked),
                                                          Qt::CheckStateRole );

        ui->solverOptionTableView->model()->setData( insertKeyIndex, OptionTokenizer::keyGeneratedStr, Qt::EditRole);
        ui->solverOptionTableView->model()->setData( insertValueIndex, OptionTokenizer::valueGeneratedStr, Qt::EditRole);
        if (mOptionTableModel->getColumnEntryNumber() > SolverOptionTableModel::COLUMN_EOL_COMMENT) {
            QModelIndex eolCommentIndex = ui->solverOptionTableView->model()->index(rowToBeInserted, SolverOptionTableModel::COLUMN_EOL_COMMENT);
            ui->solverOptionTableView->model()->setData( eolCommentIndex, OptionTokenizer::commentGeneratedStr, Qt::EditRole);
        }
        ui->solverOptionTableView->scrollTo(insertKeyIndex, QAbstractItemView::EnsureVisible);
        ui->solverOptionTableView->model()->setData( insertNumberIndex, -1, Qt::EditRole);
    } else {
        ui->solverOptionTableView->model()->insertRows(ui->solverOptionTableView->model()->rowCount(), 1, QModelIndex());
        rowToBeInserted = mOptionTableModel->rowCount()-1;
        QModelIndex insertKeyIndex = ui->solverOptionTableView->model()->index(rowToBeInserted, SolverOptionTableModel::COLUMN_OPTION_KEY);
        QModelIndex insertValueIndex = ui->solverOptionTableView->model()->index(rowToBeInserted, SolverOptionTableModel::COLUMN_OPTION_VALUE);
        QModelIndex insertNumberIndex = ui->solverOptionTableView->model()->index(rowToBeInserted, mOptionTableModel->getColumnEntryNumber());

        ui->solverOptionTableView->model()->setHeaderData(ui->solverOptionTableView->model()->rowCount()-1, Qt::Vertical,
                                                          Qt::CheckState(Qt::Checked),
                                                          Qt::CheckStateRole );

        ui->solverOptionTableView->model()->setData( insertKeyIndex, OptionTokenizer::keyGeneratedStr, Qt::EditRole);
        ui->solverOptionTableView->model()->setData( insertValueIndex, OptionTokenizer::valueGeneratedStr, Qt::EditRole);
        if (mOptionTableModel->getColumnEntryNumber() > SolverOptionTableModel::COLUMN_EOL_COMMENT) {
            QModelIndex eolCommentIndex = ui->solverOptionTableView->model()->index(rowToBeInserted, SolverOptionTableModel::COLUMN_EOL_COMMENT);
            ui->solverOptionTableView->model()->setData( eolCommentIndex, OptionTokenizer::commentGeneratedStr, Qt::EditRole);
        }
        ui->solverOptionTableView->scrollTo(insertKeyIndex, QAbstractItemView::EnsureVisible);
        ui->solverOptionTableView->model()->setData( insertNumberIndex, -1, Qt::EditRole);
    }
    connect(mOptionTableModel, &QAbstractTableModel::dataChanged, mOptionTableModel, &SolverOptionTableModel::on_updateSolverOptionItem, Qt::UniqueConnection);
    updateTableColumnSpan();
    setModified(true);
    emit itemCountChanged(ui->solverOptionTableView->model()->rowCount());

    ui->solverOptionTreeView->clearSelection();
    ui->solverOptionTableView->clearSelection();
    ui->solverOptionTableView->edit( mOptionTableModel->index(rowToBeInserted, SolverOptionTableModel::COLUMN_OPTION_KEY));
}

void SolverOptionWidget::insertComment()
{
    if  (isViewCompact())
        return;

    if (isEditing()) {
        QLineEdit* editor = qobject_cast<QLineEdit *>(mOptionCompleter->lastEditor());
        if (editor) emit editor->editingFinished();
    }

    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex &index : std::as_const(indexSelection)) {
        if (mOptionCompleter->currentEditedIndex().isValid() && mOptionCompleter->currentEditedIndex().row()==index.row())
            return;
        ui->solverOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    disconnect(mOptionTableModel, &QAbstractTableModel::dataChanged, mOptionTableModel, &SolverOptionTableModel::on_updateSolverOptionItem);
    int rowToBeInserted = -1;
    if (isThereARowSelection() ) {
        QList<int> rows;
        const auto indexes = ui->solverOptionTableView->selectionModel()->selectedRows();
        for(const QModelIndex &idx : indexes) {
            rows.append( idx.row() );
        }
        std::sort(rows.begin(), rows.end());
        rowToBeInserted = rows.at(0);
        ui->solverOptionTableView->model()->insertRows(rowToBeInserted, 1, QModelIndex());
        QModelIndex insertKeyIndex = ui->solverOptionTableView->model()->index(rowToBeInserted, SolverOptionTableModel::COLUMN_OPTION_KEY);
        QModelIndex insertValueIndex = ui->solverOptionTableView->model()->index(rowToBeInserted, SolverOptionTableModel::COLUMN_OPTION_VALUE);
        QModelIndex insertNumberIndex = ui->solverOptionTableView->model()->index(rowToBeInserted, mOptionTableModel->getColumnEntryNumber());

        ui->solverOptionTableView->model()->setHeaderData(rowToBeInserted, Qt::Vertical,
                                                          Qt::CheckState(Qt::PartiallyChecked),
                                                          Qt::CheckStateRole );
        ui->solverOptionTableView->model()->setData( insertKeyIndex, OptionTokenizer::commentGeneratedStr, Qt::EditRole);
        ui->solverOptionTableView->model()->setData( insertValueIndex, "", Qt::EditRole);
        ui->solverOptionTableView->model()->setData( insertNumberIndex, -1, Qt::EditRole);
    } else {
        ui->solverOptionTableView->model()->insertRows(ui->solverOptionTableView->model()->rowCount(), 1, QModelIndex());
        rowToBeInserted = ui->solverOptionTableView->model()->rowCount()-1;
        QModelIndex insertKeyIndex = ui->solverOptionTableView->model()->index(rowToBeInserted, SolverOptionTableModel::COLUMN_OPTION_KEY);
        QModelIndex insertValueIndex = ui->solverOptionTableView->model()->index(rowToBeInserted, SolverOptionTableModel::COLUMN_OPTION_VALUE);
        QModelIndex insertNumberIndex = ui->solverOptionTableView->model()->index(rowToBeInserted, mOptionTableModel->getColumnEntryNumber());
        ui->solverOptionTableView->model()->setHeaderData(rowToBeInserted, Qt::Vertical,
                                                          Qt::CheckState(Qt::PartiallyChecked),
                                                          Qt::CheckStateRole );

        ui->solverOptionTableView->model()->setData( insertKeyIndex, OptionTokenizer::commentGeneratedStr, Qt::EditRole);
        ui->solverOptionTableView->model()->setData( insertValueIndex, "", Qt::EditRole);
        if (mOptionTableModel->getColumnEntryNumber() > SolverOptionTableModel::COLUMN_EOL_COMMENT) {
            QModelIndex eolCommentIndex = ui->solverOptionTableView->model()->index(rowToBeInserted, SolverOptionTableModel::COLUMN_EOL_COMMENT);
            ui->solverOptionTableView->model()->setData( eolCommentIndex, "", Qt::EditRole);
        }
        ui->solverOptionTableView->model()->setData( insertNumberIndex, -1, Qt::EditRole);
    }
    connect(mOptionTableModel, &QAbstractTableModel::dataChanged, mOptionTableModel, &SolverOptionTableModel::on_updateSolverOptionItem, Qt::UniqueConnection);
    updateTableColumnSpan();
    setModified(true);
    emit itemCountChanged(ui->solverOptionTableView->model()->rowCount());

    ui->solverOptionTreeView->clearSelection();
    ui->solverOptionTableView->clearSelection();
    ui->solverOptionTableView->edit(mOptionTableModel->index(rowToBeInserted, SolverOptionTableModel::COLUMN_OPTION_KEY));
}

void SolverOptionWidget::deleteCommentsBeforeOption(int row)
{
    QList<int> rows;
    for(int r=row-1; r>=0; r--) {
       if (mOptionTableModel->headerData(r, Qt::Vertical, Qt::CheckStateRole).toUInt()==Qt::PartiallyChecked) {
           rows.append( r );
       } else {
           break;
       }
    }

    std::sort(rows.begin(), rows.end());

    int prev = -1;
    for(int i=rows.count()-1; i>=0; i--) {
        int current = rows[i];
        if (current != prev) {
            QString text = mOptionTableModel->getOptionTableEntry(current);
            ui->solverOptionTableView->model()->removeRows( current, 1 );
            mOptionTokenizer->logger()->append(QString("Option entry '%1' has been deleted").arg(text), LogMsgType::Info);
            prev = current;
        }
    }
    updateTableColumnSpan();
}

void SolverOptionWidget::deleteOption()
{
    if (isEditing()) {
        QLineEdit* editor = qobject_cast<QLineEdit *>(mOptionCompleter->lastEditor());
        if (editor) emit editor->editingFinished();
    }

    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex &index : std::as_const(indexSelection)) {
        ui->solverOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }
    if  (!isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    if (isThereARowSelection() && isEverySelectionARow()) {
        QItemSelection selection( ui->solverOptionTableView->selectionModel()->selection() );

        QList<int> rows;
        const auto indexes = ui->solverOptionTableView->selectionModel()->selectedRows();
        for(const QModelIndex & index : indexes) {
            rows.append( index.row() );
        }

        Settings* settings = Settings::settings();
        if (settings && settings->toBool(skSoDeleteCommentsAbove)) {
            const auto indexes = ui->solverOptionTableView->selectionModel()->selectedRows();
            for(const QModelIndex & index : indexes) {
                if (mOptionTableModel->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole).toUInt()!=Qt::PartiallyChecked) {
                   for(int row=index.row()-1; row>=0; row--) {
                       if (mOptionTableModel->headerData(row, Qt::Vertical, Qt::CheckStateRole).toUInt()==Qt::PartiallyChecked) {
                           rows.append( row );
                       } else {
                           break;
                       }
                   }
                }
            }
        }

        std::sort(rows.begin(), rows.end());
        int prev = -1;
        for(int i=rows.count()-1; i>=0; i--) {
            int current = rows[i];
            if (current != prev) {
                QString text = mOptionTableModel->getOptionTableEntry(current);
                ui->solverOptionTableView->model()->removeRows( current, 1 );
                mOptionTokenizer->logger()->append(QString("Option entry '%1' has been deleted").arg(text), LogMsgType::Info);
                prev = current;
            }
        }
        updateTableColumnSpan();
        setModified(true);
        emit itemCountChanged(ui->solverOptionTableView->model()->rowCount());
    }
}

void SolverOptionWidget::moveOptionUp()
{
    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex &index : std::as_const(indexSelection)) {
        ui->solverOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  (isViewCompact() || !isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    QModelIndexList selection = ui->solverOptionTableView->selectionModel()->selectedRows();
    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() < b.row(); });
    if  (idxSelection.first().row() <= 0)
        return;

    for(int i=0; i<idxSelection.size(); i++) {
        QModelIndex idx = idxSelection.at(i);
        ui->solverOptionTableView->model()->moveRows(QModelIndex(), idx.row(), 1,
                                                         QModelIndex(), idx.row()-1);
    }
//    ui->solverOptionTableView->model()->moveRows(QModelIndex(), index.row(), selection.count(),
//                                                 QModelIndex(), index.row()-1);
    updateTableColumnSpan();
    setModified(true);
 }

void SolverOptionWidget::moveOptionDown()
{
    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex &index : std::as_const(indexSelection)) {
        ui->solverOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  (isViewCompact() || !isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    QModelIndexList selection = ui->solverOptionTableView->selectionModel()->selectedRows();
    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() > b.row(); });
    if  (idxSelection.first().row() >= mOptionTableModel->rowCount()-1)
        return;

    for(int i=0; i<idxSelection.size(); i++) {
        QModelIndex idx = idxSelection.at(i);
        mOptionTableModel->moveRows(QModelIndex(), idx.row(), 1,
                                    QModelIndex(), idx.row()+2);
    }
//    mOptionTableModel->moveRows(QModelIndex(), index.row(), selection.count(),
//                                QModelIndex(), index.row()+selection.count()+1);
    updateTableColumnSpan();
    setModified(true);
}

void SolverOptionWidget::resizeColumnsToContents()
{
    if (focusWidget()==ui->solverOptionTableView) {
        if (ui->solverOptionTableView->model()->rowCount()<=0)
            return;
        ui->solverOptionTableView->resizeColumnToContents(SolverOptionTableModel::COLUMN_OPTION_KEY);
        ui->solverOptionTableView->resizeColumnToContents(SolverOptionTableModel::COLUMN_OPTION_VALUE);
    } else  if (focusWidget()==ui->solverOptionTreeView) {
        ui->solverOptionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_OPTION_NAME);
        ui->solverOptionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_SYNONYM);
        ui->solverOptionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_DEF_VALUE);
        ui->solverOptionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_RANGE);
        ui->solverOptionTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_TYPE);
    }
}

QList<int> SolverOptionWidget::getRecurrentOption(const QModelIndex &index)
{
    QList<int> optionList;

    if (!isInFocus(focusWidget()))
        return optionList;

    QVariant data = ui->solverOptionTableView->model()->headerData(index.row(), Qt::Vertical,  Qt::CheckStateRole);
    if (Qt::CheckState(data.toUInt())==Qt::PartiallyChecked)
        return optionList;

    QString optionId = ui->solverOptionTableView->model()->data( index.sibling(index.row(), mOptionTableModel->getColumnEntryNumber()), Qt::DisplayRole).toString();
    QModelIndexList indices = ui->solverOptionTableView->model()->match(ui->solverOptionTableView->model()->index(0, mOptionTableModel->getColumnEntryNumber()),
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

QString SolverOptionWidget::getOptionTableEntry(int row)
{
    QModelIndex keyIndex = ui->solverOptionTableView->model()->index(row, GamsParameterTableModel::COLUMN_OPTION_KEY);
    QVariant optionKey = ui->solverOptionTableView->model()->data(keyIndex, Qt::DisplayRole);
    QModelIndex valueIndex = ui->solverOptionTableView->model()->index(row, GamsParameterTableModel::COLUMN_OPTION_VALUE);
    QVariant optionValue = ui->solverOptionTableView->model()->data(valueIndex, Qt::DisplayRole);
    return QString("%1%2%3").arg(optionKey.toString(), mOptionTokenizer->getOption()->getDefaultSeparator(), optionValue.toString());
}

bool SolverOptionWidget::isEditing()
{
    return (mOptionCompleter->lastEditor() && !mOptionCompleter->isLastEditorClosed());
}

void SolverOptionWidget::refreshOptionTableModel(bool hideAllComments)
{
    if (hideAllComments) {
        if (mOptionTokenizer->getOption()->isEOLCharDefined())
           ui->solverOptionTableView->hideColumn(SolverOptionTableModel::COLUMN_EOL_COMMENT);
    } else {
        if (mOptionTokenizer->getOption()->isEOLCharDefined())
           ui->solverOptionTableView->showColumn(SolverOptionTableModel::COLUMN_EOL_COMMENT);
    }
    for(int i = 0; i < mOptionTableModel->rowCount(); ++i) {
       if (mOptionTableModel->headerData(i, Qt::Vertical, Qt::CheckStateRole).toUInt()==Qt::PartiallyChecked) {
          if (hideAllComments)
              ui->solverOptionTableView->hideRow(i);
          else
             ui->solverOptionTableView->showRow(i);
       }
    }
}

void SolverOptionWidget::addActions()
{
    QAction* insertOptionAction = mContextMenu.addAction(Theme::icon(":/%1/insert"), "Insert new option", [this]() { insertOption(); });
    insertOptionAction->setObjectName("actionInsert_option");
    insertOptionAction->setShortcut( QKeySequence("Ctrl+Return") );
    insertOptionAction->setShortcutVisibleInContextMenu(true);
    insertOptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->solverOptionTableView->addAction(insertOptionAction);

    QAction* insertCommentAction = mContextMenu.addAction(Theme::icon(":/%1/insert"), "Insert new comment", [this]() { insertComment(); });
    insertCommentAction->setObjectName("actionInsert_comment");
    insertCommentAction->setShortcut( QKeySequence("Ctrl+Shift+Return") );
    insertCommentAction->setShortcutVisibleInContextMenu(true);
    insertCommentAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->solverOptionTableView->addAction(insertCommentAction);

    QAction* deleteAction = mContextMenu.addAction(Theme::icon(":/%1/delete-all"), "Delete selection", [this]() { deleteOption(); });
    deleteAction->setObjectName("actionDelete_option");
    deleteAction->setShortcut( QKeySequence("Ctrl+Delete") );
    deleteAction->setShortcutVisibleInContextMenu(true);
    deleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->solverOptionTableView->addAction(deleteAction);

    QAction* moveUpAction = mContextMenu.addAction(Theme::icon(":/%1/move-up"), "Move up", [this]() { moveOptionUp(); });
    moveUpAction->setObjectName("actionMoveUp_option");
    moveUpAction->setShortcut( QKeySequence("Ctrl+Up") );
    moveUpAction->setShortcutVisibleInContextMenu(true);
    moveUpAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->solverOptionTableView->addAction(moveUpAction);

    QAction* moveDownAction = mContextMenu.addAction(Theme::icon(":/%1/move-down"), "Move down", [this]() { moveOptionDown(); });
    moveDownAction->setObjectName("actionMoveDown_option");
    moveDownAction->setShortcut( QKeySequence("Ctrl+Down") );
    moveDownAction->setShortcutVisibleInContextMenu(true);
    moveDownAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->solverOptionTableView->addAction(moveDownAction);

    QAction* selectAll = mContextMenu.addAction("Select all", ui->solverOptionTableView, [this]() { selectAllOptions(); });
    selectAll->setObjectName("actionSelect_all");
    selectAll->setShortcut( QKeySequence("Ctrl+A") );
    selectAll->setShortcutVisibleInContextMenu(true);
    selectAll->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->solverOptionTableView->addAction(selectAll);

    QAction* anotehrSelectAll = mContextMenu.addAction("Select All", ui->solverOptionTreeView, [this]() { selectAllOptions(); });
    anotehrSelectAll->setObjectName("actionSelect_all");
    anotehrSelectAll->setShortcut( QKeySequence("Ctrl+A") );
    anotehrSelectAll->setShortcutVisibleInContextMenu(true);
    anotehrSelectAll->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->solverOptionTreeView->addAction(anotehrSelectAll);

    QAction* addThisOptionAction = mContextMenu.addAction(Theme::icon(":/%1/plus"), "Add this option", [this]() {
        QModelIndexList selection = ui->solverOptionTreeView->selectionModel()->selectedRows();
        if (selection.size()>0) {
            ui->solverOptionTableView->clearSelection();
            ui->solverOptionTreeView->clearSelection();
            addOptionFromDefinition(selection.at(0));
        }
    });
    addThisOptionAction->setObjectName("actionAddThisOption");
    addThisOptionAction->setShortcut( QKeySequence(Qt::Key_Return) );
    addThisOptionAction->setShortcutVisibleInContextMenu(true);
    addThisOptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->solverOptionTreeView->addAction(addThisOptionAction);

    QAction* deleteThisOptionAction = mContextMenu.addAction(Theme::icon(":/%1/delete-all"), "Remove this option", [this]() {
        findAndSelectionOptionFromDefinition();
        deleteOption();
    });
    deleteThisOptionAction->setObjectName("actionDeleteThisOption");
    deleteThisOptionAction->setShortcut( QKeySequence(Qt::Key_Delete) );
    deleteThisOptionAction->setShortcutVisibleInContextMenu(true);
    deleteThisOptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->solverOptionTreeView->addAction(deleteThisOptionAction);

    QAction* copyDefinitionOptionNameAction = mContextMenu.addAction("Copy option name\tCtrl+C", this,
                                                                     [this]() { copyDefinitionToClipboard( SolverOptionDefinitionModel::COLUMN_OPTION_NAME ); });
    copyDefinitionOptionNameAction->setObjectName("actionCopyDefinitionOptionName");
    copyDefinitionOptionNameAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->solverOptionTreeView->addAction(copyDefinitionOptionNameAction);

    QAction* copyDefinitionOptionDescriptionAction = mContextMenu.addAction("Copy option description", this,
                                                                            [this]() { copyDefinitionToClipboard( SolverOptionDefinitionModel::COLUMN_DESCIPTION ); });
    copyDefinitionOptionDescriptionAction->setObjectName("actionCopyDefinitionOptionDescription");
    copyDefinitionOptionDescriptionAction->setShortcut( QKeySequence("Shift+C") );
    copyDefinitionOptionDescriptionAction->setShortcutVisibleInContextMenu(true);
    copyDefinitionOptionDescriptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->solverOptionTreeView->addAction(copyDefinitionOptionDescriptionAction);

    QAction* copyDefinitionTextAction = mContextMenu.addAction("Copy definition text", this,
                                                               [this]() { copyDefinitionToClipboard( -1 ); });
    copyDefinitionTextAction->setObjectName("actionCopyDefinitionText");
    copyDefinitionTextAction->setShortcut( QKeySequence("Ctrl+Shift+C") );
    copyDefinitionTextAction->setShortcutVisibleInContextMenu(true);
    copyDefinitionTextAction->setShortcutContext(Qt::WidgetShortcut);
    ui->solverOptionTreeView->addAction(copyDefinitionTextAction);

    QAction* showDefinitionAction = mContextMenu.addAction("Show option definition", this, [this]() { showOptionDefinition(true); });
    showDefinitionAction->setObjectName("actionShowDefinition_option");
    showDefinitionAction->setShortcut( QKeySequence("Ctrl+F1") );
    showDefinitionAction->setShortcutVisibleInContextMenu(true);
    showDefinitionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->solverOptionTableView->addAction(showDefinitionAction);

    QAction* showDuplicationAction = mContextMenu.addAction("Show all options of the same definition", this, [this]() { showOptionRecurrence(); });
    showDuplicationAction->setObjectName("actionShowRecurrence_option");
    showDuplicationAction->setShortcut( QKeySequence("Shift+F1") );
    showDuplicationAction->setShortcutVisibleInContextMenu(true);
    showDuplicationAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->solverOptionTableView->addAction(showDuplicationAction);

    QAction* resizeColumns = mContextMenu.addAction("Resize columns to contents", this, [this]() { resizeColumnsToContents(); });
    resizeColumns->setObjectName("actionResize_columns");
    resizeColumns->setShortcut( QKeySequence("Ctrl+R") );
    resizeColumns->setShortcutVisibleInContextMenu(true);
    resizeColumns->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->solverOptionTableView->addAction(resizeColumns);
    ui->solverOptionTreeView->addAction(resizeColumns);
}

void SolverOptionWidget::updateEditActions(bool modified)
{
    ui->openAsTextButton->setEnabled(!modified);
}

void SolverOptionWidget::updateTableColumnSpan()
{
    ui->solverOptionTableView->clearSpans();
    QList<SolverOptionItem *> optionItems = mOptionTableModel->getCurrentListOfOptionItems();
    for(int i=0; i< optionItems.size(); ++i) {
        if (optionItems.at(i)->disabled)
            ui->solverOptionTableView->setSpan(i, 0, 1, ui->solverOptionTableView->model()->columnCount());
    }
}

bool SolverOptionWidget::isThereARow() const
{
    return (ui->solverOptionTableView->model()->rowCount() > 0);
}

bool SolverOptionWidget::isThereARowSelection() const
{
    QModelIndexList selection = ui->solverOptionTableView->selectionModel()->selectedRows();
    return (selection.count() > 0);
}

MainWindow *SolverOptionWidget::getMainWindow()
{
    const auto widgets = qApp->topLevelWidgets();
    for(QWidget *widget : widgets)
        if (MainWindow *mainWindow = qobject_cast<MainWindow*>(widget))
            return mainWindow;
    return nullptr;
}

QString SolverOptionWidget::getSolverName() const
{
    return mSolverName;
}

int SolverOptionWidget::getItemCount() const
{
    return ui->solverOptionTableView->model()->rowCount();
}

bool SolverOptionWidget::isViewCompact() const
{
    return ui->compactViewCheckBox->isChecked();
}

bool SolverOptionWidget::isEverySelectionARow() const
{
    QModelIndexList selection = ui->solverOptionTableView->selectionModel()->selectedRows();
    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();

    return ((selection.count() > 0) && (indexSelection.count() % ui->solverOptionTableView->model()->columnCount() == 0));
}

void SolverOptionWidget::selectSearchField() const
{
    ui->solverOptionSearch->setFocus();
}

void SolverOptionWidget::setFileChangedExtern(bool value)
{
    mFileHasChangedExtern = value;
}

void SolverOptionWidget::on_newTableRowDropped(const QModelIndex &index)
{
    disconnect(ui->solverOptionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SolverOptionWidget::findAndSelectionOptionFromDefinition);
    updateTableColumnSpan();
    ui->solverOptionTableView->selectRow(index.row());

    QString optionName = ui->solverOptionTableView->model()->data(index, Qt::DisplayRole).toString();
    QModelIndexList definitionItems = ui->solverOptionTreeView->model()->match(ui->solverOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     optionName, 1);
    mOptionTokenizer->getOption()->setModified(optionName, true);
    for(const QModelIndex &item : std::as_const(definitionItems)) {
        ui->solverOptionTreeView->model()->setData(item, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);
    }

    emit itemCountChanged(ui->solverOptionTableView->model()->rowCount());

    if (mOptionTokenizer->getOption()->getOptionType(optionName) != optTypeEnumStr &&
        mOptionTokenizer->getOption()->getOptionType(optionName) != optTypeEnumInt &&
        mOptionTokenizer->getOption()->getOptionSubType(optionName) != optsubNoValue)
        ui->solverOptionTableView->edit( mOptionTableModel->index(index.row(), SolverOptionTableModel::COLUMN_OPTION_VALUE ));

    showOptionDefinition(true);
    connect(ui->solverOptionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SolverOptionWidget::findAndSelectionOptionFromDefinition, Qt::UniqueConnection);
}

void SolverOptionWidget::setModified(bool modified)
{
    mModified = modified;
    updateEditActions(mModified);
    emit modificationChanged( mModified );
}

bool SolverOptionWidget::saveAs(const QString &location)
{
    setModified(false);
    bool success = mOptionTokenizer->writeOptionFile(mOptionTableModel->getCurrentListOfOptionItems(), location, mEncoding);
    if (mLocation != location) {
        bool warning = false;
        if (FileType::from(FileKind::Opt) == FileType::from(QFileInfo(location).fileName()) &&
            QString::compare(QFileInfo(mLocation).completeBaseName(), QFileInfo(location).completeBaseName(), Qt::CaseInsensitive) != 0) {
            mOptionTokenizer->logger()->append(QString("Solver option file name '%1' has been changed. Saved options into '%2' may cause solver option editor to display the contents improperly.")
                                                       .arg(QFileInfo(mLocation).completeBaseName(), QFileInfo(location).fileName())
                                               , LogMsgType::Warning);
            warning = true;
        }
        if (FileType::from(FileKind::Opt) != FileType::from(QFileInfo(location).fileName()) &&
            FileType::from(FileKind::Pf) != FileType::from(QFileInfo(location).fileName())) {
            mOptionTokenizer->logger()->append(QString("Unrecognized file suffix '%1'. Saved options into '%2' may cause this editor to display the contents improperly.")
                                                       .arg(QFileInfo(location).fileName(), QFileInfo(location).fileName())
                                               , LogMsgType::Warning);
            warning = true;
        }
        if (!warning)
            mOptionTokenizer->logger()->append(QString("Saved options into %1").arg(location), LogMsgType::Info);
        mLocation = location;
    } else {
        mOptionTokenizer->logger()->append(QString("Saved options into %1").arg(mLocation), LogMsgType::Info);
    }
    return success;
}

bool SolverOptionWidget::isInFocus(QWidget *focusWidget) const
{
    return (focusWidget==ui->solverOptionTableView || focusWidget==ui->solverOptionTreeView);
}

QString SolverOptionWidget::getSelectedOptionName(QWidget *widget) const
{
    if (widget == ui->solverOptionTableView) {
        QModelIndexList selection = ui->solverOptionTableView->selectionModel()->selectedIndexes();
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            QVariant headerData = ui->solverOptionTableView->model()->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole);
            if (Qt::CheckState(headerData.toUInt())==Qt::PartiallyChecked) {
                return "";
            }
            QVariant data = ui->solverOptionTableView->model()->data( index.sibling(index.row(),0) );
            if (mOptionTokenizer->getOption()->isValid(data.toString()))
               return data.toString();
            else if (mOptionTokenizer->getOption()->isASynonym(data.toString()))
                    return mOptionTokenizer->getOption()->getNameFromSynonym(data.toString());
            else
               return "";
        }
    } else if (widget == ui->solverOptionTreeView) {
        QModelIndexList selection = ui->solverOptionTreeView->selectionModel()->selectedRows();
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            QModelIndex  parentIndex =  ui->solverOptionTreeView->model()->parent(index);
            if (parentIndex.row() >= 0) {
                return ui->solverOptionTreeView->model()->data( parentIndex.sibling(parentIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ).toString();
            } else {
                return ui->solverOptionTreeView->model()->data( index.sibling(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ).toString();
            }
        }
    }
    return "";
}


}
}
}
