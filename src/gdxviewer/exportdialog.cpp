/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#include "exportmodel.h"
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

#include <process/connectprocess.h>
#include <numerics/doubleformatter.h>

namespace gams {
namespace studio {
namespace gdxviewer {

ExportDialog::ExportDialog(GdxViewer *gdxViewer, GdxSymbolTableModel *symbolTableModel, QWidget *parent) :
    QDialog(parent),
    mGdxViewer(gdxViewer),
    mSymbolTableModel(symbolTableModel),
    mProc(new ConnectProcess(this)),
    ui(new Ui::ExportDialog)
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QMenu* m = new QMenu();
    mSaveAction = m->addAction("Save", [this]() { ui->toolButton->setDefaultAction(mSaveAction), ExportDialog::save(); });
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
    mExportModel = new ExportModel(gdxViewer, mSymbolTableModel, this);
    mProxyModel = new QSortFilterProxyModel(this);
    mProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    mProxyModel->setSourceModel(mExportModel);

    ui->tableView->setModel(mProxyModel);
    ui->tableView->hideRow(0); // hide universe symbol
    ui->tableView->setColumnHidden(6,true); // hide the "Loaded" column
    ui->tableView->resizeColumnsToContents();

    ui->tableView->sortByColumn(2,Qt::AscendingOrder);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableView->verticalHeader()->setMinimumSectionSize(1);
    ui->tableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));

    connect(mProc.get(), &ConnectProcess::finished, [this]() {  exportDone(); });
}

ExportDialog::~ExportDialog()
{
    cancelProcess(50);
    mProxyModel->setSourceModel(nullptr);
    delete mProxyModel;
    mProxyModel = nullptr;
    delete mExportModel;
    mExportModel = nullptr;
    delete ui;
}

void ExportDialog::on_pbCancel_clicked()
{
    cancelProcess();
    reject();
}


QString ExportDialog::generateInstructions()
{
    QString output = ui->leExcel->text().trimmed();
    QString inst;
    inst += generateGdxReader();
    inst += generateFilters();
    inst += generateProjections();
    inst += generatePDExcelWriter(output);
    return inst;
}

QString ExportDialog::generateGdxReader()
{
    QString inst;
    inst += "- GDXReader:\n";
    inst += "    file: " + QDir::toNativeSeparators(mGdxFile) + "\n";
    inst += "    symbols: \n";
    for(GdxSymbol* sym: mExportModel->selectedSymbols())
        inst += "      - name: " + sym->aliasedSymbol()->name() + "\n";
    for(GdxSymbol* sym: mExportModel->selectedSymbols()) {
        if (sym->type() == GMS_DT_ALIAS) {
            QString dom = generateDomains(sym);
            inst += "- Projection:\n";
            inst += "    name: " + sym->aliasedSymbol()->name() + dom + "\n";
            inst += "    newName: " + sym->name() + dom + "\n";
        }
    }
    return inst;
}

QString ExportDialog::generatePDExcelWriter(QString excelFile)
{
    QString inst = "- PandasExcelWriter:\n";
    inst += "    file: " + excelFile + "\n";
    inst += "    excelWriterArguments: { engine: null, mode: w, if_sheet_exists: null}\n";
    inst += "    symbols:\n";
    for (GdxSymbol* sym: mExportModel->selectedSymbols()) {
        QString name = hasActiveFilter(sym) ? sym->name() + FILTER_SUFFIX : sym->name();
        QString range = sym->name() + "!A1";
        int rowDimension = sym->dim();
        if (sym->type() == GMS_DT_VAR || sym->type() == GMS_DT_EQU)
            name = sym->name() + PROJ_SUFFIX;
        GdxSymbolView *symView = mGdxViewer->symbolViewByName(sym->name());
        GdxViewerState *state = mGdxViewer->state();
        GdxSymbolViewState *symViewState = nullptr;
        if (state)
            symViewState = mGdxViewer->state()->symbolViewState(sym->aliasedSymbol()->name());
        if (symView && symView->isTableViewActive())
            rowDimension = sym->dim() - symView->getTvModel()->tvColDim();
        else if (symViewState && symViewState->tableViewActive())
            rowDimension = sym->dim() - symViewState->tvColDim();
        else if (!symView && !symViewState && sym->dim() > 1 && GdxSymbolView::DefaultSymbolView::tableView == Settings::settings()->toInt(SettingsKey::skGdxDefaultSymbolView))
            rowDimension = sym->dim() - 1;
        if (generateDomains(sym) != generateDomainsNew(sym))
            name = sym->name() + PROJ_SUFFIX;
        inst += "      - name: " + name + "\n";
        inst += "        range: " + sym->name() + "!A1\n";
        inst += "        rowDimension: " + QString::number(rowDimension) + "\n";
    }
    return inst;
}

QString ExportDialog::generateProjections()
{
    QString inst;
    for(GdxSymbol* sym: mExportModel->selectedSymbols()) {
        QString name;
        QString newName;
        bool asParameter = false;
        bool domOrderChanged = false;
        QString dom = generateDomains(sym);
        QString domNew = generateDomainsNew(sym);
        if (sym->type() == GMS_DT_VAR || sym->type() == GMS_DT_EQU) {
            if (hasActiveFilter(sym))
                name = sym->name() + FILTER_SUFFIX + dom;
            else
                name = sym->name() + dom;
            newName = sym->name() + PROJ_SUFFIX + dom;
            asParameter = true;
        }
        if (dom != domNew) {
            if (hasActiveFilter(sym))
                name = sym->name() + FILTER_SUFFIX  + dom;
            else
                name = sym->name() + dom;
            newName = sym->name() + PROJ_SUFFIX + domNew;
            domOrderChanged = true;
        }
        if (!name.isEmpty()) {
            inst += "- Projection:\n";
            inst += "    name: " + name + "\n";
            inst += "    newName: " + newName + "\n";
            if (asParameter)
                inst += "    asParameter: true\n";
            if (domOrderChanged) {
                inst += "- PythonCode:\n";
                inst += "    code: |\n";
                inst += "      r = connect.container.data['" + sym->name() + PROJ_SUFFIX + "'].records\n";
                inst += "      connect.container.data['" + sym->name() + PROJ_SUFFIX + "'].records=r.sort_values([c for c in r.columns])\n";
            }
        }
    }
    return inst;
}

QString ExportDialog::generateFilters()
{
    QString inst;
    for(GdxSymbol* sym: mExportModel->selectedSymbols()) {
        if (hasActiveFilter(sym)) {
            QString name = sym->name();
            QString newName = name + FILTER_SUFFIX;
            inst += "- Filter:\n";
            inst += "    name: " + name + "\n";
            inst += "    newName: " + newName + "\n";

            // label filters
            if (hasActiveLabelFilter(sym)) {
                inst += "    labelFilters:\n";
                for (int d=0; d<sym->dim(); d++) {
                    if (sym->filterActive(d)) {
                        bool *showUels = sym->showUelInColumn().at(d);
                        std::vector<int> uels = *sym->uelsInColumn().at(d);
                        inst += "      - column: " + QString::number(d+1) + "\n";

                        // switch between keep and reject for improved performance
                        int uelCount = uels.size();
                        int showUelCount = 0;
                        for (int uel: uels) {
                            if (showUels[uel])
                                showUelCount++;
                        }
                        bool useReject = showUelCount > uelCount/2;
                        if (useReject)
                            inst += "        reject: [";
                        else
                            inst += "        keep: [";
                        for (int uel: uels) {
                            if ( (!useReject && showUels[uel]) || (useReject && !showUels[uel]) )
                                inst += "'" + mSymbolTableModel->uel2Label(uel) + "', ";
                        }
                        int pos = inst.lastIndexOf(QChar(','));
                        inst.remove(pos, 2);
                        inst += "]\n";
                    }
                }
            }

            if (hasActiveValueFilter(sym)) {
                // value filters
                inst += "    valueFilters:\n";
                for (int d=sym->dim(); d<sym->filterColumnCount(); d++) {
                    if (sym->filterActive(d)) {
                        int valColIndex = d-sym->dim();
                        QStringList valColumns;
                        valColumns << "level" << "marginal" << "lower" << "upper" << "scale";
                        ValueFilter *vf = sym->valueFilter(valColIndex);
                        if (sym->type() == GMS_DT_VAR || sym->type() == GMS_DT_EQU)
                            inst += "      - column: " + valColumns[valColIndex] + "\n";
                        else // parameters
                            inst += "      - column: value\n";
                        QString min = numerics::DoubleFormatter::format(vf->currentMin(), numerics::DoubleFormatter::g, numerics::DoubleFormatter::gFormatFull, true);
                        QString max = numerics::DoubleFormatter::format(vf->currentMax(), numerics::DoubleFormatter::g, numerics::DoubleFormatter::gFormatFull, true);
                        if (vf->exclude()) {
                            inst += "        rule: (x<" + min + ") | (x>" + max + ")\n";
                        } else
                            inst += "        rule: (x>=" + min + ") & (x<=" + max + ")\n";

                        //special values
                        if (!vf->showEps())
                            inst += "        eps: false\n";
                        if (!vf->showPInf())
                            inst += "        infinity: false\n";
                        if (!vf->showMInf())
                            inst += "        negativeInfinity: false\n";
                        if (!vf->showNA())
                            inst += "        na: false\n";
                        if (!vf->showUndef())
                            inst += "        undf: false\n";
                    }
                }
            }
        }
    }
    return inst;
}

QString ExportDialog::generateDomains(GdxSymbol *sym)
{
    QString dom;
    sym = sym->aliasedSymbol();
    if (sym->dim() > 0) {
        dom = "(";
        for (int i=0; i<sym->dim(); i++)
            dom += QString::number(i) + ",";
        dom.truncate(dom.length()-1);
        dom += ")";
    }
    return dom;
}

QString ExportDialog::generateDomainsNew(GdxSymbol *sym)
{
    QString dom;
    sym = sym->aliasedSymbol();
    if (sym->dim() > 0) {
        GdxSymbolView *symView = mGdxViewer->symbolViewByName(sym->name());
        GdxViewerState *state = mGdxViewer->state();
        GdxSymbolViewState *symViewState = nullptr;
        if (state)
            symViewState = state->symbolViewState(sym->name());
        QVector<int> dimOrder;
        if (symView) {
            if (symView->isTableViewActive())
                dimOrder = symView->getTvModel()->tvDimOrder();
            else
                dimOrder = symView->listViewDimOrder();
        } else if (symViewState) {
            if (symViewState->tableViewActive())
                dimOrder = symViewState->tvDimOrder();
            else {
                QHeaderView *dummyHeader = new QHeaderView(Qt::Horizontal);
                dummyHeader->restoreState(symViewState->listViewHeaderState());
                for (int i=0; i<sym->columnCount(); i++) {
                    int idx = dummyHeader->logicalIndex(i);
                    if (idx<sym->dim())
                        dimOrder << idx;
                }
                delete dummyHeader;
            }
        }
        if (!dimOrder.isEmpty()) {
            dom = "(";
            for (int i=0; i<sym->dim(); i++)
                dom += QString::number(dimOrder.at(i)) + ",";
            dom.truncate(dom.length()-1);
            dom += ")";
            return dom;
        } else
            return generateDomains(sym);
    }
    return dom;
}

bool ExportDialog::hasActiveLabelFilter(GdxSymbol *sym)
{
    if (!ui->cbFilter->isChecked())
        return false;
    for (int i=0; i<sym->dim(); i++)
        if (sym->filterActive(i))
            return true;
    return false;
}

bool ExportDialog::hasActiveValueFilter(GdxSymbol *sym)
{
    if (!ui->cbFilter->isChecked())
        return false;
    for (int i=sym->dim(); i<sym->filterColumnCount(); i++)
        if (sym->filterActive(i))
            return true;
    return false;
}

bool ExportDialog::hasActiveFilter(GdxSymbol *sym)
{
    if (!ui->cbFilter->isChecked())
        return false;
    return hasActiveLabelFilter(sym) || hasActiveValueFilter(sym);
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

void ExportDialog::save()
{
    setControlsEnabled(false);
    QString connectFile = ui->leConnect->text().trimmed();
    save(connectFile, false);
    setControlsEnabled(true);
}

void ExportDialog::saveAndExecute()
{
    setControlsEnabled(false);
    QString connectFile = ui->leConnect->text().trimmed();
    if (save(connectFile))
        execute(connectFile);
    else
        setControlsEnabled(true);
}

void ExportDialog::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e)
    on_pbCancel_clicked();
}

void ExportDialog::exportDone()
{
    setControlsEnabled(true);
    accept();
}

bool ExportDialog::save(QString connectFile, bool fileExistsWarning)
{
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
    if (fileExistsWarning && QFileInfo(output).exists()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Overwrite Existing File");
        msgBox.setText(QFileInfo(output).fileName() + " already exists.\nDo you want to overwrite it?");
        msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        msgBox.setIcon(QMessageBox::Warning);
        if (msgBox.exec() == QMessageBox::No)
            return false;
    }

    mRecentPath = QFileInfo(output).path();
    ui->leExcel->setText(output);

    QFile f(connectFile);
    if (f.open(QFile::WriteOnly | QFile::Text)) {
        f.write(generateInstructions().toUtf8());
        f.close();
    }
    return true;
}

void ExportDialog::execute(QString connectFile)
{
    QStringList l;
    l << connectFile;
    mProc->setParameters(l);
    mProc->setWorkingDirectory(mRecentPath);
    mProc->execute();
}

void ExportDialog::cancelProcess(int waitMSec)
{
    if (mProc->state() != QProcess::NotRunning)
        mProc->stop(waitMSec);
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
