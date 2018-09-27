#include <QStandardItemModel>

#include "solveroptionwidget.h"
#include "ui_solveroptionwidget.h"

#include "addoptionheaderview.h"
#include "optioncompleterdelegate.h"
#include "optiondefinitionmodel.h"
#include "optionsortfilterproxymodel.h"
#include "optiontablemodel.h"
#include "mainwindow.h"

namespace gams {
namespace studio {
namespace option {

SolverOptionWidget::SolverOptionWidget(QString solverName, QString optionFilePath, QWidget *parent) :
          QWidget(parent),
          ui(new Ui::SolverOptionWidget),
          mSolverName(solverName)
{
    ui->setupUi(this);
    mOptionTokenizer = new OptionTokenizer(QString("opt%1.def").arg(solverName));

    QList<OptionItem> optionItem = mOptionTokenizer->readOptionParameterFile( optionFilePath );
    QString normalizedText = mOptionTokenizer->normalize(optionItem);
    qDebug() << "[" << normalizedText << "]";
    OptionTableModel* optionTableModel = new OptionTableModel(normalizedText, mOptionTokenizer,  this);
    ui->solverOptionTableView->setModel( optionTableModel );

    ui->solverOptionTableView->setItemDelegate( new OptionCompleterDelegate(mOptionTokenizer, ui->solverOptionTableView));
    ui->solverOptionTableView->setEditTriggers(QAbstractItemView::DoubleClicked
                       | QAbstractItemView::EditKeyPressed
                       | QAbstractItemView::AnyKeyPressed );
    ui->solverOptionTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->solverOptionTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->solverOptionTableView->setAutoScroll(true);
    ui->solverOptionTableView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->solverOptionTableView->setDragEnabled(true);
    ui->solverOptionTableView->viewport()->setAcceptDrops(true);
    ui->solverOptionTableView->setDropIndicatorShown(true);
    ui->solverOptionTableView->setDragDropMode(QAbstractItemView::DragDrop);
    ui->solverOptionTableView->setDragDropOverwriteMode(true);

    AddOptionHeaderView* headerView = new AddOptionHeaderView(Qt::Horizontal, ui->solverOptionTableView);
    headerView->setSectionResizeMode(QHeaderView::Stretch);
    ui->solverOptionTableView->setHorizontalHeader(headerView);
    ui->solverOptionTableView->horizontalHeader()->setStretchLastSection(true);
    connect(ui->solverOptionTableView, &QTableView::customContextMenuRequested,this, &SolverOptionWidget::showOptionContextMenu);
    connect(this, &SolverOptionWidget::optionTableModelChanged, optionTableModel, &OptionTableModel::on_optionTableModelChanged);

    QList<OptionGroup> optionGroupList = mOptionTokenizer->getOption()->getOptionGroupList();
    QMap<int, QVariant> groupRoles;
    groupRoles.insert( Qt::DisplayRole, QVariant(0));
    groupRoles.insert( Qt::StatusTipRole, QVariant("--- All Options ---") );
    for(OptionGroup group : optionGroupList) {
        groupRoles.insert( Qt::DisplayRole, QVariant(group.number));
        groupRoles.insert( Qt::StatusTipRole, QVariant(group.description) );
    }

    QStandardItemModel* groupModel = new QStandardItemModel(optionGroupList.size()+1, 3);
    int i = 0;
    groupModel->setItem(0, 0, new QStandardItem("--- All Options ---"));
    groupModel->setItem(0, 1, new QStandardItem("0"));
    groupModel->setItem(0, 2, new QStandardItem("All Options"));
    for(OptionGroup group : optionGroupList) {
        ++i;
        groupModel->setItem(i, 0, new QStandardItem(group.description));
        groupModel->setItem(i, 1, new QStandardItem(QString::number(group.number)));
        groupModel->setItem(i, 2, new QStandardItem(group.name));
    }
    ui->solverOptionGroup->setModel(groupModel);
    ui->solverOptionGroup->setModelColumn(0);

    QSortFilterProxyModel* proxymodel = new OptionSortFilterProxyModel(this);
    OptionDefinitionModel* optdefmodel =  new OptionDefinitionModel(mOptionTokenizer->getOption(), 0, this);
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
    connect(ui->solverOptionTreeView, &QAbstractItemView::doubleClicked, this, &SolverOptionWidget::addOptionFromDefinition);

    connect(ui->solverOptionGroup, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
         qDebug() << solverName << ":index=" << index << ":" << groupModel->data(groupModel->index(index, 0)) << ", " << groupModel->data(groupModel->index(index, 1));
         optdefmodel->loadOptionFromGroup( groupModel->data(groupModel->index(index, 1)).toInt() );
    });
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

void SolverOptionWidget::showOptionContextMenu(const QPoint &pos)
{
    QModelIndexList selection = ui->solverOptionTableView->selectionModel()->selectedRows();

    QMenu menu(this);
    QAction* addAction = menu.addAction(QIcon(":/img/plus"), "add new option");
    QAction* insertAction = menu.addAction(QIcon(":/img/insert"), "insert new option");
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
        insertAction->setVisible(false);
        deleteAction->setVisible(false);
        moveUpAction->setVisible(false);
        moveDownAction->setVisible(false);
    } else {
        QModelIndex index = selection.at(0);
        if (index.row()==0)
            moveUpAction->setVisible(false);
        if (index.row()+1 == ui->solverOptionTableView->model()->rowCount())
            moveDownAction->setVisible(false);
    }

    QAction* action = menu.exec(ui->solverOptionTableView->viewport()->mapToGlobal(pos));
    if (action == addAction) {
       ui->solverOptionTableView->model()->insertRows(ui->solverOptionTableView->model()->rowCount(), 1, QModelIndex());
       ui->solverOptionTableView->selectRow(ui->solverOptionTableView->model()->rowCount()-1);
    } else if (action == insertAction) {
            if (selection.count() > 0) {
                QModelIndex index = selection.at(0);
                ui->solverOptionTableView->model()->insertRows(index.row(), 1, QModelIndex());
                ui->solverOptionTableView->selectRow(index.row());
            }
    } else if (action == moveUpAction) {
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            ui->solverOptionTableView->model()->moveRows(QModelIndex(), index.row(), 1, QModelIndex(), index.row()-1);
            ui->solverOptionTableView->selectRow(index.row()-1);
        }

    } else if (action == moveDownAction) {
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            ui->solverOptionTableView->model()->moveRows(QModelIndex(), index.row(), 1, QModelIndex(), index.row()+2);
            ui->solverOptionTableView->selectRow(index.row()+1);
        }
    } else if (action == deleteAction) {
             if (selection.count() > 0) {
                 QModelIndex index = selection.at(0);
                 ui->solverOptionTableView->model()->removeRow(index.row(), QModelIndex());
             }
    } else if (action == deleteAllActions) {
        emit optionTableModelChanged("");
    }
}

void SolverOptionWidget::addOptionFromDefinition(const QModelIndex &index)
{
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

    int i;
    for(i=0; i < ui->solverOptionTableView->model()->rowCount(); ++i) {
        QModelIndex idx = ui->solverOptionTableView->model()->index(i, 0, QModelIndex());
        QString optionName = ui->solverOptionTableView->model()->data(idx, Qt::DisplayRole).toString();
        if ((QString::compare(optionNameData, optionName, Qt::CaseInsensitive)==0) ||
            (QString::compare(synonymData, optionName, Qt::CaseInsensitive)==0))
            break;
    }
    if (i < ui->solverOptionTableView->model()->rowCount()) {
        ui->solverOptionTableView->model()->setData( ui->solverOptionTableView->model()->index(i, 1), selectedValueData, Qt::EditRole);
        ui->solverOptionTableView->selectRow(i);
        return;
    }

    ui->solverOptionTableView->model()->insertRows(ui->solverOptionTableView->model()->rowCount(), 1, QModelIndex());
    QModelIndex insertKeyIndex = ui->solverOptionTableView->model()->index(ui->solverOptionTableView->model()->rowCount()-1, 0);
    QModelIndex insertValueIndex = ui->solverOptionTableView->model()->index(ui->solverOptionTableView->model()->rowCount()-1, 1);
    ui->solverOptionTableView->model()->setData( insertKeyIndex, optionNameData, Qt::EditRole);
    ui->solverOptionTableView->model()->setData( insertValueIndex, selectedValueData, Qt::EditRole);
    ui->solverOptionTableView->selectRow(ui->solverOptionTableView->model()->rowCount()-1);
}

}
}
}
