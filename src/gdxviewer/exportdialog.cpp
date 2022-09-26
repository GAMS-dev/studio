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
#include <QFileDialog>
#include <QMessageBox>

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
    mRecentPath = Settings::settings()->toString(skDefaultWorkspace);
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
    QString output = ui->lineEdit->text().trimmed();
    if (output.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("GDX Export");
        msgBox.setText("Output file can not be empty:\n" + output);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    else if (QFileInfo(output).isRelative())
        output = QDir::toNativeSeparators(Settings::settings()->toString(skDefaultWorkspace) + QDir::separator() + output);
    if (QFileInfo(output).suffix().isEmpty())
        output = output + ".xlsx";
    if (QFileInfo(output).exists()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Overwrite Existing File");
        msgBox.setText(QFileInfo(output).fileName() + " already exists.\nDo you want to overwrite it?");
        msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        msgBox.setIcon(QMessageBox::Warning);
        if (msgBox.exec() == QMessageBox::No)
            return;
    }

    mRecentPath = QFileInfo(output).path();
    ui->lineEdit->setText(output);

    QString instFilePath = mRecentPath + "/" + "do_export.yaml";
    QFile f(instFilePath);
    if (f.open(QFile::WriteOnly | QFile::Text)) {
        f.write(generateInstructions().toUtf8());
        f.close();
    }
    QStringList l;
    l << instFilePath;
    mProc->setParameters(l);
    mProc->setWorkingDirectory(Settings::settings()->toString(skDefaultWorkspace));
    mProc->execute();
}

QString ExportDialog::generateInstructions()
{
    QString output = ui->lineEdit->text().trimmed();
    QString inst;
    inst += generateGdxReader();
    inst += generateProjections();
    inst += generatePDExcelWriter(output);
    return inst;
}

QString ExportDialog::generateGdxReader()
{
    QString inst;
    inst += "- GDXReader:\n";
    inst += "    file: " + mGdxViewer->gdxFile() + "\n";
    inst += "    symbols: \n";
    for(GdxSymbol* sym: mExportModel->selectedSymbols())
        inst += "      - name: " + sym->name() + "\n";
    return inst;
}

QString ExportDialog::generatePDExcelWriter(QString excelFile)
{
    QString inst = "- PandasExcelWriter:\n";
    inst += "    file: " + excelFile + "\n";
    inst += "    symbols:\n";
    for (GdxSymbol* sym: mExportModel->selectedSymbols()) {
        QString name = sym->name();
        QString range = sym->name() + "!A1";
        int rowDimension = sym->dim();
        if (sym->type() == GMS_DT_VAR || sym->type() == GMS_DT_EQU)
            name += "_proj";
        GdxSymbolView *symView = mGdxViewer->symbolViewByName(sym->name());
        if (symView && symView->isTableViewActive()) {
            if (generateDomains(sym) != generateDomainsNew(sym))
                name = sym->name() + "_proj";
            rowDimension = sym->dim() - symView->getTvModel()->tvColDim();
        }
        inst += "      - name: " + name + "\n";
        inst += "        range: " + range + "\n";
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
        if (sym->type() == GMS_DT_VAR || sym->type() == GMS_DT_EQU) {
            name = sym->name() + dom;
            newName = sym->name() + "_proj" + dom;
            asParameter = true;
        }
        GdxSymbolView *symView = mGdxViewer->symbolViewByName(sym->name());
        if (symView && symView->isTableViewActive()) {
            QString domNew = generateDomainsNew(sym);
            if (dom != domNew) {
                name = sym->name() + dom;
                newName = sym->name() + "_proj" + domNew;
                domOrderChanged = true;
            }
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
                inst += "      r = connect.container.data['" + sym->name() + "_proj'].records\n";
                inst += "      connect.container.data['" + sym->name() + "_proj'].records=r.sort_values([c for c in r.columns])\n";
            }
        }
    }
    return inst;
}

QString ExportDialog::generateDomains(GdxSymbol *sym)
{
    QString dom;
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
    if (sym->dim() > 0) {
        GdxSymbolView *symView = mGdxViewer->symbolViewByName(sym->name());
        if (symView && symView->isTableViewActive()) {
            QVector<int> dimOrder = symView->getTvModel()->tvDimOrder();
            dom = "(";
            for (int i=0; i<sym->dim(); i++)
                dom += QString::number(dimOrder.at(i)) + ",";
            dom.truncate(dom.length()-1);
            dom += ")";
            return dom;
        }
        return generateDomains(sym);
    }
    return dom;
}

void ExportDialog::setOutput(QString filePath)
{
    ui->lineEdit->setText(QDir::toNativeSeparators(filePath));
}

void ExportDialog::on_pbBrows_clicked()
{
    QString filter("Excel file (*.xlsx);");
    QString filePath = QFileDialog::getSaveFileName(this, "Choose Excel File...",
                                                            mRecentPath,
                                                            filter,
                                                            nullptr,
                                                            QFileDialog::DontConfirmOverwrite);
    if (!filePath.isEmpty()) {
        mRecentPath = QFileInfo(filePath).path();
        setOutput(filePath);
    }
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
