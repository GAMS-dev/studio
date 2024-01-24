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
#include "exportdriver.h"
#include "gdxsymbol.h"
#include "gdxsymbolview.h"
#include "gdxviewer.h"
#include "valuefilter.h"
#include <settings.h>

#include <QDir>
#include <QMessageBox>
#include <QString>

#include <process/connectprocess.h>

namespace gams {
namespace studio {
namespace gdxviewer {

ExportDriver::ExportDriver(GdxViewer *gdxViewer, ExportModel *exportModel, QObject *parent)
    : QObject (parent),
      mProc(new ConnectProcess(this)),
      mGdxViewer(gdxViewer),
      mExportModel(exportModel)
{
    connect(mProc.get(), &ConnectProcess::finished, this, [this]() { emit exportDone(); });
}

ExportDriver::~ExportDriver()
{

}

bool ExportDriver::save(const QString& connectFile, const QString &output, bool applyFilters)
{
    QFile f(connectFile);
    if (f.open(QFile::WriteOnly | QFile::Text)) {
        f.write(generateInstructions(mGdxViewer->gdxFile(), output, applyFilters).toUtf8());
        f.close();
    }
    return true;
}

void ExportDriver::execute(const QString &connectFile, const QString &workingDirectory)
{
    QStringList l;
    l << connectFile;
    mProc->setParameters(l);
    mProc->setWorkingDirectory(workingDirectory);
    mProc->execute();
}

void ExportDriver::saveAndExecute(const QString &connectFile, const QString &output, const QString &workingDirectory, bool applyFilters)
{
    if (save(connectFile, output, applyFilters))
        execute(connectFile, workingDirectory);
}

void ExportDriver::cancelProcess(int waitMSec)
{
    if (mProc->state() != QProcess::NotRunning)
        mProc->stop(waitMSec);
}

QString ExportDriver::generateInstructions(const QString &gdxFile, const QString &output, bool applyFilters)
{
    QString inst;
    inst += generateGdxReader(gdxFile);
    if (applyFilters)
        inst += generateFilters();
    inst += generateProjections(applyFilters);
    inst += generatePDExcelWriter(output, applyFilters);
    return inst;
}

QString ExportDriver::generateGdxReader(const QString &gdxFile)
{
    QString inst;
    inst += "- GDXReader:\n";
    inst += "    file: " + QDir::toNativeSeparators(gdxFile) + "\n";
    inst += "    symbols: \n";
    QString instNames = "";
    QString instProjections = "";
    for (int i=0; i<mExportModel->selectedSymbols().size(); i++) {
        GdxSymbol* sym = mExportModel->selectedSymbols().at(i);
        instNames += "      - name: " + sym->aliasedSymbol()->name() + "\n";
        if (sym->type() == GMS_DT_ALIAS) {
            QString dom = generateDomains(sym);
            instProjections += "- Projection:\n";
            instProjections += "    name: " + sym->aliasedSymbol()->name() + dom + "\n";
            instProjections += "    newName: " + sym->name() + dom + "\n";
        }
    }
    return inst + instNames + instProjections;
}


QString ExportDriver::generatePDExcelWriter(const QString &excelFile, bool applyFilters)
{
    QString inst = "- PandasExcelWriter:\n";
    inst += "    file: " + excelFile + "\n";
    inst += "    excelWriterArguments: { engine: null, mode: w, if_sheet_exists: null}\n";
    inst += "    symbols:\n";
    for (int i=0; i<mExportModel->selectedSymbols().size(); i++) {
        GdxSymbol* sym = mExportModel->selectedSymbols().at(i);
        QString name = hasActiveFilter(sym) && applyFilters ? sym->name() + FILTER_SUFFIX : sym->name();
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


QString ExportDriver::generateProjections(bool applyFilters)
{
    QString inst;
    for (int i=0; i<mExportModel->selectedSymbols().size(); i++) {
        GdxSymbol* sym = mExportModel->selectedSymbols().at(i);
        QString name;
        QString newName;
        bool asParameter = false;
        bool domOrderChanged = false;
        QString dom = generateDomains(sym);
        QString domNew = generateDomainsNew(sym);
        if (sym->type() == GMS_DT_VAR || sym->type() == GMS_DT_EQU) {
            if (hasActiveFilter(sym) && applyFilters)
                name = sym->name() + FILTER_SUFFIX + dom;
            else
                name = sym->name() + dom;
            newName = sym->name() + PROJ_SUFFIX + dom;
            asParameter = true;
        }
        if (dom != domNew) {
            if (hasActiveFilter(sym) && applyFilters)
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


QString ExportDriver::generateFilters()
{
    QString inst;
    for (int i=0; i<mExportModel->selectedSymbols().size(); i++) {
        GdxSymbol* sym = mExportModel->selectedSymbols().at(i);
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
                        size_t uelCount = uels.size();
                        size_t showUelCount = 0;
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
                                inst += "'" + mGdxViewer->gdxSymbolTable()->uel2Label(uel) + "', ";
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


QString ExportDriver::generateDomains(GdxSymbol *sym)
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

QString ExportDriver::generateDomainsNew(GdxSymbol *sym)
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


bool ExportDriver::hasActiveLabelFilter(GdxSymbol *sym)
{
    for (int i=0; i<sym->dim(); i++)
        if (sym->filterActive(i))
            return true;
    return false;
}

bool ExportDriver::hasActiveValueFilter(GdxSymbol *sym)
{
    for (int i=sym->dim(); i<sym->filterColumnCount(); i++)
        if (sym->filterActive(i))
            return true;
    return false;
}

bool ExportDriver::hasActiveFilter(GdxSymbol *sym)
{
    return hasActiveLabelFilter(sym) || hasActiveValueFilter(sym);
}



} // namespace gdxviewer
} // namespace studio
} // namespace gams
