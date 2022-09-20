#include "exportdialog.h"
#include "exportmodel.h"
#include "gdxsymbol.h"
#include "gdxsymboltablemodel.h"
#include "gdxsymbolview.h"
#include "gdxviewer.h"
#include "ui_exportdialog.h"

#include <headerviewproxy.h>
#include <settings.h>

#include <QDebug>
#include <QDir>
#include <QFile>

#include <process/connectprocess.h>

namespace gams {
namespace studio {
namespace gdxviewer {

ExportDialog::ExportDialog(GdxViewer *gdxViewer, GdxSymbolTableModel *symbolTableModel, QWidget *parent) :
    QDialog(parent),
    mGdxViewer(gdxViewer),
    mSymbolTableModel(symbolTableModel),
    ui(new Ui::ExportDialog)
{
    ui->setupUi(this);
    mProc = new ConnectProcess(this);
    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->tableView->horizontalHeader()->setStyle(HeaderViewProxy::instance());
    mExportModel = new ExportModel(gdxViewer, mSymbolTableModel, this);
    ui->tableView->setModel(mExportModel);
    ui->tableView->hideRow(0);
    ui->tableView->resizeColumnsToContents();
}

ExportDialog::~ExportDialog()
{
    delete mExportModel;
    mExportModel = nullptr;
    delete mProc;
    mProc = nullptr;
    delete ui;
}

void ExportDialog::on_pbCancel_clicked()
{
    reject();
}

void ExportDialog::on_pbExport_clicked()
{
    QString inst = "";
    for(GdxSymbol* sym: mExportModel->selectedSymbols()) {
        QString csvFile = Settings::settings()->toString(skDefaultWorkspace) + "/" + "export_" + sym->name() + ".csv";
        writeSymbolToCsv(sym, csvFile);
        inst += "- CSVReader:\n";
        inst += "    file: " + csvFile + "\n";
        inst += "    name: " + sym->name() + "\n";
        inst += "    header: false\n";
        if (sym->type() == GMS_DT_PAR) {
            inst += "    indexColumns: ";
            QString idxCols = "";
            for (int i=1; i<=sym->dim(); i++)
                idxCols += QString::number(i) + ",";
            idxCols.truncate(idxCols.length()-1);
            inst += idxCols + "\n";
            inst += "    valueColumns: " + QString::number(sym->dim()+1) + "\n";
        }
    }

    QString excelFile = "output.xlsx";

    inst += "- PandasExcelWriter:\n";
    inst += "    file: " + excelFile + "\n";
    inst += "    symbols:\n";
    for(GdxSymbol* sym: mExportModel->selectedSymbols()) {
        inst += "      - name: " + sym->name() + "\n";
        int rowDimension = sym->dim();
        if (mGdxViewer->symbolViewByName(sym->name())->isTableViewActive())
            rowDimension = mGdxViewer->symbolViewByName(sym->name())->getTvModel()->tvColDim() - sym->dim();
        inst += "        rowDimension: " + QString::number(rowDimension) + "\n";
    }

    QString instYaml = Settings::settings()->toString(skDefaultWorkspace) + "/" + "do_export.yaml";
    QFile f(instYaml);
    if (f.open(QFile::WriteOnly | QFile::Text)) {
        f.write(inst.toUtf8());
        f.close();
    }
    QStringList l;
    l << QDir::toNativeSeparators(instYaml);
    mProc->setParameters(l);
    mProc->setWorkingDirectory(Settings::settings()->toString(skDefaultWorkspace));
    mProc->execute();
}

bool ExportDialog::writeSymbolToCsv(GdxSymbol *sym, QString file)
{
    QFile f(file);
    if (f.open(QFile::WriteOnly | QFile::Text)) {
        if (!mGdxViewer->symbolViewByName(sym->name())) {
            mGdxViewer->createSymbolView(sym, sym->nr());
            if (!sym->isLoaded())
                sym->loadData();
        }
        GdxSymbolView *symView = mGdxViewer->symbolViewByName(sym->name());
        f.write(symView->dataAsCsv().toUtf8());
        f.close();
        return true;
    }
    return false;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
