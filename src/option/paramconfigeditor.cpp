/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "common.h"
#include "definitionitemdelegate.h"
#include "gamsoptiondefinitionmodel.h"
#include "optionsortfilterproxymodel.h"
#include "paramconfigeditor.h"
#include "ui_paramconfigeditor.h"

namespace gams {
namespace studio {
namespace option {

ParamConfigEditor::ParamConfigEditor(QWidget *parent):
    QWidget(parent),
    ui(new Ui::ParamConfigEditor)
{
    ui->setupUi(this);

    mOptionTokenizer = new OptionTokenizer(GamsOptDefFile);
    setFocusPolicy(Qt::StrongFocus);

    QList<ParamConfigItem *> optionItem;
// TODO (JP): initialize from gamsconfig.yaml
//    QString normalizedText = mOptionTokenizer->normalize(optionItem);
    mParameterTableModel = new GamsConfigParamTableModel(optionItem, mOptionTokenizer, this);
    ui->ParamCfgTableView->setModel( mParameterTableModel );

    mOptionCompleter = new OptionCompleterDelegate(mOptionTokenizer, ui->ParamCfgTableView);
    ui->ParamCfgTableView->setItemDelegate( mOptionCompleter );
    connect(mOptionCompleter, &QStyledItemDelegate::commitData, this, &ParamConfigEditor::parameterItemCommitted);
    ui->ParamCfgTableView->setEditTriggers(QAbstractItemView::DoubleClicked
                       | QAbstractItemView::SelectedClicked
                       | QAbstractItemView::EditKeyPressed
                       | QAbstractItemView::AnyKeyPressed );
    ui->ParamCfgTableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->ParamCfgTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->ParamCfgTableView->setAutoScroll(true);
    ui->ParamCfgTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->ParamCfgTableView->setSortingEnabled(false);

    ui->ParamCfgTableView->setDragEnabled(true);
    ui->ParamCfgTableView->viewport()->setAcceptDrops(true);
    ui->ParamCfgTableView->setDropIndicatorShown(true);
    ui->ParamCfgTableView->setDragDropMode(QAbstractItemView::DropOnly);
    ui->ParamCfgTableView->setDragDropOverwriteMode(true);
    ui->ParamCfgTableView->setDefaultDropAction(Qt::CopyAction);

    ui->ParamCfgTableView->setColumnHidden(GamsConfigParamTableModel::COLUMN_ENTRY_NUMBER, false); // TODO (JP) true);
    ui->ParamCfgTableView->verticalHeader()->setMinimumSectionSize(1);
    ui->ParamCfgTableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));
    ui->ParamCfgTableView->horizontalHeader()->setStretchLastSection(true);
    ui->ParamCfgTableView->verticalHeader()->setMinimumSectionSize(1);
    ui->ParamCfgTableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));

    QSortFilterProxyModel* proxymodel = new OptionSortFilterProxyModel(this);
    GamsOptionDefinitionModel* optdefmodel =  new GamsOptionDefinitionModel(mOptionTokenizer->getOption(), 0, this);
    proxymodel->setFilterKeyColumn(-1);
    proxymodel->setSourceModel( optdefmodel );
    proxymodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxymodel->setSortCaseSensitivity(Qt::CaseInsensitive);
    connect(ui->ParamCfgDefSearch, &QLineEdit::textChanged,
            proxymodel, static_cast<void(QSortFilterProxyModel::*)(const QString &)>(&QSortFilterProxyModel::setFilterRegExp));

    ui->ParamCfgDefTreeView->setModel( proxymodel );
    ui->ParamCfgDefTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->ParamCfgDefTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->ParamCfgDefTreeView->setDragEnabled(true);
    ui->ParamCfgDefTreeView->setDragDropMode(QAbstractItemView::DragOnly);

    ui->ParamCfgDefTreeView->setItemDelegate( new DefinitionItemDelegate(ui->ParamCfgDefTreeView) );
    ui->ParamCfgDefTreeView->setItemsExpandable(true);
    ui->ParamCfgDefTreeView->setSortingEnabled(true);
    ui->ParamCfgDefTreeView->sortByColumn(0, Qt::AscendingOrder);
    ui->ParamCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_OPTION_NAME);
    ui->ParamCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_SYNONYM);
    ui->ParamCfgDefTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_DEF_VALUE);
    ui->ParamCfgDefTreeView->setExpandsOnDoubleClick(false);
    ui->ParamCfgDefTreeView->setColumnHidden(OptionDefinitionModel::COLUMN_ENTRY_NUMBER, true);
    ui->ParamCfgDefTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
}

ParamConfigEditor::~ParamConfigEditor()
{
    delete ui;
    if (mOptionTokenizer)
        delete mOptionTokenizer;
    if (mParameterTableModel)
        delete mParameterTableModel;
    if (mOptionCompleter)
        delete mOptionCompleter;
}

void ParamConfigEditor::parameterItemCommitted(QWidget *editor)
{
    Q_UNUSED(editor)
    if (mOptionCompleter->currentEditedIndex().isValid()) {
        ui->ParamCfgTableView->selectionModel()->select( mOptionCompleter->currentEditedIndex(), QItemSelectionModel::ClearAndSelect );
        ui->ParamCfgTableView->setCurrentIndex( mOptionCompleter->currentEditedIndex() );
        ui->ParamCfgTableView->setFocus();
    }
}


}
}
}
