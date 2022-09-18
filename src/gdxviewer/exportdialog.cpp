#include "exportdialog.h"
#include "exportmodel.h"
#include "gdxsymbol.h"
#include "gdxsymboltablemodel.h"
#include "gdxsymbolview.h"
#include "ui_exportdialog.h"

#include <headerviewproxy.h>
#include <settings.h>

#include <QDebug>
#include <QFile>

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
    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->tableView->horizontalHeader()->setStyle(HeaderViewProxy::instance());
    mExportModel = new ExportModel(gdxViewer, mSymbolTableModel, this);
    ui->tableView->setModel(mExportModel);
    ui->tableView->hideRow(0);
    ui->tableView->resizeColumnsToContents();
}

ExportDialog::~ExportDialog()
{
    //delete mExportModel;
    //mExportModel = nullptr;
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
        inst += "    file: " + csvFile;
        inst += "    symbol:\n";
        inst += "      - name: " + sym->name() + "\n";
    }
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
