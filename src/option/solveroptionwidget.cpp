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

    mOptionTokenizer = new OptionTokenizer(QString("opt%1.def").arg(solverName));

    SystemLogEdit* logEdit = new SystemLogEdit(this);
    mOptionTokenizer->provideLogger(logEdit);
    ui->solverOptionTabWidget->addTab( logEdit, "Message" );

    SolverOptionSetting* settingEdit = new SolverOptionSetting(mOptionTokenizer->getOption()->getEOLChars(), this);
    mOptionTokenizer->on_EOLCommentChar_changed( settingEdit->getDefaultEOLCharacter() );
    connect(settingEdit, &SolverOptionSetting::EOLCharChanged, mOptionTokenizer, &OptionTokenizer::on_EOLCommentChar_changed);
    connect(settingEdit, &SolverOptionSetting::separatorCharChanged, mOptionTokenizer, &OptionTokenizer::on_separatorChar_changed);
    ui->solverOptionTabWidget->addTab( settingEdit, "Setting" );

    mOptionTokenizer->logger()->append(QString("Loading options from %1").arg(mLocation), LogMsgType::Info);

    QList<SolverOptionItem *> optionItem = mOptionTokenizer->readOptionFile(mLocation, mCodec);
    mOptionTableModel = new SolverOptionTableModel(optionItem, mOptionTokenizer,  this);
    ui->solverOptionTableView->setModel( mOptionTableModel );
    updateTableColumnSpan();

    ui->solverOptionTableView->setItemDelegate( new OptionCompleterDelegate(mOptionTokenizer, ui->solverOptionTableView));
    ui->solverOptionTableView->setEditTriggers(QAbstractItemView::DoubleClicked
                       | QAbstractItemView::EditKeyPressed
                       | QAbstractItemView::AnyKeyPressed );
    ui->solverOptionTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->solverOptionTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->solverOptionTableView->setAutoScroll(true);
    ui->solverOptionTableView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->solverOptionTableView->viewport()->setAcceptDrops(true);
    ui->solverOptionTableView->setDropIndicatorShown(true);
    ui->solverOptionTableView->setDragDropMode(QAbstractItemView::DropOnly);
    ui->solverOptionTableView->setDragDropOverwriteMode(true);
    ui->solverOptionTableView->setDefaultDropAction(Qt::CopyAction);

    ui->solverOptionTableView->resizeColumnToContents(1);
//    ui->solverOptionTableView->resizeColumnToContents(2);
    ui->solverOptionTableView->horizontalHeader()->setStretchLastSection(true);
//    ui->solverOptionTableView->setColumnHidden(2, true); //false);

    connect(ui->solverOptionTableView->verticalHeader(), &QHeaderView::sectionClicked, this, &SolverOptionWidget::on_toggleRowHeader);
    connect(ui->solverOptionTableView, &QTableView::customContextMenuRequested, this, &SolverOptionWidget::showOptionContextMenu);
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
//    ui->solverOptionTreeView->setColumnHidden(OptionDefinitionModel::COLUMN_ENTRY_NUMBER, true); // false);
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

    connect(settingEdit, &SolverOptionSetting::addOptionDescriptionAsComment, this, &SolverOptionWidget::on_addEOLCommentChanged);
    connect(settingEdit, &SolverOptionSetting::addOptionDescriptionAsComment, mOptionTableModel, &SolverOptionTableModel::on_addEOLCommentCheckBox_stateChanged);

    connect(settingEdit, &SolverOptionSetting::addCommentAboveChanged, this, &SolverOptionWidget::on_addCommentAboveChanged);
    connect(settingEdit, &SolverOptionSetting::addCommentAboveChanged, mOptionTableModel, &SolverOptionTableModel::on_addCommentAbove_stateChanged);
    connect(settingEdit, &SolverOptionSetting::addCommentAboveChanged, optdefmodel, &SolverOptionDefinitionModel::on_addCommentAbove_stateChanged);

    connect(this, &SolverOptionWidget::compactViewChanged, optdefmodel, &SolverOptionDefinitionModel::on_compactViewChanged);

    ui->solverOptionHSplitter->setSizes(QList<int>({25, 75}));
    ui->solverOptionVSplitter->setSizes(QList<int>({80, 20}));

    setModified(false);

    // shortcuts
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F1), this, SLOT(showOptionDefinition()));

}

SolverOptionWidget::~SolverOptionWidget()
{
    delete ui;
    delete mOptionTokenizer;
    delete mOptionTableModel;
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
    QModelIndexList selection = ui->solverOptionTableView->selectionModel()->selectedRows();

    QMenu menu(this);
    QAction* commentAction = menu.addAction("Toggle comment/option selection");
    menu.addSeparator();
    QAction* insertOptionAction = menu.addAction(QIcon(":/img/insert"), "insert new Option");
    QAction* insertCommentAction = menu.addAction(QIcon(":/img/insert"), "insert new Comment");
    menu.addSeparator();
    QAction* moveUpAction = menu.addAction(QIcon(":/img/move-up"), "move selection Up");
    QAction* moveDownAction = menu.addAction(QIcon(":/img/move-down"), "move selection Down");
    menu.addSeparator();
    QAction* deleteAction = menu.addAction(QIcon(":/img/delete-all"), "remove Selection");
    menu.addSeparator();

    QAction* selectAll = menu.addAction("Select All", ui->solverOptionTableView, &QTableView::selectAll);

    bool thereIsASelection = (selection.count() > 0);
    bool thereIsARow = (ui->solverOptionTableView->model()->rowCount() > 0);

    commentAction->setVisible( thereIsASelection && !isViewCompact() );

    insertOptionAction->setVisible( thereIsASelection || !thereIsARow || !isViewCompact() );
    insertCommentAction->setVisible( (thereIsASelection || !thereIsARow) && !isViewCompact() );

    deleteAction->setVisible( thereIsASelection );

    moveUpAction->setVisible(  thereIsASelection && !isViewCompact() && (selection.first().row() > 0) );
    moveDownAction->setVisible( thereIsASelection && !isViewCompact() && (selection.last().row() < mOptionTableModel->rowCount()-1) );

    selectAll->setVisible( thereIsARow );

    if (thereIsASelection) {
        // move up and down actions are disabled when selection are not contiguous
        QList<SolverOptionItem*> items = mOptionTableModel->getCurrentListOfOptionItems();
        QModelIndex index = selection.at(0);
        if (index.row()==0)
            moveUpAction->setVisible(false);
        if (index.row()+1 == ui->solverOptionTableView->model()->rowCount())
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
            moveUpAction->setVisible(false);
            moveDownAction->setVisible(false);
        }

        // toggle comment action is disabled when all checkstates of section are not the same
        index = selection.at(0);
        bool checkState = items.at(index.row())->disabled;
        for (i=1; i<selection.count(); ++i) {
            index = selection.at(i);
            if (checkState != items.at(index.row())->disabled) {
                break;
            }
        }
        if (i != selection.count())
            commentAction->setVisible(false);
    }

    QAction* action = menu.exec(ui->solverOptionTableView->viewport()->mapToGlobal(pos));
    bool modified = false;
    if (action == commentAction) {
        for(int i=0; i<selection.count(); ++i) {
            on_toggleRowHeader( selection.at(i).row() );
            modified = true;
        }
        if (modified) {
            setModified(modified);
        }
    } else if (action == insertCommentAction ) {
        if (thereIsASelection)  {
            QModelIndex index = selection.at(0);
            ui->solverOptionTableView->model()->insertRows(index.row(), 1, QModelIndex());
            ui->solverOptionTableView->model()->setHeaderData(index.row(), Qt::Vertical,
                                                              Qt::CheckState(Qt::PartiallyChecked),
                                                              Qt::CheckStateRole );
            ui->solverOptionTableView->selectRow(index.row());
        } else {
            ui->solverOptionTableView->model()->insertRows(ui->solverOptionTableView->model()->rowCount(), 1, QModelIndex());
            ui->solverOptionTableView->model()->setHeaderData(ui->solverOptionTableView->model()->rowCount()-1, Qt::Vertical,
                                                              Qt::CheckState(Qt::PartiallyChecked),
                                                              Qt::CheckStateRole );
            ui->solverOptionTableView->selectRow(ui->solverOptionTableView->model()->rowCount()-1);
        }
        updateTableColumnSpan();
        modified = true;
        setModified(modified);
        emit itemCountChanged(ui->solverOptionTableView->model()->rowCount());
    } else if (action == insertOptionAction) {
              if (thereIsASelection)  {
                 QModelIndex index = selection.at(0);
                 ui->solverOptionTableView->model()->insertRows(index.row(), 1, QModelIndex());
                 ui->solverOptionTableView->model()->setHeaderData(index.row(), Qt::Vertical,
                                                                  Qt::CheckState(Qt::Unchecked),
                                                                  Qt::CheckStateRole );
                 ui->solverOptionTableView->selectRow(index.row());
              } else {
                  ui->solverOptionTableView->model()->insertRows(ui->solverOptionTableView->model()->rowCount(), 1, QModelIndex());
                  ui->solverOptionTableView->model()->setHeaderData(ui->solverOptionTableView->model()->rowCount()-1, Qt::Vertical,
                                                                    Qt::CheckState(Qt::Unchecked),
                                                                    Qt::CheckStateRole );
                  ui->solverOptionTableView->selectRow(ui->solverOptionTableView->model()->rowCount()-1);
              }
              updateTableColumnSpan();
              modified = true;
              setModified(modified);
              emit itemCountChanged(ui->solverOptionTableView->model()->rowCount());
    } else if (action == moveUpAction) {
        if (thereIsASelection) {
            QModelIndex index = selection.at(0);
            ui->solverOptionTableView->model()->moveRows(QModelIndex(), index.row(), selection.count(),
                                                         QModelIndex(), index.row()-1);
            updateTableColumnSpan();
            modified = true;
        }
    } else if (action == moveDownAction) {
        if (thereIsASelection) {
            QModelIndex index = selection.at(0);
            mOptionTableModel->moveRows(QModelIndex(), index.row(), selection.count(),
                                        QModelIndex(), index.row()+selection.count()+1);
            updateTableColumnSpan();
            modified = true;
        }
    } else if (action == deleteAction) {
         if (thereIsASelection) {
            QModelIndex index = selection.at(0);
            QModelIndex removeTableIndex = mOptionTableModel->index(index.row(), 0);
            QVariant optionName = mOptionTableModel->data(removeTableIndex, Qt::DisplayRole);

            ui->solverOptionTableView->model()->removeRows(index.row(), selection.count(), QModelIndex());
            mOptionTokenizer->getOption()->setModified(optionName.toString(), false);
            updateTableColumnSpan();
            setModified(true);
            emit itemCountChanged(ui->solverOptionTableView->model()->rowCount());
        }
    }
}

void SolverOptionWidget::showDefinitionContextMenu(const QPoint &pos)
{
    QModelIndexList selection = ui->solverOptionTreeView->selectionModel()->selectedRows();
    if (selection.count() <= 0)
        return;

    QMenu menu(this);
    QAction* addThisOptionAction = menu.addAction(QIcon(":/img/plus"), "add This Option");
    menu.addSeparator();
    QAction* copyAction = menu.addAction("Copy Text");
    menu.addSeparator();
    QAction* copyNameAction = menu.addAction("Copy Option Name");
    QAction* copyDescriptionAction = menu.addAction("Copy Description");

    QAction* action = menu.exec(ui->solverOptionTreeView->viewport()->mapToGlobal(pos));
    if (action == copyAction) {
        copyDefinitionToClipboard( -1 );
    } else if (action == copyNameAction) {
        copyDefinitionToClipboard( SolverOptionDefinitionModel::COLUMN_OPTION_NAME );
    } else if (action == copyDescriptionAction) {
        copyDefinitionToClipboard( SolverOptionDefinitionModel::COLUMN_DESCIPTION );
    } else if (action == addThisOptionAction) {
         addOptionFromDefinition(selection.at(0));
    }
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

    if (addCommentAbove) { // insert comment description row
        int indexRow = index.row();
        int parentIndexRow = parentIndex.row();
        QModelIndex descriptionIndex = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index(indexRow, OptionDefinitionModel::COLUMN_DESCIPTION):
                                                               ui->solverOptionTreeView->model()->index(parentIndexRow, OptionDefinitionModel::COLUMN_DESCIPTION);
        QString descriptionData = ui->solverOptionTreeView->model()->data(descriptionIndex).toString();
        ui->solverOptionTableView->model()->insertRows(ui->solverOptionTableView->model()->rowCount(), 1, QModelIndex());

        int row = ui->solverOptionTableView->model()->rowCount()-1;
        QModelIndex insertKeyIndex = ui->solverOptionTableView->model()->index(row, SolverOptionTableModel::COLUMN_OPTION_KEY);
        QModelIndex insertValueIndex = ui->solverOptionTableView->model()->index(row, SolverOptionTableModel::COLUMN_OPTION_VALUE);
        QModelIndex insertNumberIndex = ui->solverOptionTableView->model()->index(row, mOptionTableModel->getColumnEntryNumber());

        ui->solverOptionTableView->model()->setHeaderData( insertKeyIndex.row(), Qt::Vertical,
                                                           Qt::CheckState(Qt::PartiallyChecked),
                                                           Qt::CheckStateRole );
        ui->solverOptionTableView->model()->setData( insertKeyIndex, descriptionData, Qt::EditRole);
        ui->solverOptionTableView->model()->setData( insertValueIndex, "", Qt::EditRole);
        ui->solverOptionTableView->model()->setData( insertNumberIndex, -1, Qt::EditRole);
        if (parentIndex.row() >= 0) {  // insert enum comment description row
            descriptionIndex = ui->solverOptionTreeView->model()->index(indexRow, OptionDefinitionModel::COLUMN_DESCIPTION, parentIndex);
            QString strData =  selectedValueData;
            strData.append( " - " );
            strData.append( ui->solverOptionTreeView->model()->data(descriptionIndex).toString() );

            ui->solverOptionTableView->model()->insertRows(ui->solverOptionTableView->model()->rowCount(), 1, QModelIndex());
            row = ui->solverOptionTableView->model()->rowCount()-1;
            insertKeyIndex = ui->solverOptionTableView->model()->index(row, SolverOptionTableModel::COLUMN_OPTION_KEY);
            insertValueIndex = ui->solverOptionTableView->model()->index(row, SolverOptionTableModel::COLUMN_OPTION_VALUE);
            insertNumberIndex = ui->solverOptionTableView->model()->index(row, mOptionTableModel->getColumnEntryNumber());

            ui->solverOptionTableView->model()->setHeaderData( insertKeyIndex.row(), Qt::Vertical,
                                                               Qt::CheckState(Qt::PartiallyChecked),
                                                               Qt::CheckStateRole );
            ui->solverOptionTableView->model()->setData( insertKeyIndex, strData, Qt::EditRole);
            ui->solverOptionTableView->model()->setData( insertValueIndex, "", Qt::EditRole);
            ui->solverOptionTableView->model()->setData( insertNumberIndex, -1, Qt::EditRole);
        }
    }
    // insert option row
    ui->solverOptionTableView->model()->insertRows(ui->solverOptionTableView->model()->rowCount(), 1, QModelIndex());
    QModelIndex insertKeyIndex = ui->solverOptionTableView->model()->index(ui->solverOptionTableView->model()->rowCount()-1, SolverOptionTableModel::COLUMN_OPTION_KEY);
    QModelIndex insertValueIndex = ui->solverOptionTableView->model()->index(ui->solverOptionTableView->model()->rowCount()-1, SolverOptionTableModel::COLUMN_OPTION_VALUE);
    QModelIndex insertNumberIndex = ui->solverOptionTableView->model()->index(ui->solverOptionTableView->model()->rowCount()-1, mOptionTableModel->getColumnEntryNumber());
    ui->solverOptionTableView->model()->setData( insertKeyIndex, optionNameData, Qt::EditRole);
    ui->solverOptionTableView->model()->setData( insertValueIndex, selectedValueData, Qt::EditRole);
    if (addEOLComment) {
        QModelIndex commentIndex = (parentIndex.row()<0) ? ui->solverOptionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_DESCIPTION):
                                                               ui->solverOptionTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_DESCIPTION);
        QString commentData = ui->solverOptionTreeView->model()->data(commentIndex).toString();
        QModelIndex insertEOLCommentIndex = ui->solverOptionTableView->model()->index(ui->solverOptionTableView->model()->rowCount()-1, SolverOptionTableModel::COLUMN_EOL_COMMENT);
        ui->solverOptionTableView->model()->setData( insertEOLCommentIndex, commentData, Qt::EditRole);
    }
    int optionEntryNumber = mOptionTokenizer->getOption()->getOptionDefinition(optionNameData).number;
    ui->solverOptionTableView->model()->setData( insertNumberIndex, optionEntryNumber, Qt::EditRole);
    ui->solverOptionTableView->selectRow(ui->solverOptionTableView->model()->rowCount()-1);

    int lastColumn = ui->solverOptionTableView->model()->columnCount()-1;
    int lastRow = ui->solverOptionTableView->model()->rowCount()-1;
    int firstRow = (addCommentAbove ? lastRow-2 : lastRow);
    mOptionTableModel->on_updateSolverOptionItem( ui->solverOptionTableView->model()->index(firstRow, lastColumn),
                                                  ui->solverOptionTableView->model()->index(lastRow, lastColumn),
                                                  {Qt::EditRole});

    connect(mOptionTableModel, &QAbstractTableModel::dataChanged, mOptionTableModel, &SolverOptionTableModel::on_updateSolverOptionItem);

    updateTableColumnSpan();

    showOptionDefinition();

    emit itemCountChanged(ui->solverOptionTableView->model()->rowCount());
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
    if (codec != mCodec) {
        mCodec = codec;
        mOptionTokenizer->logger()->append(QString("Loading options from %1").arg(mLocation), LogMsgType::Info);
        mOptionTableModel->reloadSolverOptionModel( mOptionTokenizer->readOptionFile(mLocation, codec) );
        setModified(false);
    }
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
        ui->solverOptionTableView->hideColumn(SolverOptionTableModel::COLUMN_EOL_COMMENT);
        ui->solverOptionTableView->hideColumn(mOptionTableModel->getColumnEntryNumber());
    } else {
        ui->solverOptionTableView->showColumn(SolverOptionTableModel::COLUMN_EOL_COMMENT);
        ui->solverOptionTableView->showColumn(mOptionTableModel->getColumnEntryNumber());
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

void SolverOptionWidget::on_saveButton_clicked(bool checked)
{
    Q_UNUSED(checked);
    MainWindow* main = getMainWindow();
    if (!main) return;

    emit main->saved();
}

void SolverOptionWidget::on_saveAsButton_clicked(bool checked)
{
    Q_UNUSED(checked);
    MainWindow* main = getMainWindow();
    if (!main) return;

    emit main->savedAs();
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

    ui->solverOptionTreeView->selectionModel()->clearSelection();

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

void SolverOptionWidget::updateEditActions(bool modified)
{
    ui->saveButton->setEnabled(modified);
    ui->saveAsButton->setEnabled(true);
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

void SolverOptionWidget::selectSearchField() const
{
    ui->solverOptionSearch->setFocus();
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
    mOptionTokenizer->logger()->append(QString("Saved options into %1").arg(location), LogMsgType::Info);
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
