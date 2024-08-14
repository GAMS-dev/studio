/**
 * GAMS Studio
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
#include "modeldialog.h"
#include "ui_modeldialog.h"
#include "commonpaths.h"
#include "glbparser.h"
#include "libraryitem.h"
#include "librarymodel.h"
#include "common.h"
#include "headerviewproxy.h"
#include "descriptiondialog.h"
#include "tableview.h"

#include <QHeaderView>
#include <QDirIterator>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTableView>
#include <QSortFilterProxyModel>
#include "editors/sysloglocator.h"
#include "editors/abstractsystemlogger.h"
#include "theme.h"

namespace gams {
namespace studio {
namespace modeldialog {

ModelDialog::ModelDialog(QWidget *parent)
    : ModelDialog("", parent)
{

}

ModelDialog::ModelDialog(const QString &userLibPath, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ModelDialog),
      mUserLibPath(userLibPath)
{
    ui->setupUi(this);
    ui->lineEdit->setKeyColumn(2);
    ui->lineEdit->setOptionState(FilterLineEdit::foColumn, 1);

    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    if (!mUserLibPath.isEmpty())
        loadUserLibs();

    QDir gamsSysDir(CommonPaths::systemDir());
    QList<LibraryItem> items;

    GlbParser glbParser;

    QStringList gamsGlbFiles;
    gamsGlbFiles << "gamslib_ml/gamslib.glb"
                 << "testlib_ml/testlib.glb"
                 << "apilib_ml/apilib.glb"
                 << "datalib_ml/datalib.glb"
                 << "emplib_ml/emplib.glb"
                 << "finlib_ml/finlib.glb"
                 << "noalib_ml/noalib.glb"
                 << "psoptlib_ml/psoptlib.glb";

    QStringList gamsLibNames;
    gamsLibNames << "Model Library"
                 << "Test Library"
                 << "API Library"
                 << "Data Utilities Library"
                 << "EMP Library"
                 << "FIN Library"
                 << "NOA Library"
                 << "PSO Library";

    for (int i=0; i<gamsGlbFiles.size(); i++) {
        if (glbParser.parseFile(gamsSysDir.filePath(gamsGlbFiles[i]))) {
            items = glbParser.libraryItems();
            items.at(0).library()->setName(gamsLibNames[i]);
            addLibrary(items);
        } else {
            mHasGlbErrors = true;
            SysLogLocator::systemLog()->append(glbParser.errorMessage(),LogMsgType::Error);
        }
    }

    connect(ui->lineEdit, &FilterLineEdit::regExpChanged, this, &ModelDialog::clearSelections);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &ModelDialog::clearSelections);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &ModelDialog::storeSelectedTab);

    if (mHasGlbErrors) {
        QMessageBox msgBox(this);
        msgBox.setText("Some model libraries could not be loaded due to parsing problems in the corresponding GLB files. See the system output for details.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }

    // bind filter mechanism to textChanged
    connect(ui->lineEdit, &FilterLineEdit::regExpChanged, this, [this](const QRegularExpression &value) {
        for (int i=0; i<proxyModelList.size(); i++)
            applyFilter(value, i);
    });
    connect(ui->lineEdit, &FilterLineEdit::regExpChanged, this, &ModelDialog::jumpToNonEmptyTab);
}

ModelDialog::~ModelDialog()
{
    delete ui;
}

QTableView* ModelDialog::tableAt(int i)
{
    if (i >= 0 && i < tableViewList.size())
        return tableViewList.at(i);
    else
        return nullptr;
}

void ModelDialog::changeHeader(QTableView *view)
{
    auto index = ui->tabWidget->indexOf(view);
    int rowCount = view->model()->rowCount();
    QString baseName = ui->tabWidget->tabText(index).split("(").at(0).trimmed();
    ui->tabWidget->setTabText(index, baseName + " (" + QString::number(rowCount) + ")");
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
    for (auto tv : std::as_const(tableViewList))
        tv->clearSelection();
}

void ModelDialog::storeSelectedTab()
{
    mLastTabIndex = ui->tabWidget->currentIndex();
}

void ModelDialog::addLibrary(const QList<LibraryItem>& items, bool isUserLibrary)
{
    TableView* tableView;
    QSortFilterProxyModel* proxyModel;

    tableView = new TableView();
    if (HeaderViewProxy::platformShouldDrawBorder())
        tableView->horizontalHeader()->setStyle(HeaderViewProxy::instance());
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
    connect(ui->lineEdit, &FilterLineEdit::columnScopeChanged, this, [this, proxyModel]() {
        proxyModel->setFilterKeyColumn(ui->lineEdit->effectiveKeyColumn());
    });

    tableViewList.append(tableView);
    proxyModelList.append(proxyModel);
    connect(proxyModel, &QAbstractItemModel::rowsRemoved,
            this, [this, tableView]{ changeHeader(tableView); });
    connect(proxyModel, &QAbstractItemModel::rowsInserted,
            this, [this, tableView]{ changeHeader(tableView); });

    tableView->setModel(proxyModel);
    QString label = items.at(0).library()->name() + " (" +  QString::number(items.size()) + ")";
    int tabIdx=0;
    if (isUserLibrary)
        tabIdx = ui->tabWidget->addTab(tableView, Theme::icon(mIconUserLib), label);
    else
        tabIdx = ui->tabWidget->addTab(tableView, label);
    ui->tabWidget->setTabToolTip(tabIdx, items.at(0).library()->longName());

    connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ModelDialog::updateSelectedLibraryItem);

    connect(tableView  , &QTableView::doubleClicked, this, &ModelDialog::accept);
    connect(ui->pbLoad  , &QPushButton::clicked     , this, &ModelDialog::accept);
    connect(ui->pbCancel, &QPushButton::clicked     , this, &ModelDialog::reject);

    tableView->horizontalHeader()->setResizeContentsPrecision(100);
    tableView->resizeColumnsToContents();
    tableView->sortByColumn(items.at(0).library()->initSortCol(), Qt::AscendingOrder);
    tableView->setSortingEnabled(true);
}

void ModelDialog::loadUserLibs()
{
    GlbParser glbParser;
    QDirIterator iter(mUserLibPath, QDirIterator::Subdirectories);
    while (!iter.next().isEmpty()) {
        if (QFileInfo(iter.filePath()).suffix() == "glb") {
            QList<LibraryItem> items;
            if (glbParser.parseFile(iter.filePath())) {
                items = glbParser.libraryItems();
                addLibrary(items, true);
            } else {
                mHasGlbErrors = true;
                SysLogLocator::systemLog()->append(glbParser.errorMessage(),LogMsgType::Error);
            }
        }
    }
}

LibraryItem *ModelDialog::selectedLibraryItem() const
{
    return mSelectedLibraryItem;
}

void ModelDialog::on_pbDescription_clicked()
{
    DescriptionDialog dialog(this);
    dialog.setWindowTitle("Description for '" + mSelectedLibraryItem->name() +
                          "' from " + mSelectedLibraryItem->library()->name());
    dialog.setText(mSelectedLibraryItem->name() + "\n\n" + mSelectedLibraryItem->longDescription());
    dialog.exec();
}

void ModelDialog::applyFilter(const QRegularExpression &filterRex, int proxyModelIndex)
{
    if (filterRex.isValid()) {
        proxyModelList[proxyModelIndex]->setFilterRegularExpression(filterRex);
    }
}

void ModelDialog::jumpToNonEmptyTab()
{
    if (mLastTabIndex != ui->tabWidget->currentIndex() && proxyModelList[mLastTabIndex]->rowCount() > 0) { //jump back to last manual selected tab if it becomes non-empty
        disconnect(ui->tabWidget, &QTabWidget::currentChanged, this, &ModelDialog::storeSelectedTab);
        ui->tabWidget->setCurrentIndex(mLastTabIndex);
        connect(ui->tabWidget, &QTabWidget::currentChanged, this, &ModelDialog::storeSelectedTab);
    } else if (proxyModelList[ui->tabWidget->currentIndex()]->rowCount() == 0){ // jump to the first non-mepty tab in case the current tab runs out of results
        for (int i=0; i<proxyModelList.size(); i++) {
            if (proxyModelList[i]->rowCount() > 0) {
                disconnect(ui->tabWidget, &QTabWidget::currentChanged, this, &ModelDialog::storeSelectedTab);
                ui->tabWidget->setCurrentIndex(i);
                connect(ui->tabWidget, &QTabWidget::currentChanged, this, &ModelDialog::storeSelectedTab);
                break;
            }
        }
    }
}

} // namespace modeldialog
} // namespace studio
} // namespace gams
