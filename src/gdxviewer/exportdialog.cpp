/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "exportdialog.h"
#include "gdxsymbol.h"
#include "gdxsymboltablemodel.h"
#include "gdxsymbolview.h"
#include "gdxviewer.h"
#include "ui_exportdialog.h"
#include "valuefilter.h"

#include <headerviewproxy.h>
#include <settings.h>

#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>

#include <numerics/doubleformatter.h>

namespace gams {
namespace studio {
namespace gdxviewer {

ExportDialog::ExportDialog(GdxViewer *gdxViewer, QWidget *parent) :
    QDialog(parent),
    mGdxViewer(gdxViewer),
    ui(new Ui::ExportDialog)
{
    ui->setupUi(this);
    mSymbolTableModel = mGdxViewer->gdxSymbolTable();
    mExportModel = new ExportModel(mSymbolTableModel, this);
    mExportDriver = new ExportDriver(mGdxViewer, mExportModel, this);
    connect(mExportDriver, &ExportDriver::exportDone, this, [this]() { setControlsEnabled(true); accept(); });
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QMenu* m = new QMenu();
    mSaveAction = m->addAction("Save", this, [this]() { ui->toolButton->setDefaultAction(mSaveAction), ExportDialog::save(false); setControlsEnabled(true); });
    mExportAction = m->addAction("Export", this, [this]() { ui->toolButton->setDefaultAction(mExportAction); ExportDialog::saveAndExecute(); });
    ui->toolButton->setMenu(m);
    ui->toolButton->setDefaultAction(mExportAction);
    ui->toolButton->setToolTip("<html><head/><body><p><span style=' font-weight:600;'>Export</span> data and save GAMS Connect file</p><p><span style=' font-weight:600;'>Save</span> GAMS Connect file only</p></body></html>");

    mGdxFile = gdxViewer->gdxFile();
    mRecentPath = QFileInfo(mGdxFile).path();
    QString connectFile = mRecentPath + "/" + QFileInfo(mGdxFile).completeBaseName() + "_export.yaml";
    ui->leConnect->setText(QDir::toNativeSeparators(connectFile));
    QString excelFile = mRecentPath + "/" + QFileInfo(mGdxFile).completeBaseName() + "_export.xlsx";
    ui->leExcel->setText(QDir::toNativeSeparators(excelFile));
    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->tableView->horizontalHeader()->setStyle(HeaderViewProxy::instance());

    mProxyModel = new QSortFilterProxyModel(this);
    mProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    mProxyModel->setSortRole(Qt::UserRole);
    mProxyModel->setSourceModel(mExportModel);

    ui->tableView->setModel(mProxyModel);
    ui->tableView->hideRow(0); // hide universe symbol
    ui->tableView->setColumnHidden(6,true); // hide the "Loaded" column
    ui->tableView->resizeColumnsToContents();

    ui->tableView->sortByColumn(2,Qt::AscendingOrder);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableView->verticalHeader()->setMinimumSectionSize(1);
    ui->tableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));
}

ExportDialog::~ExportDialog()
{
    mExportDriver->cancelProcess(50);
    mProxyModel->setSourceModel(nullptr);
    delete mProxyModel;
    mProxyModel = nullptr;
    delete mExportDriver;
    mExportDriver = nullptr;
    delete mExportModel;
    mExportModel = nullptr;
    delete ui;
}

void ExportDialog::on_pbCancel_clicked()
{
    mExportDriver->cancelProcess();
    reject();
}

void ExportDialog::on_pbBrowseExcel_clicked()
{
    QString filter("Excel file (*.xlsx)");
    QString filePath = QFileDialog::getSaveFileName(this, "Choose Excel File...",
                                                            mRecentPath,
                                                            filter,
                                                            nullptr,
                                                            QFileDialog::DontConfirmOverwrite);
    if (!filePath.isEmpty()) {
        mRecentPath = QFileInfo(filePath).path();
        ui->leExcel->setText(QDir::toNativeSeparators(filePath));
    }
}

void ExportDialog::on_pbBrowseConnect_clicked()
{
    QString filter("Connect instructions (*.yaml)");
    QString filePath = QFileDialog::getSaveFileName(this, "Choose Connect File...",
                                                            mRecentPath,
                                                            filter,
                                                            nullptr,
                                                            QFileDialog::DontConfirmOverwrite);
    if (!filePath.isEmpty()) {
        mRecentPath = QFileInfo(filePath).path();
        ui->leConnect->setText(QDir::toNativeSeparators(filePath));
    }
}

bool ExportDialog::save(bool fileExistsWarning)
{
    setControlsEnabled(false);
    QString connectFile = ui->leConnect->text().trimmed();
    QString output = ui->leExcel->text().trimmed();
    if (output.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("GDX Export");
        msgBox.setText("Excel file can not be empty");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return false;
    }
    if (QFileInfo(output).isRelative())
        output = QDir::toNativeSeparators(mRecentPath + QDir::separator() + output);
    if (QFileInfo(output).suffix().isEmpty())
        output = output + ".xlsx";
    if (QFileInfo(output).suffix() != "xlsx") {
        QMessageBox msgBox;
        msgBox.setWindowTitle("GDX Export");
        msgBox.setText("File extension of Excel file needs to be 'xlsx' but is '" + QFileInfo(output).suffix() + "'");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return false;
    }
    if (mExportModel->selectedSymbols().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("GDX Export");
        msgBox.setText("At least one symbol has to be selected for export");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return false;
    }
    if (fileExistsWarning && QFileInfo::exists(output)) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Overwrite Existing File");
        msgBox.setText(QFileInfo(output).filePath() + " already exists.\nDo you want to overwrite it?");
        msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        msgBox.setIcon(QMessageBox::Warning);
        if (msgBox.exec() == QMessageBox::No)
            return false;
        else
            QFile(output).remove();
    }
    mRecentPath = QFileInfo(output).path();
    ui->leExcel->setText(output);
    bool rc =  mExportDriver->save(connectFile, ui->leExcel->text().trimmed(), ui->cbFilter->isChecked(), ui->leEps->text(), ui->lePosInf->text(), ui->leNegInf->text(), ui->leUndef->text(), ui->leNA->text());
    return rc;
}

void ExportDialog::saveAndExecute()
{
    setControlsEnabled(false);
    QString connectFile = ui->leConnect->text().trimmed();
    if (save())
        mExportDriver->execute(connectFile, mRecentPath);
    else
        setControlsEnabled(true);
}

void ExportDialog::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e)
    on_pbCancel_clicked();
}

void ExportDialog::setControlsEnabled(bool enabled)
{
    ui->toolButton->setEnabled(enabled);
    ui->cbFilter->setEnabled(enabled);
    ui->leConnect->setEnabled(enabled);
    ui->leExcel->setEnabled(enabled);
    ui->pbBrowseConnect->setEnabled(enabled);
    ui->pbBrowseExcel->setEnabled(enabled);
    ui->comboBox->setEnabled(enabled);
    ui->tableView->setEnabled(enabled);
}

void ExportDialog::on_pbSelectAll_clicked()
{
    mExportModel->selectAll();
}

void ExportDialog::on_pbDeselectAll_clicked()
{
    mExportModel->deselectAll();
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
