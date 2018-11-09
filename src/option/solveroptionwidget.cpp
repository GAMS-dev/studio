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
#include <QMessageBox>
#include <QFileDialog>

#include "solveroptionwidget.h"
#include "ui_solveroptionwidget.h"

#include "optioncompleterdelegate.h"
#include "optionsortfilterproxymodel.h"
#include "solveroptiondefinitionmodel.h"
#include "mainwindow.h"
#include "editors/systemlogedit.h"

namespace gams {
namespace studio {
namespace option {

SolverOptionWidget::SolverOptionWidget(QString solverName, QString optionFilePath, QWidget *parent) :
          QWidget(parent),
          ui(new Ui::SolverOptionWidget),
          mLocation(optionFilePath),
          mSolverName(solverName)
{
    ui->setupUi(this);

    mOptionTokenizer = new OptionTokenizer(QString("opt%1.def").arg(solverName));

    SystemLogEdit* logEdit = new SystemLogEdit(this);
    mOptionTokenizer->provideLogger(logEdit);
    ui->solverOptionMessageTabWidget->addTab(logEdit, "Message");
    mOptionTokenizer->logger()->appendLog(QString("Loading options from %1").arg(mLocation), LogMsgType::Info);

    QList<SolverOptionItem *> optionItem = mOptionTokenizer->readOptionFile(mLocation);
    mOptionTableModel = new SolverOptionTableModel(optionItem, mOptionTokenizer,  this);
    ui->solverOptionTableView->setModel( mOptionTableModel );
    updateTableColumnSpan();

    ui->solverOptionTableView->setItemDelegate( new OptionCompleterDelegate(mOptionTokenizer, ui->solverOptionTableView));
    ui->solverOptionTableView->setEditTriggers(QAbstractItemView::DoubleClicked
                       | QAbstractItemView::EditKeyPressed
                       | QAbstractItemView::AnyKeyPressed );
    ui->solverOptionTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->solverOptionTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->solverOptionTableView->setAutoScroll(true);
    ui->solverOptionTableView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->solverOptionTableView->viewport()->setAcceptDrops(true);
    ui->solverOptionTableView->setDropIndicatorShown(true);
    ui->solverOptionTableView->setDragDropMode(QAbstractItemView::DropOnly);
    ui->solverOptionTableView->setDragDropOverwriteMode(true);
    ui->solverOptionTableView->setDefaultDropAction(Qt::CopyAction);

    ui->solverOptionTableView->horizontalHeader()->setStretchLastSection(true);
    ui->solverOptionTableView->setColumnHidden(2, true); //false);

    connect(ui->solverOptionTableView->verticalHeader(), &QHeaderView::sectionClicked, this, &SolverOptionWidget::on_toggleRowHeader);
    connect(ui->solverOptionTableView, &QTableView::customContextMenuRequested,this, &SolverOptionWidget::showOptionContextMenu);
    connect(mOptionTableModel, &SolverOptionTableModel::newTableRowDropped, this, &SolverOptionWidget::on_newTableRowDropped);

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
    connect(ui->solverOptionSearch, &QLineEdit::textChanged,
            proxymodel, static_cast<void(QSortFilterProxyModel::*)(const QString &)>(&QSortFilterProxyModel::setFilterRegExp));

    ui->solverOptionTreeView->setModel( proxymodel );
    ui->solverOptionTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
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
    ui->solverOptionTreeView->setColumnHidden(OptionDefinitionModel::COLUMN_ENTRY_NUMBER, true); // false);
    connect(ui->solverOptionTreeView, &QAbstractItemView::doubleClicked, this, &SolverOptionWidget::addOptionFromDefinition);

    connect(ui->solverOptionGroup, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
         qDebug() << solverName << ":index=" << index << ":" << groupModel->data(groupModel->index(index, 0)) << ", " << groupModel->data(groupModel->index(index, 1));
         optdefmodel->loadOptionFromGroup( groupModel->data(groupModel->index(index, 1)).toInt() );
    });

    connect(ui->solverOptionTableView->model(), &QAbstractTableModel::dataChanged, this, &SolverOptionWidget::on_dataItemChanged);
    connect(mOptionTableModel, &SolverOptionTableModel::solverOptionModelChanged, optdefmodel, &SolverOptionDefinitionModel::modifyOptionDefinition);
    connect(mOptionTableModel, &SolverOptionTableModel::solverOptionValueChanged, mOptionTableModel, &SolverOptionTableModel::on_solverOptionValueChanged);

    ui->solverOptionHSplitter->setSizes(QList<int>({25, 75}));
    ui->solverOptionVSplitter->setSizes(QList<int>({80, 20}));

    setModified(false);

}

SolverOptionWidget::~SolverOptionWidget()
{
    delete ui;
    delete mOptionTokenizer;
}

bool SolverOptionWidget::isInFocused(QWidget *focusWidget)
{
    return (focusWidget==ui->solverOptionTableView || focusWidget==ui->solverOptionTreeView);
}

FileId SolverOptionWidget::fileId() const
{
    return mFileId;
}

void SolverOptionWidget::setFileId(const FileId &fileId)
{
    mFileId = fileId;
}

NodeId SolverOptionWidget::groupId() const
{
    return mGroupId;
}

void SolverOptionWidget::setGroupId(const NodeId &groupId)
{
    mGroupId = groupId;
}

bool SolverOptionWidget::isModified() const
{
    return mModified;
}

void SolverOptionWidget::showOptionContextMenu(const QPoint &pos)
{
    QModelIndexList selection = ui->solverOptionTableView->selectionModel()->selectedRows();

    QMenu menu(this);
    QAction* toggleCommentAction = menu.addAction("comment this option");
    QAction* toggleOptionAction = menu.addAction("Uncomment this line");
    menu.addSeparator();
    QAction* insertOptionAction = menu.addAction(QIcon(":/img/insert"), "insert new option");
    QAction* insertCommentAction = menu.addAction(QIcon(":/img/insert"), "insert new comment");
    menu.addSeparator();
    QAction* moveUpAction = menu.addAction(QIcon(":/img/move-up"), "move selected option up");
    QAction* moveDownAction = menu.addAction(QIcon(":/img/move-down"), "move selected option down");
    menu.addSeparator();
    QAction* deleteAction = menu.addAction(QIcon(":/img/delete"), "remove selected option");
    menu.addSeparator();
    QAction* deleteAllActions = menu.addAction(QIcon(":/img/delete-all"), "remove all options");

    if (ui->solverOptionTableView->model()->rowCount() <= 0) {
        deleteAllActions->setVisible(false);
    }
    if (selection.count() <= 0) {
        toggleCommentAction->setVisible(false);
        toggleOptionAction->setVisible(false);
        insertOptionAction->setVisible(false);
        insertCommentAction->setVisible(false);
        deleteAction->setVisible(false);
        moveUpAction->setVisible(false);
        moveDownAction->setVisible(false);
    } else {
        QModelIndex index = selection.at(0);
        if (index.row()==0)
            moveUpAction->setVisible(false);
        if (index.row()+1 == ui->solverOptionTableView->model()->rowCount())
            moveDownAction->setVisible(false);
        if (mOptionTableModel->getCurrentListOfOptionItems().at(index.row())->disabled)
            toggleCommentAction->setVisible(false);
        else
            toggleOptionAction->setVisible(false);
    }

    QAction* action = menu.exec(ui->solverOptionTableView->viewport()->mapToGlobal(pos));
    if ((action == toggleCommentAction) || (action == toggleOptionAction)) {
        if (selection.count() > 0) {
            on_toggleRowHeader( selection.at(0).row() );
        }
    } else if (action == insertCommentAction) {
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            ui->solverOptionTableView->model()->insertRows(index.row(), 1, QModelIndex());
            ui->solverOptionTableView->model()->setHeaderData(index.row(), Qt::Vertical,
                                                              Qt::CheckState(Qt::PartiallyChecked),
                                                              Qt::CheckStateRole );
            ui->solverOptionTableView->selectRow(index.row());
            updateTableColumnSpan();
            setModified(true);
        }
    } else if (action == insertOptionAction) {
            if (selection.count() > 0) {
                QModelIndex index = selection.at(0);
                ui->solverOptionTableView->model()->insertRows(index.row(), 1, QModelIndex());
                ui->solverOptionTableView->model()->setHeaderData(index.row(), Qt::Vertical,
                                                                  Qt::CheckState(Qt::Checked),
                                                                  Qt::CheckStateRole );
                ui->solverOptionTableView->selectRow(index.row());
                updateTableColumnSpan();
                setModified(true);
            }
    } else if (action == moveUpAction) {
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            ui->solverOptionTableView->model()->moveRows(QModelIndex(), index.row(), 1, QModelIndex(), index.row()-1);
            ui->solverOptionTableView->selectRow(index.row()-1);
            updateTableColumnSpan();
            setModified(true);
        }

    } else if (action == moveDownAction) {
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            ui->solverOptionTableView->model()->moveRows(QModelIndex(), index.row(), 1, QModelIndex(), index.row()+2);
            ui->solverOptionTableView->selectRow(index.row()+1);
            updateTableColumnSpan();
            setModified(true);
        }
    } else if (action == deleteAction) {
        if (selection.count() > 0) {
            setModified(true);
            QModelIndex index = selection.at(0);
            QModelIndex removeTableIndex = ui->solverOptionTableView->model()->index(index.row(), 0);
            QVariant optionName = ui->solverOptionTableView->model()->data(removeTableIndex, Qt::DisplayRole);

            ui->solverOptionTableView->model()->removeRows(index.row(), 1, QModelIndex());

            mOptionTokenizer->getOption()->setModified(optionName.toString(), false);

            QModelIndexList items = ui->solverOptionTreeView->model()->match(ui->solverOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                             Qt::DisplayRole,
                                                                             optionName, 1); //, Qt::MatchRecursive);
            for(QModelIndex item : items) {
                ui->solverOptionTreeView->model()->setData(item, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole);
            }
            updateTableColumnSpan();
            setModified(true);
       }
    } else if (action == deleteAllActions) {
        setModified(true);
        mOptionTokenizer->getOption()->resetModficationFlag();

        QModelIndexList items = ui->solverOptionTreeView->model()->match(ui->solverOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                         Qt::CheckStateRole,
                                                                         Qt::CheckState(Qt::Checked),
                                                                         ui->solverOptionTreeView->model()->rowCount());
        for(QModelIndex item : items) {
            ui->solverOptionTreeView->model()->setData(item, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole);
        }
        ui->solverOptionTableView->model()->removeRows(0, ui->solverOptionTableView->model()->rowCount(), QModelIndex());
        updateTableColumnSpan();
        setModified(true);
    }
}

void SolverOptionWidget::addOptionFromDefinition(const QModelIndex &index)
{
    setModified(true);

    QModelIndex parentIndex =  ui->solverOptionTreeView->model()->parent(index);
    QModelIndex optionNameIndex = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) :
                                                          ui->solverOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ;
    QModelIndex synonymIndex = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_SYNONYM) :
                                                       ui->solverOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_SYNONYM) ;
    QModelIndex defValueIndex = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_DEF_VALUE) :
                                                        ui->solverOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_DEF_VALUE) ;
    QModelIndex selectedValueIndex = (parentIndex.row()<0) ? defValueIndex : index ;

    QString optionNameData = ui->solverOptionTreeView->model()->data(optionNameIndex).toString();
    QString synonymData = ui->solverOptionTreeView->model()->data(synonymIndex).toString();
    QString selectedValueData = ui->solverOptionTreeView->model()->data(selectedValueIndex).toString();

    mOptionTokenizer->getOption()->setModified(optionNameData, true);
    ui->solverOptionTreeView->model()->setData(optionNameIndex, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);

    int optionEntryNumber = mOptionTokenizer->getOption()->getOptionDefinition(optionNameData).number;

    ui->solverOptionTableView->model()->insertRows(ui->solverOptionTableView->model()->rowCount(), 1, QModelIndex());
    QModelIndex insertKeyIndex = ui->solverOptionTableView->model()->index(ui->solverOptionTableView->model()->rowCount()-1, 0);
    QModelIndex insertValueIndex = ui->solverOptionTableView->model()->index(ui->solverOptionTableView->model()->rowCount()-1, 1);
    QModelIndex insertNumberIndex = ui->solverOptionTableView->model()->index(ui->solverOptionTableView->model()->rowCount()-1, 2);
    ui->solverOptionTableView->model()->setData( insertKeyIndex, optionNameData, Qt::EditRole);
    ui->solverOptionTableView->model()->setData( insertValueIndex, selectedValueData, Qt::EditRole);
    ui->solverOptionTableView->model()->setData( insertNumberIndex, optionEntryNumber, Qt::EditRole);
    ui->solverOptionTableView->selectRow(ui->solverOptionTableView->model()->rowCount()-1);
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

void SolverOptionWidget::on_problemSavingOptionFile(const QString &location)
{
    int answer = QMessageBox::question(this, "Problem Saving Option File"
                                       , QString("File %1 has been saved.\nBut there is an errror and not all option and values may not have beeen saved correctly.").arg(location)
                                       , "Load the saved option file", "Continue editing the options");
    if (answer==0)
        on_reloadSolverOptionFile();
    else
        setModified(true);
}

void SolverOptionWidget::on_reloadSolverOptionFile()
{
    mOptionTokenizer->logger()->appendLog(QString("Loading options from %1").arg(mLocation), LogMsgType::Info);
//    OptionTableModel* model = static_cast<OptionTableModel*>(ui->solverOptionTableView->model());
//    model->reloadOptionModel( mOptionTokenizer->readOptionParameterFile(mLocation) );
    mOptionTableModel->reloadSolverOptionModel( mOptionTokenizer->readOptionFile(mLocation) );
    setModified(false);
}

void SolverOptionWidget::on_toggleRowHeader(int logicalIndex)
{
    mOptionTableModel->on_toggleRowHeader(logicalIndex);
    updateTableColumnSpan();
    setModified(true);
}

void SolverOptionWidget::updateEditActions(bool modified)
{
    ui->saveButton->setEnabled(modified);
    ui->saveAsButton->setEnabled(modified);
    ui->openAsTextButton->setEnabled(!modified);
}

void SolverOptionWidget::updateTableColumnSpan()
{
    ui->solverOptionTableView->clearSpans();
    QList<SolverOptionItem *> optionItems = mOptionTableModel->getCurrentListOfOptionItems();
    for(int i=0; i< optionItems.size(); ++i) {
        if (optionItems.at(i)->disabled)
            ui->solverOptionTableView->setSpan(i, 0, 1, 3);
    }
}

QString SolverOptionWidget::getSolverName() const
{
    return mSolverName;
}

void SolverOptionWidget::on_newTableRowDropped(const QModelIndex &index)
{
    QString optionName = ui->solverOptionTableView->model()->data(index, Qt::DisplayRole).toString();
    QModelIndexList definitionItems = ui->solverOptionTreeView->model()->match(ui->solverOptionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     optionName, 1);
    mOptionTokenizer->getOption()->setModified(optionName, true);
    for(QModelIndex item : definitionItems) {
        ui->solverOptionTreeView->model()->setData(item, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);
    }
}

void SolverOptionWidget::setModified(bool modified)
{
    mModified = modified;
    updateEditActions(mModified);
    emit modificationChanged( mFileId );
}

bool SolverOptionWidget::saveAs(const QString &location)
{
    setModified(false);
    bool success = mOptionTokenizer->writeOptionFile(mOptionTableModel->getCurrentListOfOptionItems(), location);
    mOptionTokenizer->logger()->appendLog(QString("Saved options into %1").arg(location), LogMsgType::Info);
    return success;
}

bool SolverOptionWidget::isAnOptionWidgetFocused(QWidget *focusWidget)
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
