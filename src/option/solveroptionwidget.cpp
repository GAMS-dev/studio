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
#include <QStandardItemModel>
#include <QShortcut>
#include <QMessageBox>
#include <QFileDialog>
#include <QClipboard>

#include "solveroptionwidget.h"
#include "ui_solveroptionwidget.h"

#include "optioncompleterdelegate.h"
#include "optionsortfilterproxymodel.h"
#include "solveroptiondefinitionmodel.h"
#include "solveroptionsetting.h"
#include "mainwindow.h"
#include "editors/systemlogedit.h"

namespace gams {
namespace studio {
namespace option {

SolverOptionWidget::SolverOptionWidget(QString solverName, QString optionFilePath, FileId id, QTextCodec* codec, QWidget *parent) :
          QWidget(parent),
          ui(new Ui::SolverOptionWidget),
          mFileId(id),
          mLocation(optionFilePath),
          mSolverName(solverName),
          mCodec(codec)
{
    ui->setupUi(this);

    addActions();

    init();
}

SolverOptionWidget::~SolverOptionWidget()
{
    delete ui;
    delete mOptionTokenizer;
    delete mOptionTableModel;
}

bool SolverOptionWidget::init()
{
    mOptionTokenizer = new OptionTokenizer(QString("opt%1.def").arg(mSolverName));

    SystemLogEdit* logEdit = new SystemLogEdit(this);
    mOptionTokenizer->provideLogger(logEdit);
    ui->solverOptionTabWidget->addTab( logEdit, "Message" );

    QList<SolverOptionItem *> optionItem = mOptionTokenizer->readOptionFile(mLocation, mCodec);
    mOptionTableModel = new SolverOptionTableModel(optionItem, mOptionTokenizer,  this);
    ui->solverOptionTableView->setModel( mOptionTableModel );
    updateTableColumnSpan();

    ui->solverOptionTableView->setItemDelegate( new OptionCompleterDelegate(mOptionTokenizer, ui->solverOptionTableView));
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

    ui->solverOptionTableView->horizontalHeader()->setStretchLastSection(true);
    ui->solverOptionTableView->setColumnHidden( mOptionTableModel->getColumnEntryNumber(), true);
    ui->solverOptionTableView->resizeColumnToContents(0);
    ui->solverOptionTableView->resizeColumnToContents(1);
    //    ui->solverOptionTableView->resizeColumnToContents(2);

    ui->solverOptionTableView->horizontalHeader()->setHighlightSections(false);
    ui->solverOptionTableView->verticalHeader()->setDefaultSectionSize(ui->solverOptionTableView->verticalHeader()->minimumSectionSize());

    QList<OptionGroup> optionGroupList = mOptionTokenizer->getOption()->getOptionGroupList();
    int groupsize = 0;
    for(OptionGroup group : optionGroupList) {
        if (group.hidden)
            continue;
        else
            ++groupsize;
    }

    QStandardItemModel* groupModel = new QStandardItemModel(groupsize+1, 3);
    int i = 0;
    groupModel->setItem(0, 0, new QStandardItem("--- All Options ---"));
    groupModel->setItem(0, 1, new QStandardItem("0"));
    groupModel->setItem(0, 2, new QStandardItem("All Options"));
    for(OptionGroup group : optionGroupList) {
        if (group.hidden)
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

    ui->solverOptionTreeView->setModel( proxymodel );
    ui->solverOptionTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->solverOptionTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->solverOptionTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->solverOptionTreeView->setDragEnabled(true);
    ui->solverOptionTreeView->setDragDropMode(QAbstractItemView::DragOnly);

    ui->solverOptionTreeView->setItemsExpandable(true);
    ui->solverOptionTreeView->setSortingEnabled(true);
    ui->solverOptionTreeView->sortByColumn(0, Qt::AscendingOrder);
    ui->solverOptionTreeView->resizeColumnToContents(0);
    ui->solverOptionTreeView->resizeColumnToContents(2);
    ui->solverOptionTreeView->resizeColumnToContents(3);
    ui->solverOptionTreeView->setAlternatingRowColors(true);
    ui->solverOptionTreeView->setExpandsOnDoubleClick(false);
    if (!mOptionTokenizer->getOption()->isSynonymDefined())
        ui->solverOptionTreeView->setColumnHidden( 1, true);
    ui->solverOptionTreeView->setColumnHidden(OptionDefinitionModel::COLUMN_ENTRY_NUMBER, true);

    ui->solverOptionHSplitter->setSizes(QList<int>({25, 75}));
    ui->solverOptionVSplitter->setSizes(QList<int>({80, 20}));

    setModified(false);

    if (!mOptionTokenizer->getOption()->available())  {
        ui->solverOptionSearch->setReadOnly(true);
        ui->compactViewCheckBox->setEnabled(false);

        QString msg1 = QString("Unable to open %1 Properly").arg(mLocation);
        QString msg2 = QString("%1 is not a valid solver name").arg(mSolverName);
        mOptionTokenizer->logger()->append(msg2, LogMsgType::Warning);
        mOptionTokenizer->logger()->append(QString("Unable to load options from %1 properly.").arg(mLocation), LogMsgType::Warning);

        QMessageBox msgBox;
        msgBox.setWindowTitle("Problem openning Solver Option File");
        msgBox.setText(QString("%1.\nProblem: %2.").arg(msg1).arg(msg2));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();

        return false;
    }
    else {
        SolverOptionSetting* settingEdit = new SolverOptionSetting(mOptionTokenizer->getOption()->getEOLChars(),
                                                                   mOptionTokenizer->getOption()->getDefaultSeparator(),
                                                                   mOptionTokenizer->getOption()->getDefaultStringquote(),
                                                                   this);
        mOptionTokenizer->on_EOLCommentChar_changed( settingEdit->getDefaultEOLCharacter() );
        ui->solverOptionTabWidget->addTab( settingEdit, "Setting" );

        connect(settingEdit, &SolverOptionSetting::EOLCharChanged, [=](QChar ch){
                mOptionTokenizer->on_EOLCommentChar_changed(ch);
        });

        connect(ui->solverOptionTableView->verticalHeader(), &QHeaderView::sectionClicked, this, &SolverOptionWidget::on_selectAndToggleRow);
        connect(ui->solverOptionTableView->verticalHeader(), &QHeaderView::customContextMenuRequested, this, &SolverOptionWidget::showOptionContextMenu);
        connect(ui->solverOptionTableView, &QTableView::customContextMenuRequested, this, &SolverOptionWidget::showOptionContextMenu);
        connect(mOptionTableModel, &SolverOptionTableModel::newTableRowDropped, this, &SolverOptionWidget::on_newTableRowDropped);

        connect(ui->solverOptionSearch, &QLineEdit::textChanged,
                proxymodel, static_cast<void(QSortFilterProxyModel::*)(const QString &)>(&QSortFilterProxyModel::setFilterRegExp));

        connect(ui->solverOptionTreeView, &QAbstractItemView::doubleClicked, this, &SolverOptionWidget::addOptionFromDefinition);
        connect(ui->solverOptionTreeView, &QTreeView::customContextMenuRequested, this, &SolverOptionWidget::showDefinitionContextMenu);

        connect(ui->solverOptionGroup, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
             optdefmodel->loadOptionFromGroup( groupModel->data(groupModel->index(index, 1)).toInt() );
        });

        connect(mOptionTableModel, &QAbstractTableModel::dataChanged, this, &SolverOptionWidget::on_dataItemChanged);
        connect(mOptionTableModel, &QAbstractTableModel::dataChanged, mOptionTableModel, &SolverOptionTableModel::on_updateSolverOptionItem);
        connect(mOptionTableModel, &SolverOptionTableModel::solverOptionModelChanged, optdefmodel, &SolverOptionDefinitionModel::modifyOptionDefinition);
        connect(mOptionTableModel, &SolverOptionTableModel::solverOptionItemModelChanged, optdefmodel, &SolverOptionDefinitionModel::modifyOptionDefinitionItem);
        connect(mOptionTableModel, &SolverOptionTableModel::solverOptionItemRemoved, mOptionTableModel, &SolverOptionTableModel::on_removeSolverOptionItem);
        connect(mOptionTableModel, &SolverOptionTableModel::optionDefinitionSelected, this, &SolverOptionWidget::findAndSelectionOptionFromDefinition);

        connect(settingEdit, &SolverOptionSetting::addOptionDescriptionAsComment, this, &SolverOptionWidget::on_addEOLCommentChanged);
        connect(settingEdit, &SolverOptionSetting::addOptionDescriptionAsComment, mOptionTableModel, &SolverOptionTableModel::on_addEOLCommentCheckBox_stateChanged);

        connect(settingEdit, &SolverOptionSetting::overrideExistingOptionChanged, this, &SolverOptionWidget::on_overrideExistingOptionChanged);
        connect(settingEdit, &SolverOptionSetting::overrideExistingOptionChanged, mOptionTableModel, &SolverOptionTableModel::on_overrideExistingOptionChanged);
        connect(settingEdit, &SolverOptionSetting::addCommentAboveChanged, this, &SolverOptionWidget::on_addCommentAboveChanged);
        connect(settingEdit, &SolverOptionSetting::addCommentAboveChanged, mOptionTableModel, &SolverOptionTableModel::on_addCommentAbove_stateChanged);
        connect(settingEdit, &SolverOptionSetting::addCommentAboveChanged, optdefmodel, &SolverOptionDefinitionModel::on_addCommentAbove_stateChanged);

        connect(this, &SolverOptionWidget::compactViewChanged, optdefmodel, &SolverOptionDefinitionModel::on_compactViewChanged);

        mOptionTokenizer->logger()->append(QString("Loading options from %1").arg(mLocation), LogMsgType::Info);
        return true;
    }
}

bool SolverOptionWidget::isInFocused(QWidget *focusWidget)
{
    return (focusWidget==ui->solverOptionTableView || focusWidget==ui->solverOptionTreeView);
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
    bool thereIsAnIndexSelection = indexSelection.count();

    for(QModelIndex index : indexSelection) {
        ui->solverOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }
    QModelIndexList selection = ui->solverOptionTableView->selectionModel()->selectedRows();
    bool thereIsARowSelection = isThereARowSelection();
    bool viewIsCompact = isViewCompact();

    QMenu menu(this);
    QAction* moveUpAction = nullptr;
    QAction* moveDownAction = nullptr;
    for(QAction* action : this->actions()) {
        if (action->objectName().compare("actionToggle_comment")==0) {
            action->setVisible( !viewIsCompact && thereIsARowSelection );
            menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionInsert_option")==0) {
            action->setVisible( !viewIsCompact && (!isThereARow() || isThereARowSelection()) );
            menu.addAction(action);
        } else if (action->objectName().compare("actionInsert_comment")==0) {
            action->setVisible( !viewIsCompact && (!isThereARow() || isThereARowSelection()) );
            menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionDelete_option")==0) {
            action->setVisible( !viewIsCompact && thereIsARowSelection );
            menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionMoveUp_option")==0) {
            action->setVisible( !viewIsCompact && thereIsARowSelection && (selection.first().row() > 0) );
            menu.addAction(action);
        } else if (action->objectName().compare("actionMoveDown_option")==0) {
            action->setVisible( !viewIsCompact && thereIsARowSelection && (selection.last().row() < mOptionTableModel->rowCount()-1) );
            menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionSelect_all")==0) {
            action->setVisible( thereIsAnIndexSelection );
            menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionShowDefinition_option")==0) {

            bool thereIsAnOptionSelection = false;
            for (QModelIndex s : selection) {
                QVariant data = ui->solverOptionTableView->model()->headerData(s.row(), Qt::Vertical,  Qt::CheckStateRole);
                if (Qt::CheckState(data.toUInt())!=Qt::PartiallyChecked) {
                    thereIsAnOptionSelection = true;
                    break;
                }
            }
            action->setVisible( thereIsAnOptionSelection );
            menu.addAction(action);
        }
    }

    if (thereIsARowSelection) {
        // move up and down actions are disabled when selection are not contiguous
        QList<SolverOptionItem*> items = mOptionTableModel->getCurrentListOfOptionItems();
        QModelIndex index = selection.at(0);
        if (index.row()==0 && moveUpAction)
            moveUpAction->setVisible(false);
        if (index.row()+1 == ui->solverOptionTableView->model()->rowCount() && moveDownAction)
            moveDownAction->setVisible(false);

        int row = index.row();
        int i = 1;
        for (i=1; i<selection.count(); ++i) {
            index = selection.at(i);
            if (row+1 != index.row()) {
                break;
            }
            row = index.row();
        }
        if (i != selection.count()) {
           if (moveUpAction)
               moveUpAction->setVisible(false);
           if (moveDownAction)
               moveDownAction->setVisible(false);
        }
    }

    menu.exec(ui->solverOptionTableView->viewport()->mapToGlobal(pos));
}

void SolverOptionWidget::showDefinitionContextMenu(const QPoint &pos)
{
    QModelIndexList selection = ui->solverOptionTreeView->selectionModel()->selectedRows();
    if (selection.count() <= 0)
        return;

    bool viewIsCompact = isViewCompact();
    bool hasSelectionBeenAdded = (selection.size()>0);
    // assume single selection
    for (QModelIndex idx : selection) {
        QModelIndex parentIdx = ui->solverOptionTreeView->model()->parent(idx);
        QVariant data = (parentIdx.row() < 0) ?  ui->solverOptionTreeView->model()->data(idx, Qt::CheckStateRole)
                                              : ui->solverOptionTreeView->model()->data(parentIdx, Qt::CheckStateRole);
        hasSelectionBeenAdded = (Qt::CheckState(data.toInt()) == Qt::Checked);
    }

    QMenu menu(this);
    for(QAction* action : this->actions()) {
        if (action->objectName().compare("actionFindThisOption")==0) {
            action->setVisible(  hasSelectionBeenAdded );
            menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionAddThisOption")==0) {
            action->setVisible( !hasSelectionBeenAdded && !viewIsCompact );
            menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionDeleteThisOption")==0) {
            action->setVisible( hasSelectionBeenAdded && !viewIsCompact );
            menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionCopyDefinitionText")==0) {
            menu.addAction(action);
        } else if (action->objectName().compare("actionCopyDefinitionOptionName")==0) {
            menu.addAction(action);
        } else if (action->objectName().compare("actionCopyDefinitionOptionDescription")==0) {
            menu.addAction(action);
        }
    }

    menu.exec(ui->solverOptionTreeView->viewport()->mapToGlobal(pos));
}

void SolverOptionWidget::addOptionFromDefinition(const QModelIndex &index)
{
    if (isViewCompact())
        return;

    setModified(true);

    disconnect(mOptionTableModel, &QAbstractTableModel::dataChanged, mOptionTableModel, &SolverOptionTableModel::on_updateSolverOptionItem);

    QModelIndex parentIndex =  ui->solverOptionTreeView->model()->parent(index);
    QModelIndex optionNameIndex = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) :
                                                          ui->solverOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ;
//    QModelIndex synonymIndex = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_SYNONYM) :
//                                                       ui->solverOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_SYNONYM) ;
    QModelIndex defValueIndex = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_DEF_VALUE) :
                                                        ui->solverOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_DEF_VALUE) ;
    QModelIndex selectedValueIndex = (parentIndex.row()<0) ? defValueIndex :
                                                             ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex) ;

    QVariant data = ui->solverOptionTreeView->model()->data(optionNameIndex, Qt::CheckStateRole);
    if (Qt::CheckState(data.toUInt())==Qt::Checked) {
        findAndSelectionOptionFromDefinition();
    }

    bool replaceExistingEntry = false;
    QString optionNameData = ui->solverOptionTreeView->model()->data(optionNameIndex).toString();
    QModelIndex optionIdIndex = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER) :
                                                        ui->solverOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER) ;
    QString optionIdData = ui->solverOptionTreeView->model()->data(optionIdIndex).toString();

    int rowToBeAdded = ui->solverOptionTableView->model()->rowCount();
    if (overrideExistingOption) {
        QModelIndexList indices = ui->solverOptionTableView->model()->match(ui->solverOptionTableView->model()->index(0, mOptionTableModel->getColumnEntryNumber()),
                                                                            Qt::DisplayRole,
                                                                            optionIdData, Qt::MatchRecursive);
        bool singleEntryExisted = (indices.size()==1);
        bool multipleEntryExisted = (indices.size()>1);
        if (singleEntryExisted ) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Option Entry exists");
            msgBox.setText("Option '" + optionNameData + "' already exists in your option file.");
            msgBox.setInformativeText("Do you want to add new entry or replace the entry?");
            msgBox.setStandardButtons(QMessageBox::Abort);
            msgBox.addButton("Replace existing entry", QMessageBox::ActionRole);
            msgBox.addButton("Add new entry", QMessageBox::ActionRole);

            switch(msgBox.exec()) {
            case 0: // replace
                replaceExistingEntry = true;
                rowToBeAdded = indices.at(0).row();
                break;
            case 1: // add
                break;
            case QMessageBox::Abort:
                return;
            }
        } else if (multipleEntryExisted) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Multiple Option Entries exist");
            msgBox.setText("Multiple entries of Option '" + optionNameData + "' already exists in your option file.");
            msgBox.setInformativeText("Do you want to replace first entry (and delete other entries) or add new entry?");
            msgBox.setStandardButtons(QMessageBox::Abort);
            msgBox.addButton("Replace first entry and delete other entries", QMessageBox::ActionRole);
            msgBox.addButton("Add new entry", QMessageBox::ActionRole);

            switch(msgBox.exec()) {
            case 0: // delete and replace
                disconnect(mOptionTableModel, &SolverOptionTableModel::solverOptionItemRemoved, mOptionTableModel, &SolverOptionTableModel::on_removeSolverOptionItem);
                deleteOption(true);
                connect(mOptionTableModel, &SolverOptionTableModel::solverOptionItemRemoved, mOptionTableModel, &SolverOptionTableModel::on_removeSolverOptionItem);
                replaceExistingEntry = true;
                indices = ui->solverOptionTableView->model()->match(ui->solverOptionTableView->model()->index(0, mOptionTableModel->getColumnEntryNumber()),
                                                                    Qt::DisplayRole,
                                                                    optionIdData, Qt::MatchRecursive);
                rowToBeAdded = indices.at(0).row();
                break;
            case 1: // add
                break;
            case QMessageBox::Abort:
                return;
            }

        } // else entry not exist
    }
qDebug() << "rowToBeAdd=" << rowToBeAdded << (rowToBeAdded != ui->solverOptionTableView->model()->rowCount() ? ", no" : ", yes");
//    QString synonymData = ui->solverOptionTreeView->model()->data(synonymIndex).toString();
    QString selectedValueData = ui->solverOptionTreeView->model()->data(selectedValueIndex).toString();
qDebug() << "01";
    mOptionTokenizer->getOption()->setModified(optionNameData, true);
    ui->solverOptionTreeView->model()->setData(optionNameIndex, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);

    bool firstTime = (ui->solverOptionTableView->model()->rowCount()==0);
    if (addCommentAbove) { // insert comment description row
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
    if (addEOLComment) {
        QModelIndex commentIndex = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_DESCIPTION)
                                                         : ui->solverOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_DESCIPTION);
        QString commentData = ui->solverOptionTreeView->model()->data(commentIndex).toString();
        QModelIndex insertEOLCommentIndex = ui->solverOptionTableView->model()->index(rowToBeAdded, SolverOptionTableModel::COLUMN_EOL_COMMENT);
        ui->solverOptionTableView->model()->setData( insertEOLCommentIndex, commentData, Qt::EditRole);
    }
    int optionEntryNumber = mOptionTokenizer->getOption()->getOptionDefinition(optionNameData).number;
    ui->solverOptionTableView->model()->setData( insertNumberIndex, optionEntryNumber, Qt::EditRole);
    ui->solverOptionTableView->selectRow(rowToBeAdded);

    QString text = mOptionTableModel->getOptionTableEntry(insertNumberIndex.row());
    if (replaceExistingEntry)
        mOptionTokenizer->logger()->append(QString("Option entry '%1' has been replaced").arg(text), LogMsgType::Info);
    else
        mOptionTokenizer->logger()->append(QString("Option entry '%1' has been added").arg(text), LogMsgType::Info);

    int lastColumn = ui->solverOptionTableView->model()->columnCount()-1;
    int lastRow = rowToBeAdded;
    int firstRow = (addCommentAbove ? lastRow-2 : lastRow);
    if (addCommentAbove && firstTime) {
        firstRow = 0;
        lastRow = 2;
    }
    mOptionTableModel->on_updateSolverOptionItem( ui->solverOptionTableView->model()->index(firstRow, lastColumn),
                                                  ui->solverOptionTableView->model()->index(lastRow, lastColumn),
                                                  {Qt::EditRole});

    connect(mOptionTableModel, &QAbstractTableModel::dataChanged, mOptionTableModel, &SolverOptionTableModel::on_updateSolverOptionItem);

    updateTableColumnSpan();

    showOptionDefinition();

    emit itemCountChanged(ui->solverOptionTableView->model()->rowCount());

    if (parentIndex.row()<0)
        ui->solverOptionTableView->edit(insertValueIndex);
}


void SolverOptionWidget::on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    setModified(true);
}

bool SolverOptionWidget::saveOptionFile(const QString &location)
{
    return saveAs(location);
}

void SolverOptionWidget::on_reloadSolverOptionFile(QTextCodec* codec)
{
     if (mCodec != codec)
         mOptionTokenizer->logger()->append(QString("Loading options from %1 with %2 encoding").arg(mLocation).arg(QString(codec->name())), LogMsgType::Info);
     else if (mFileHasChangedExtern)
              mOptionTokenizer->logger()->append(QString("Loading options from %1").arg(mLocation), LogMsgType::Info);
     mCodec = codec;
     mOptionTableModel->reloadSolverOptionModel( mOptionTokenizer->readOptionFile(mLocation, codec) );
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
    if (ui->compactViewCheckBox->isChecked())
        return;

    mOptionTableModel->on_toggleRowHeader(logicalIndex);
    updateTableColumnSpan();
    setModified(true);
}

void SolverOptionWidget::on_compactViewCheckBox_stateChanged(int checkState)
{
    bool isViewCompact = (Qt::CheckState(checkState) == Qt::Checked);
    if (isViewCompact) {
        mOptionTokenizer->logger()->append(QString("Compact View is activated, comments are hidden and only edit action is allowed"), LogMsgType::Info);
        if (mOptionTokenizer->getOption()->isEOLCharDefined())
           ui->solverOptionTableView->hideColumn(SolverOptionTableModel::COLUMN_EOL_COMMENT);
    } else {
        mOptionTokenizer->logger()->append(QString("Compact View is deactivated, comments are now visible and all actions are allowed"), LogMsgType::Info);
        if (mOptionTokenizer->getOption()->isEOLCharDefined())
           ui->solverOptionTableView->showColumn(SolverOptionTableModel::COLUMN_EOL_COMMENT);
    }
    for(int i = 0; i < mOptionTableModel->rowCount(); ++i) {
       if (mOptionTableModel->headerData(i, Qt::Vertical, Qt::CheckStateRole).toUInt()==Qt::PartiallyChecked) {
          if (isViewCompact)
              ui->solverOptionTableView->hideRow(i);
          else
             ui->solverOptionTableView->showRow(i);
       }
    }
    emit compactViewChanged(isViewCompact);
}

void SolverOptionWidget::on_openAsTextButton_clicked(bool checked)
{
    Q_UNUSED(checked);
    MainWindow* main = getMainWindow();
    if (!main) return;

    emit main->projectRepo()->closeFileEditors(fileId());

    FileMeta* fileMeta = main->fileRepo()->fileMeta(fileId());
    ProjectFileNode* fileNode = main->projectRepo()->findFileNode(this);
    ProjectRunGroupNode* runGroup = (fileNode ? fileNode->assignedRunGroup() : nullptr);

    emit main->projectRepo()->openFile(fileMeta, true, runGroup, -1, true);
}

void SolverOptionWidget::on_overrideExistingOptionChanged(int checkState)
{
    overrideExistingOption = (Qt::CheckState(checkState) == Qt::Checked);
}

void SolverOptionWidget::on_addCommentAboveChanged(int checkState)
{
    addCommentAbove = (Qt::CheckState(checkState) == Qt::Checked);
}

void SolverOptionWidget::on_addEOLCommentChanged(int checkState)
{
    addEOLComment = (Qt::CheckState(checkState) == Qt::Checked);
}

void SolverOptionWidget::showOptionDefinition()
{
    if (ui->solverOptionTableView->model()->rowCount() <= 0)
        return;

    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();
    if (indexSelection.count() <= 0)
        return;

    ui->solverOptionGroup->setCurrentIndex(0);
    ui->solverOptionSearch->clear();
    ui->solverOptionTreeView->selectionModel()->clearSelection();
    selectAnOption();

    QModelIndexList selection = ui->solverOptionTableView->selectionModel()->selectedRows();
    if (selection.count() > 0) {
        for (int i=0; i<selection.count(); i++) {
            QModelIndex index = selection.at(i);
            if (Qt::CheckState(ui->solverOptionTableView->model()->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole).toUInt())==Qt::PartiallyChecked)
                continue;

            QVariant optionId = ui->solverOptionTableView->model()->data( index.sibling(index.row(), mOptionTableModel->getColumnEntryNumber()), Qt::DisplayRole);
            QModelIndexList indices = ui->solverOptionTreeView->model()->match(ui->solverOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_ENTRY_NUMBER),
                                                                               Qt::DisplayRole,
                                                                               optionId.toString(), 1); //, Qt::MatchRecursive);
            for(QModelIndex idx : indices) {
                QModelIndex  parentIndex =  ui->solverOptionTreeView->model()->parent(index);

                if (parentIndex.row() < 0 && !ui->solverOptionTreeView->isExpanded(idx))
                    ui->solverOptionTreeView->expand(idx);
                QItemSelection selection = ui->solverOptionTreeView->selectionModel()->selection();
                selection.select(ui->solverOptionTreeView->model()->index(idx.row(), 0),
                                ui->solverOptionTreeView->model()->index(idx.row(), ui->solverOptionTreeView->model()->columnCount()-1));
                ui->solverOptionTreeView->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
            }
            if (indices.size() > 0)
                ui->solverOptionTreeView->scrollTo(indices.first(), QAbstractItemView::PositionAtCenter);
        }
    }
}

void SolverOptionWidget::copyDefinitionToClipboard(int column)
{
    if (ui->solverOptionTreeView->selectionModel()->selectedRows().count() <= 0)
        return;

    QModelIndex index = ui->solverOptionTreeView->selectionModel()->selectedRows().at(0);

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
    ui->solverOptionTableView->selectionModel()->clearSelection();
    QModelIndex index = ui->solverOptionTreeView->selectionModel()->currentIndex();
    QModelIndex parentIndex =  ui->solverOptionTreeView->model()->parent(index);

    QModelIndex idx = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index( index.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER )
                                            : ui->solverOptionTreeView->model()->index( parentIndex.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER );
    QVariant data = ui->solverOptionTreeView->model()->data( idx, Qt::DisplayRole );
    QModelIndexList indices = ui->solverOptionTableView->model()->match(ui->solverOptionTableView->model()->index(0, mOptionTableModel->getColumnEntryNumber()),
                                                                       Qt::DisplayRole,
                                                                       data.toString(), Qt::MatchRecursive);
    ui->solverOptionTableView->clearSelection();
    QItemSelection selection;
    for(QModelIndex i :indices) {
        QModelIndex leftIndex  = ui->solverOptionTableView->model()->index(i.row(), 0);
        QModelIndex rightIndex = ui->solverOptionTableView->model()->index(i.row(), ui->solverOptionTableView->model()->columnCount() -1);

        QItemSelection rowSelection(leftIndex, rightIndex);
        selection.merge(rowSelection, QItemSelectionModel::Select);
    }

    ui->solverOptionTableView->selectionModel()->select(selection, QItemSelectionModel::Select);
}

void SolverOptionWidget::toggleCommentOption()
{
    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->solverOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if ( isViewCompact() || (isThereARow() && !isThereARowSelection() && !isEverySelectionARow()) )
        return;

    bool modified = false;
    QModelIndexList selection = ui->solverOptionTableView->selectionModel()->selectedRows();
    for(int i=0; i<selection.count(); ++i) {
        on_toggleRowHeader( selection.at(i).row() );
        modified = true;
    }
    if (modified) {
        setModified(modified);
    }
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
}

void SolverOptionWidget::insertOption()
{
    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->solverOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  (isViewCompact() || (isThereARow() && !isThereARowSelection() && !isEverySelectionARow()))
        return;

    disconnect(mOptionTableModel, &QAbstractTableModel::dataChanged, mOptionTableModel, &SolverOptionTableModel::on_updateSolverOptionItem);
    if (isThereARowSelection() ) {
        QModelIndex index = ui->solverOptionTableView->selectionModel()->selectedRows().at(0);
        ui->solverOptionTableView->model()->insertRows(index.row(), 1, QModelIndex());
        QModelIndex insertKeyIndex = ui->solverOptionTableView->model()->index(index.row(), SolverOptionTableModel::COLUMN_OPTION_KEY);
        QModelIndex insertValueIndex = ui->solverOptionTableView->model()->index(index.row(), SolverOptionTableModel::COLUMN_OPTION_VALUE);
        QModelIndex insertNumberIndex = ui->solverOptionTableView->model()->index(index.row(), mOptionTableModel->getColumnEntryNumber());

        ui->solverOptionTableView->model()->setHeaderData(index.row(), Qt::Vertical,
                                                        Qt::CheckState(Qt::Checked),
                                                        Qt::CheckStateRole );

        ui->solverOptionTableView->model()->setData( insertKeyIndex, OptionTokenizer::keyGeneratedStr, Qt::EditRole);
        ui->solverOptionTableView->model()->setData( insertValueIndex, OptionTokenizer::valueGeneratedStr, Qt::EditRole);
        if (mOptionTableModel->getColumnEntryNumber() > SolverOptionTableModel::COLUMN_EOL_COMMENT) {
            QModelIndex eolCommentIndex = ui->solverOptionTableView->model()->index(index.row(), SolverOptionTableModel::COLUMN_EOL_COMMENT);
            ui->solverOptionTableView->model()->setData( eolCommentIndex, OptionTokenizer::commentGeneratedStr, Qt::EditRole);
        }
        ui->solverOptionTableView->model()->setData( insertNumberIndex, -1, Qt::EditRole);

        ui->solverOptionTableView->clearSelection();
        ui->solverOptionTableView->selectRow(index.row());
    } else {
        ui->solverOptionTableView->model()->insertRows(ui->solverOptionTableView->model()->rowCount(), 1, QModelIndex());
        QModelIndex insertKeyIndex = ui->solverOptionTableView->model()->index(ui->solverOptionTableView->model()->rowCount()-1, SolverOptionTableModel::COLUMN_OPTION_KEY);
        QModelIndex insertValueIndex = ui->solverOptionTableView->model()->index(ui->solverOptionTableView->model()->rowCount()-1, SolverOptionTableModel::COLUMN_OPTION_VALUE);
        QModelIndex insertNumberIndex = ui->solverOptionTableView->model()->index(ui->solverOptionTableView->model()->rowCount()-1, mOptionTableModel->getColumnEntryNumber());

        ui->solverOptionTableView->model()->setHeaderData(ui->solverOptionTableView->model()->rowCount()-1, Qt::Vertical,
                                                          Qt::CheckState(Qt::Checked),
                                                          Qt::CheckStateRole );

        ui->solverOptionTableView->model()->setData( insertKeyIndex, OptionTokenizer::keyGeneratedStr, Qt::EditRole);
        ui->solverOptionTableView->model()->setData( insertValueIndex, OptionTokenizer::valueGeneratedStr, Qt::EditRole);
        if (mOptionTableModel->getColumnEntryNumber() > SolverOptionTableModel::COLUMN_EOL_COMMENT) {
            QModelIndex eolCommentIndex = ui->solverOptionTableView->model()->index(ui->solverOptionTableView->model()->rowCount()-1, SolverOptionTableModel::COLUMN_EOL_COMMENT);
            ui->solverOptionTableView->model()->setData( eolCommentIndex, OptionTokenizer::commentGeneratedStr, Qt::EditRole);
        }
        ui->solverOptionTableView->model()->setData( insertNumberIndex, -1, Qt::EditRole);

        ui->solverOptionTableView->selectRow(ui->solverOptionTableView->model()->rowCount()-1);
    }
    connect(mOptionTableModel, &QAbstractTableModel::dataChanged, mOptionTableModel, &SolverOptionTableModel::on_updateSolverOptionItem);
    updateTableColumnSpan();
    setModified(true);
    emit itemCountChanged(ui->solverOptionTableView->model()->rowCount());

}

void SolverOptionWidget::insertComment()
{
    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->solverOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  (isViewCompact() || (isThereARow() && !isThereARowSelection() && !isEverySelectionARow()))
        return;

    disconnect(mOptionTableModel, &QAbstractTableModel::dataChanged, mOptionTableModel, &SolverOptionTableModel::on_updateSolverOptionItem);
    if (isThereARowSelection() ) {

        QModelIndex index = ui->solverOptionTableView->selectionModel()->selectedRows().at(0);
        ui->solverOptionTableView->model()->insertRows(index.row(), 1, QModelIndex());
        QModelIndex insertKeyIndex = ui->solverOptionTableView->model()->index(index.row(), SolverOptionTableModel::COLUMN_OPTION_KEY);
        QModelIndex insertValueIndex = ui->solverOptionTableView->model()->index(index.row(), SolverOptionTableModel::COLUMN_OPTION_VALUE);
        QModelIndex insertNumberIndex = ui->solverOptionTableView->model()->index(index.row(), mOptionTableModel->getColumnEntryNumber());

        ui->solverOptionTableView->model()->setHeaderData(index.row(), Qt::Vertical,
                                                          Qt::CheckState(Qt::PartiallyChecked),
                                                          Qt::CheckStateRole );
        ui->solverOptionTableView->model()->setData( insertKeyIndex, OptionTokenizer::commentGeneratedStr, Qt::EditRole);
        ui->solverOptionTableView->model()->setData( insertValueIndex, "", Qt::EditRole);
        ui->solverOptionTableView->model()->setData( insertNumberIndex, -1, Qt::EditRole);

        ui->solverOptionTableView->selectRow(index.row());
    } else {
        ui->solverOptionTableView->model()->insertRows(ui->solverOptionTableView->model()->rowCount(), 1, QModelIndex());
        QModelIndex insertKeyIndex = ui->solverOptionTableView->model()->index(ui->solverOptionTableView->model()->rowCount()-1, SolverOptionTableModel::COLUMN_OPTION_KEY);
        QModelIndex insertValueIndex = ui->solverOptionTableView->model()->index(ui->solverOptionTableView->model()->rowCount()-1, SolverOptionTableModel::COLUMN_OPTION_VALUE);
        QModelIndex insertNumberIndex = ui->solverOptionTableView->model()->index(ui->solverOptionTableView->model()->rowCount()-1, mOptionTableModel->getColumnEntryNumber());
        ui->solverOptionTableView->model()->setHeaderData(ui->solverOptionTableView->model()->rowCount()-1, Qt::Vertical,
                                                          Qt::CheckState(Qt::PartiallyChecked),
                                                          Qt::CheckStateRole );

        ui->solverOptionTableView->model()->setData( insertKeyIndex, OptionTokenizer::commentGeneratedStr, Qt::EditRole);
        ui->solverOptionTableView->model()->setData( insertValueIndex, "", Qt::EditRole);
        ui->solverOptionTableView->model()->setData( insertNumberIndex, -1, Qt::EditRole);

        ui->solverOptionTableView->selectRow(ui->solverOptionTableView->model()->rowCount()-1);
    }
    connect(mOptionTableModel, &QAbstractTableModel::dataChanged, mOptionTableModel, &SolverOptionTableModel::on_updateSolverOptionItem);
    updateTableColumnSpan();
    setModified(true);
    emit itemCountChanged(ui->solverOptionTableView->model()->rowCount());
}

void SolverOptionWidget::deleteOption(bool keepFirstOne)
{
    QModelIndexList indexSelection = ui->solverOptionTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : indexSelection) {
        ui->solverOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  (isViewCompact() || !isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    if (isThereARowSelection() && isEverySelectionARow()) {
        QItemSelection selection( ui->solverOptionTableView->selectionModel()->selection() );

        QList<int> rows;
        for( const QModelIndex & index : selection.indexes() ) {
            if (!rows.contains(index.row()))
                rows.append( index.row() );
        }
        std::sort(rows.begin(), rows.end());
        int prev = -1;
        for(int i=rows.count()-1; i>=0; i--) {
            int current = rows[i];
            if (keepFirstOne && i==0)
                continue;
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
    for(QModelIndex index : indexSelection) {
        ui->solverOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  (isViewCompact() || !isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    QModelIndexList selection = ui->solverOptionTableView->selectionModel()->selectedRows();
    if  (selection.first().row() <= 0)
        return;

    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() < b.row(); });

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
    for(QModelIndex index : indexSelection) {
        ui->solverOptionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  (isViewCompact() || !isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    QModelIndexList selection = ui->solverOptionTableView->selectionModel()->selectedRows();
    if  (selection.last().row() >= mOptionTableModel->rowCount()-1)
        return;

    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() > b.row(); });

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

void SolverOptionWidget::addActions()
{
    QAction* commentAction = mContextMenu.addAction("Toggle comment/option selection", [this]() { toggleCommentOption(); });
    commentAction->setObjectName("actionToggle_comment");
    commentAction->setShortcut( tr("Ctrl+T") );
    commentAction->setShortcutVisibleInContextMenu(true);
    commentAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(commentAction);

    QAction* insertOptionAction = mContextMenu.addAction(QIcon(":/img/insert"), "insert new Option", [this]() { insertOption(); });
    insertOptionAction->setObjectName("actionInsert_option");
    insertOptionAction->setShortcut( tr("Ctrl+Insert") );
    insertOptionAction->setShortcutVisibleInContextMenu(true);
    insertOptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(insertOptionAction);

    QAction* insertCommentAction = mContextMenu.addAction(QIcon(":/img/insert"), "insert new Comment", [this]() { insertComment(); });
    insertCommentAction->setObjectName("actionInsert_comment");
    insertCommentAction->setShortcut( tr("Ctrl+Alt+Insert") );
    insertCommentAction->setShortcutVisibleInContextMenu(true);
    insertCommentAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(insertCommentAction);

    QAction* deleteAction = mContextMenu.addAction(QIcon(":/img/delete-all"), "delete Selection", [this]() { deleteOption(); });
    deleteAction->setObjectName("actionDelete_option");
    deleteAction->setShortcut( tr("Ctrl+Delete") );
    deleteAction->setShortcutVisibleInContextMenu(true);
    deleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(deleteAction);

    QAction* moveUpAction = mContextMenu.addAction(QIcon(":/img/move-up"), "move Up", [this]() { moveOptionUp(); });
    moveUpAction->setObjectName("actionMoveUp_option");
    moveUpAction->setShortcut( tr("Ctrl+Up") );
    moveUpAction->setShortcutVisibleInContextMenu(true);
    moveUpAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(moveUpAction);

    QAction* moveDownAction = mContextMenu.addAction(QIcon(":/img/move-down"), "move Down", [this]() { moveOptionDown(); });
    moveDownAction->setObjectName("actionMoveDown_option");
    moveDownAction->setShortcut( tr("Ctrl+Down") );
    moveDownAction->setShortcutVisibleInContextMenu(true);
    moveDownAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(moveDownAction);

    QAction* selectAll = mContextMenu.addAction("Select All", ui->solverOptionTableView, &QTableView::selectAll);
    selectAll->setObjectName("actionSelect_all");
    selectAll->setShortcut( tr("Ctrl+A") );
    selectAll->setShortcutVisibleInContextMenu(true);
    selectAll->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(selectAll);

    QAction* showDefinitionAction = mContextMenu.addAction("show Option Definition", [this]() { showOptionDefinition(); });
    showDefinitionAction->setObjectName("actionShowDefinition_option");
    showDefinitionAction->setShortcut( tr("Ctrl+F1") );
    showDefinitionAction->setShortcutVisibleInContextMenu(true);
    showDefinitionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(showDefinitionAction);


    QAction* findThisOptionAction = mContextMenu.addAction("show Selection of This Option", [this]() {
        findAndSelectionOptionFromDefinition();
    });
    findThisOptionAction->setObjectName("actionFindThisOption");
    findThisOptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(findThisOptionAction);

    QAction* addThisOptionAction = mContextMenu.addAction(QIcon(":/img/plus"), "add This Option", [this]() {
        QModelIndexList selection = ui->solverOptionTreeView->selectionModel()->selectedRows();
        if (selection.size()>0)
            addOptionFromDefinition(selection.at(0));
    });
    addThisOptionAction->setObjectName("actionAddThisOption");
    addThisOptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(addThisOptionAction);

    QAction* deleteThisOptionAction = mContextMenu.addAction(QIcon(":/img/delete-all"), "remove This Option", [this]() {
        findAndSelectionOptionFromDefinition();
        deleteOption();
    });
    deleteThisOptionAction->setObjectName("actionDeleteThisOption");
    deleteThisOptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(deleteThisOptionAction);

    QAction* copyDefinitionTextAction = mContextMenu.addAction("Copy Definition Text",
                                                               [this]() { copyDefinitionToClipboard( -1 ); });
    copyDefinitionTextAction->setObjectName("actionCopyDefinitionText");
    copyDefinitionTextAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(copyDefinitionTextAction);

    QAction* copyDefinitionOptionNameAction = mContextMenu.addAction("Copy Definition Option Name",
                                                                     [this]() { copyDefinitionToClipboard( SolverOptionDefinitionModel::COLUMN_OPTION_NAME ); });
    copyDefinitionOptionNameAction->setObjectName("actionCopyDefinitionOptionName");
    copyDefinitionOptionNameAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(copyDefinitionOptionNameAction);

    QAction* copyDefinitionOptionDescriptionAction = mContextMenu.addAction("Copy Definition Description",
                                                                            [this]() { copyDefinitionToClipboard( SolverOptionDefinitionModel::COLUMN_DESCIPTION ); });
    copyDefinitionOptionDescriptionAction->setObjectName("actionCopyDefinitionOptionDescription");
    copyDefinitionOptionDescriptionAction->setShortcutVisibleInContextMenu(true);
    copyDefinitionOptionDescriptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(copyDefinitionOptionDescriptionAction);

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
    for(QWidget *widget : qApp->topLevelWidgets())
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
    updateTableColumnSpan();
    ui->solverOptionTableView->selectRow(index.row());

    QString optionName = ui->solverOptionTableView->model()->data(index, Qt::DisplayRole).toString();
    QModelIndexList definitionItems = ui->solverOptionTreeView->model()->match(ui->solverOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     optionName, 1);
    mOptionTokenizer->getOption()->setModified(optionName, true);
    for(QModelIndex item : definitionItems) {
        ui->solverOptionTreeView->model()->setData(item, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);
    }

    emit itemCountChanged(ui->solverOptionTableView->model()->rowCount());

    ui->solverOptionTableView->edit( ui->solverOptionTableView->model()->index(index.row(), SolverOptionTableModel::COLUMN_OPTION_VALUE ));

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
    bool success = mOptionTokenizer->writeOptionFile(mOptionTableModel->getCurrentListOfOptionItems(), location, mCodec);
    if (mLocation != location) {
        bool warning = false;
        if (QString::compare(QFileInfo(mLocation).completeBaseName(), QFileInfo(location).completeBaseName(), Qt::CaseInsensitive)!=0 ) {
            mOptionTokenizer->logger()->append(QString("Solver option file name '%1' has been changed. Saved options into '%2' may cause solver option editor to display the contents improperly.")
                                                       .arg(QFileInfo(mLocation).completeBaseName()).arg(QFileInfo(location).fileName())
                                               , LogMsgType::Warning);
            warning = true;
        }
        if (FileType::from(FileKind::Opt) != FileType::from(QFileInfo(location).suffix())) {
            mOptionTokenizer->logger()->append(QString("Unrecognized file suffix '%1'. Saved options into '%2' may cause solver option editor to display the contents improperly.")
                                                       .arg(QFileInfo(location).suffix()).arg(QFileInfo(location).fileName())
                                               , LogMsgType::Warning);
            warning = true;
        }
        if (!warning)
            mOptionTokenizer->logger()->append(QString("Saved options into %1").arg(location), LogMsgType::Info);
        mLocation = location;
    }
    return success;
}

bool SolverOptionWidget::isAnOptionWidgetFocused(QWidget *focusWidget) const
{
    return (focusWidget==ui->solverOptionTableView || focusWidget==ui->solverOptionTreeView);
}

QString SolverOptionWidget::getSelectedOptionName(QWidget *widget) const
{
    QString selectedOptions = "";
    if (widget == ui->solverOptionTableView) {
        QModelIndexList selection = ui->solverOptionTableView->selectionModel()->selectedRows();
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            QVariant headerData = ui->solverOptionTableView->model()->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole);
            if (Qt::CheckState(headerData.toUInt())==Qt::Checked) {
                return "";
            }
            QVariant data = ui->solverOptionTableView->model()->data( index.sibling(index.row(),0) );
            if (mOptionTokenizer->getOption()->isDoubleDashedOption(data.toString())) {
               return "";
            } else if (mOptionTokenizer->getOption()->isASynonym(data.toString())) {
                return mOptionTokenizer->getOption()->getNameFromSynonym(data.toString());
            }
            return data.toString();
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
    return selectedOptions;
}


}
}
}
