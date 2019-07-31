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
#include "modeldialog.h"
#include "ui_modeldialog.h"
#include "commonpaths.h"
#include "glbparser.h"
#include "libraryitem.h"
#include "librarymodel.h"
#include "common.h"

#include <QDirIterator>
#include <QMessageBox>
#include <QTableView>
#include <QHeaderView>
#include <QSortFilterProxyModel>

namespace gams {
namespace studio {

ModelDialog::ModelDialog(QWidget *parent)
    : ModelDialog("", parent)
{

}

ModelDialog::ModelDialog(QString userLibPath, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ModelDialog),
      mUserLibPath(userLibPath)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QDir gamsSysDir(CommonPaths::systemDir());
    QList<LibraryItem> items;

    items = GlbParser::parseFile(gamsSysDir.filePath("gamslib_ml/gamslib.glb"));
    items.at(0).library()->setName("Model Library");
    addLibrary(items);

    items = GlbParser::parseFile(gamsSysDir.filePath("testlib_ml/testlib.glb"));
    items.at(0).library()->setName("Test Library");
    addLibrary(items);

    items = GlbParser::parseFile(gamsSysDir.filePath("apilib_ml/apilib.glb"));
    items.at(0).library()->setName("API Library");
    addLibrary(items);

    items = GlbParser::parseFile(gamsSysDir.filePath("datalib_ml/datalib.glb"));
    items.at(0).library()->setName("Data Utilities Library");
    addLibrary(items);

    items = GlbParser::parseFile(gamsSysDir.filePath("emplib_ml/emplib.glb"));
    items.at(0).library()->setName("EMP Library");
    addLibrary(items);

    items = GlbParser::parseFile(gamsSysDir.filePath("finlib_ml/finlib.glb"));
    items.at(0).library()->setName("FIN Library");
    addLibrary(items);

    items = GlbParser::parseFile(gamsSysDir.filePath("noalib_ml/noalib.glb"));
    items.at(0).library()->setName("NOA Library");
    addLibrary(items);

    items = GlbParser::parseFile(gamsSysDir.filePath("psoptlib_ml/psoptlib.glb"));
    items.at(0).library()->setName("PSO Library");
    addLibrary(items);

    if (!mUserLibPath.isEmpty())
        loadUserLibs();

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &ModelDialog::clearSelections);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &ModelDialog::clearSelections);
}

ModelDialog::~ModelDialog()
{
    delete ui;
}

//TODO(CW): updating the header and displaying the number of models in a library works but this solution is not optimal
void ModelDialog::changeHeader()
{
    for (int i=0; i<ui->tabWidget->count(); i++) {
        QTableView *tv = tableViewList.at(i);
        int rowCount = tv->model()->rowCount();
        QString baseName = ui->tabWidget->tabText(i).split("(").at(0).trimmed();
        ui->tabWidget->setTabText(i, baseName + " (" + QString::number(rowCount) + ")");
    }
}

QTableView* ModelDialog::tableAt(int i)
{
    if (i >= 0 && i < tableViewList.size())
        return tableViewList.at(i);
    else
        return nullptr;
}

void ModelDialog::updateSelectedLibraryItem()
{
    int idx = ui->tabWidget->currentIndex();
    QModelIndexList modelIndexList = tableViewList.at(idx)->selectionModel()->selectedIndexes();
    if (modelIndexList.size()>0) {
        QModelIndex index = modelIndexList.at(0);
        index = static_cast<const QAbstractProxyModel*>(index.model())->mapToSource(index);
        mSelectedLibraryItem = static_cast<LibraryItem*>(index.data(Qt::UserRole).value<void*>());
        ui->pbLoad->setEnabled(true);
        if (mSelectedLibraryItem->longDescription().isEmpty()) //enable button only if a long description is available
            ui->pbDescription->setEnabled(false);
        else
            ui->pbDescription->setEnabled(true);
    }
    else {
        mSelectedLibraryItem = nullptr;
        ui->pbLoad->setEnabled(false);
        ui->pbDescription->setEnabled(false);
    }
}

void ModelDialog::clearSelections()
{
    for (auto tv : tableViewList)
        tv->clearSelection();
}

void ModelDialog::addLibrary(QList<LibraryItem> items, bool isUserLibrary)
{
    QTableView* tableView;
    QSortFilterProxyModel* proxyModel;

    tableView = new QTableView();
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->verticalHeader()->hide();
    tableView->horizontalHeader()->setHighlightSections(false);
    tableView->setAlternatingRowColors(true);
    tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tableView->verticalHeader()->setMinimumSectionSize(1);
    tableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setFilterKeyColumn(-1);
    proxyModel->setSourceModel(new LibraryModel(items, this));
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    tableViewList.append(tableView);
    proxyModelList.append(proxyModel);

    tableView->setModel(proxyModel);
    QString label = items.at(0).library()->name() + " (" +  QString::number(items.size()) + ")";
    int tabIdx=0;
    if (isUserLibrary)
        tabIdx = ui->tabWidget->addTab(tableView, QIcon(mIconUserLib), label);
    else
        tabIdx = ui->tabWidget->addTab(tableView, label);
    ui->tabWidget->setTabToolTip(tabIdx, items.at(0).library()->longName());

    connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ModelDialog::updateSelectedLibraryItem);

    connect(tableView  , &QTableView::doubleClicked, this, &ModelDialog::accept);
    connect(ui->pbLoad  , &QPushButton::clicked     , this, &ModelDialog::accept);
    connect(ui->pbCancel, &QPushButton::clicked     , this, &ModelDialog::reject);

    connect(ui->lineEdit, &QLineEdit::textChanged, proxyModel, &QSortFilterProxyModel::setFilterWildcard);

    connect(proxyModel, &QAbstractProxyModel::rowsRemoved, this, &ModelDialog::changeHeader);
    connect(proxyModel, &QAbstractProxyModel::rowsInserted, this, &ModelDialog::changeHeader);

    tableView->horizontalHeader()->setResizeContentsPrecision(100);
    tableView->resizeColumnsToContents();
    tableView->sortByColumn(0, Qt::AscendingOrder);
    tableView->setSortingEnabled(true);
}

void ModelDialog::loadUserLibs()
{
    QDirIterator iter(mUserLibPath, QDirIterator::Subdirectories);
    while (!iter.next().isEmpty()) {
        if (QFileInfo(iter.filePath()).suffix() == "glb")
            addLibrary(GlbParser::parseFile(iter.filePath()), true);
    }
}

LibraryItem *ModelDialog::selectedLibraryItem() const
{
    return mSelectedLibraryItem;
}

void ModelDialog::on_pbDescription_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Description for '" + mSelectedLibraryItem->name() + "' from " + mSelectedLibraryItem->library()->name());
    msgBox.setText(mSelectedLibraryItem->name());
    msgBox.setInformativeText(mSelectedLibraryItem->longDescription());
    msgBox.exec();
}

void ModelDialog::on_cbRegEx_toggled(bool checked)
{
    if (checked) {
        for (auto proxy : proxyModelList) {
            disconnect(ui->lineEdit, &QLineEdit::textChanged, proxy, &QSortFilterProxyModel::setFilterWildcard);
            connect(ui->lineEdit, SIGNAL(textChanged(const QString &)), proxy, SLOT(setFilterRegExp(const QString &)));
        }
    }
    else {
        for (auto proxy : proxyModelList) {
            disconnect(ui->lineEdit, SIGNAL(textChanged(const QString &)), proxy, SLOT(setFilterRegExp(const QString &)));
            connect(ui->lineEdit, &QLineEdit::textChanged, proxy, &QSortFilterProxyModel::setFilterWildcard);
        }
    }
    emit ui->lineEdit->textChanged(ui->lineEdit->text());
}

}
}
