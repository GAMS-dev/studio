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
    QString instGDXR = "";
    instGDXR += "- GDXReader:\n";
    instGDXR += "    file: " + mGdxViewer->gdxFile() + "\n";
    instGDXR += "    symbols: \n";
    for(GdxSymbol* sym: mExportModel->selectedSymbols())
        instGDXR += "      - name: " + sym->name() + "\n";

    QString excelFile = "output.xlsx";
    QStringList instProjections;

    QString instPDEW = "";
    instPDEW += "- PandasExcelWriter:\n";
    instPDEW += "    file: " + excelFile + "\n";
    instPDEW += "    symbols:\n";
    for(GdxSymbol* sym: mExportModel->selectedSymbols()) {
        int rowDimension = sym->dim();
        GdxSymbolView *symView = mGdxViewer->symbolViewByName(sym->name());
        if (symView && symView->isTableViewActive()) {
            instPDEW += "      - name: " + sym->name() + "_proj\n";
            instPDEW += "        range: " + sym->name() + "!A1\n";
            rowDimension = sym->dim() - symView->getTvModel()->tvColDim();
            QVector<int> dimOrder = symView->getTvModel()->tvDimOrder();
            QString ip = "- Projection:\n";
            QString dom = "(";
            QString domNew = "(";
            for (int i=0; i<sym->dim(); i++) {
                dom += QString::number(i) + ",";
                domNew += QString::number(dimOrder.at(i)) + ",";
            }
            dom.truncate(dom.length()-1);
            domNew.truncate(domNew.length()-1);
            dom += ")";
            domNew += ")";
            ip += "    name: " + sym->name() + dom + "\n";
            ip += "    newName: " + sym->name() + "_proj" + domNew + "\n";
            ip += "- PythonCode:\n";
            ip += "    code: |\n";
            ip += "      r = connect.container.data['" + sym->name() + "_proj'].records\n";
            ip += "      connect.container.data['" + sym->name() + "_proj'].records=r.sort_values([c for c in r.columns])\n";
            instProjections.append(ip);
        } else {
            instPDEW += "      - name: " + sym->name() + "\n";
            instPDEW += "        range: " + sym->name() + "!A1\n";
        }
        instPDEW += "        rowDimension: " + QString::number(rowDimension) + "\n";
    }

    QString instYaml = Settings::settings()->toString(skDefaultWorkspace) + "/" + "do_export.yaml";
    QFile f(instYaml);
    if (f.open(QFile::WriteOnly | QFile::Text)) {
        f.write(instGDXR.toUtf8());
        for (QString i : instProjections)
            f.write(i.toUtf8());
        f.write(instPDEW.toUtf8());
        f.close();
    }
    QStringList l;
    l << instYaml;
    mProc->setParameters(l);
    mProc->setWorkingDirectory(Settings::settings()->toString(skDefaultWorkspace));
    mProc->execute();
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
