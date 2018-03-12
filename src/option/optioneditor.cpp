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
#include "optioneditor.h"
#include "optioncompleterdelegate.h"
#include "optiondefinitionmodel.h"
#include "optionparametermodel.h"
#include "addoptionheaderview.h"

namespace gams {
namespace studio {

OptionEditor::OptionEditor(CommandLineOption* option, CommandLineTokenizer* tokenizer, QWidget *parent) :
    QWidget(parent), mCommandLineOption(option), mTokenizer(tokenizer)
{
    setupUi(this);
}

OptionEditor::~OptionEditor()
{

}

void OptionEditor::setupUi(QWidget* optionEditor)
{
    QList<OptionItem> optionItem = mTokenizer->tokenize(mCommandLineOption->lineEdit()->text());
    QString normalizedText = mTokenizer->normalize(optionItem);
    optionParamModel = new OptionParameterModel(normalizedText, mTokenizer,  this);

    if (optionEditor->objectName().isEmpty())
        optionEditor->setObjectName(QStringLiteral("OptionEditor"));
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(optionEditor->sizePolicy().hasHeightForWidth());
    optionEditor->setSizePolicy(sizePolicy);

    verticalLayout = new QVBoxLayout(optionEditor);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));

    splitter = new QSplitter(optionEditor);
    splitter->setObjectName(QStringLiteral("splitter"));
    sizePolicy.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
    splitter->setSizePolicy(sizePolicy);
    splitter->setOrientation(Qt::Horizontal);
    commandLineTableView = new QTableView(splitter);
    commandLineTableView->setObjectName(QStringLiteral("commandLineTableView"));
    commandLineTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    commandLineTableView->setItemDelegate( new OptionCompleterDelegate(mTokenizer, commandLineTableView));
    commandLineTableView->setEditTriggers(QAbstractItemView::DoubleClicked
                       | QAbstractItemView::EditKeyPressed
                       | QAbstractItemView::AnyKeyPressed );
    commandLineTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    commandLineTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    commandLineTableView->setAutoScroll(true);
    commandLineTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    commandLineTableView->setModel( optionParamModel );
    commandLineTableView->horizontalHeader()->setStretchLastSection(true);

    AddOptionHeaderView* headerView = new AddOptionHeaderView(Qt::Horizontal, commandLineTableView);
    headerView->setSectionResizeMode(QHeaderView::Stretch);
    commandLineTableView->setHorizontalHeader(headerView);

    splitter->addWidget(commandLineTableView);
    splitter->setSizes(QList<int>({INT_MAX, INT_MAX}));

    verticalLayoutWidget = new QWidget(splitter);
    verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
    optionDefinition_VLayout = new QVBoxLayout(verticalLayoutWidget);
    optionDefinition_VLayout->setObjectName(QStringLiteral("optionDefinition_VLayout"));
    optionDefinition_VLayout->setContentsMargins(0, 0, 0, 0);
    searchLineEdit = new QLineEdit(verticalLayoutWidget);
    searchLineEdit->setObjectName(QStringLiteral("searchLineEdit"));
    searchLineEdit->setPlaceholderText("Search Option...");

    optionDefinition_VLayout->addWidget(searchLineEdit);

    QSortFilterProxyModel* proxymodel = new OptionSortFilterProxyModel(this);
    OptionDefinitionModel* optdefmodel =  new OptionDefinitionModel(mTokenizer->getGamsOption(), this);
    proxymodel->setFilterKeyColumn(-1);
    proxymodel->setSourceModel( optdefmodel );
    proxymodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxymodel->setSortCaseSensitivity(Qt::CaseInsensitive);

    optionDefinitionTreeView = new QTreeView(verticalLayoutWidget);
    optionDefinitionTreeView->setObjectName(QStringLiteral("optionDefinitionTreeView"));
    optionDefinitionTreeView->setItemsExpandable(true);
    optionDefinitionTreeView->setSortingEnabled(true);
    optionDefinitionTreeView->sortByColumn(0, Qt::AscendingOrder);
    optionDefinitionTreeView->setModel( proxymodel );
    optionDefinitionTreeView->resizeColumnToContents(0);
    optionDefinitionTreeView->resizeColumnToContents(2);
    optionDefinitionTreeView->resizeColumnToContents(3);
    optionDefinitionTreeView->setAlternatingRowColors(true);
    optionDefinitionTreeView->setExpandsOnDoubleClick(false);

    optionDefinition_VLayout->addWidget(optionDefinitionTreeView);

    splitter->addWidget(verticalLayoutWidget);

    verticalLayout->addWidget(splitter);

    // retranslateUi
    optionEditor->setWindowTitle(QApplication::translate("optionEditor", "Editor", nullptr));
    searchLineEdit->setText(QString());

    updateCommandLineStr( normalizedText );

    connect(searchLineEdit, &QLineEdit::textChanged,
            proxymodel, static_cast<void(QSortFilterProxyModel::*)(const QString &)>(&QSortFilterProxyModel::setFilterRegExp));
//    connect(commandLineTableView->verticalHeader(), &QHeaderView::sectionClicked,
//            optionParamModel, &OptionParameterModel::toggleActiveOptionItem);
    connect(commandLineTableView, &QTableView::customContextMenuRequested,this, &OptionEditor::showOptionContextMenu);

    connect(this, &OptionEditor::optionTableModelChanged, optionParamModel, &OptionParameterModel::updateCurrentOption);

    connect(optionParamModel, &OptionParameterModel::optionModelChanged,
            this, static_cast<void(OptionEditor::*)(const QList<OptionItem> &)> (&OptionEditor::updateCommandLineStr));
    connect(this, static_cast<void(OptionEditor::*)(QLineEdit*, const QList<OptionItem> &)>(&OptionEditor::commandLineOptionChanged),
            mTokenizer, &CommandLineTokenizer::formatItemLineEdit);

    connect(optionDefinitionTreeView, &QAbstractItemView::doubleClicked, this, &OptionEditor::addOptionFromDefinition);

}

QList<OptionItem> OptionEditor::getCurrentListOfOptionItems()
{
    return optionParamModel->getCurrentListOfOptionItems();
}

void OptionEditor::updateTableModel(QLineEdit* lineEdit, const QString &commandLineStr)
{
    Q_UNUSED(lineEdit);
    emit optionTableModelChanged(commandLineStr);
}

void OptionEditor::updateCommandLineStr(const QString &commandLineStr)
{
    if (isHidden())
       return;

    mCommandLineOption->lineEdit()->setText( commandLineStr );
    emit commandLineOptionChanged(mCommandLineOption->lineEdit(), commandLineStr);
}

void OptionEditor::updateCommandLineStr(const QList<OptionItem> &opionItems)
{
    if (isHidden())
       return;

    emit commandLineOptionChanged(mCommandLineOption->lineEdit(), opionItems);
}

void OptionEditor::showOptionContextMenu(const QPoint &pos)
{
    QModelIndexList selection = commandLineTableView->selectionModel()->selectedRows();

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

    if (commandLineTableView->model()->rowCount() <= 0) {
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
        if (index.row()+1 == commandLineTableView->model()->rowCount())
            moveDownAction->setVisible(false);
    }

    QAction* action = menu.exec(commandLineTableView->viewport()->mapToGlobal(pos));
    if (action == addAction) {
         commandLineTableView->model()->insertRows(commandLineTableView->model()->rowCount(), 1, QModelIndex());
         commandLineTableView->selectRow(commandLineTableView->model()->rowCount()-1);
    } else if (action == insertAction) {
            if (selection.count() > 0) {
                QModelIndex index = selection.at(0);
                commandLineTableView->model()->insertRows(index.row(), 1, QModelIndex());
                commandLineTableView->selectRow(index.row());
            }
    } else if (action == moveUpAction) {
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            commandLineTableView->model()->moveRows(QModelIndex(), index.row(), 1, QModelIndex(), index.row()-1);
            commandLineTableView->selectRow(index.row()-1);
        }

    } else if (action == moveDownAction) {
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            commandLineTableView->model()->moveRows(QModelIndex(), index.row(), 1, QModelIndex(), index.row()+2);
            commandLineTableView->selectRow(index.row()+1);
        }
    } else if (action == deleteAction) {
             if (selection.count() > 0) {
                 QModelIndex index = selection.at(0);
                 commandLineTableView->model()->removeRow(index.row(), QModelIndex());
             }
    } else if (action == deleteAllActions) {
        emit optionTableModelChanged("");
    }
}

void OptionEditor::addOptionFromDefinition(const QModelIndex &index)
{
    QVariant data = optionDefinitionTreeView->model()->data(index);
    QModelIndex defValueIndex = optionDefinitionTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_DEF_VALUE);
    QModelIndex  parentIndex =  optionDefinitionTreeView->model()->parent(index);
//    qDebug() <<  "parentIndex=" << parentIndex.row();
    // TODO insert before selected  or at the end when no selection
    commandLineTableView->model()->insertRows(commandLineTableView->model()->rowCount(), 1, QModelIndex());
    QModelIndex insertKeyIndex = commandLineTableView->model()->index(commandLineTableView->model()->rowCount()-1, 0);
    QModelIndex insertValueIndex = commandLineTableView->model()->index(commandLineTableView->model()->rowCount()-1, 1);
    if (parentIndex.row() < 0) {
        commandLineTableView->model()->setData( insertKeyIndex, data.toString(), Qt::EditRole);
        commandLineTableView->model()->setData( insertValueIndex, optionDefinitionTreeView->model()->data(defValueIndex).toString(), Qt::EditRole);
    } else {
        QVariant parentData = optionDefinitionTreeView->model()->data( parentIndex );
        commandLineTableView->model()->setData( insertKeyIndex, parentData.toString(), Qt::EditRole);
        commandLineTableView->model()->setData( insertValueIndex, data.toString(), Qt::EditRole);
    }
    commandLineTableView->selectRow(commandLineTableView->model()->rowCount()-1);
}

} // namespace studio
} // namespace gams
