#include "exportdialog.h"
#include "exportmodel.h"
#include "gdxsymbol.h"
#include "gdxsymboltablemodel.h"
#include "gdxsymbolview.h"
#include "gdxviewer.h"
#include "ui_exportdialog.h"

#include <headerviewproxy.h>
#include <settings.h>

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

    mGdxFile = gdxViewer->gdxFile();
    mRecentPath = QFileInfo(mGdxFile).path();
    QString connectFile = mRecentPath + "/" + QFileInfo(mGdxFile).completeBaseName() + "_export.yaml";
    ui->leConnect->setText(QDir::toNativeSeparators(connectFile));
    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->tableView->horizontalHeader()->setStyle(HeaderViewProxy::instance());
    mExportModel = new ExportModel(gdxViewer, mSymbolTableModel, this);
    mProxyModel = new QSortFilterProxyModel(this);
    mProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    mProxyModel->setSourceModel(mExportModel);

    ui->tableView->setModel(mProxyModel);
    ui->tableView->hideRow(0); // hide universe symbol
    ui->tableView->setColumnHidden(6,true); // hide the "Loaded" column
    ui->tableView->setColumnHidden(7,true); // hide the "Text" column
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
    inst += generateProjections();
    inst += generatePDExcelWriter(output);
    return inst;
}

QString ExportDialog::generateGdxReader()
{
    QString inst;
    inst += "- GDXReader:\n";
    inst += "    file: " + mGdxFile + "\n";
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
    inst += "    symbols:\n";
    for (GdxSymbol* sym: mExportModel->selectedSymbols()) {
        QString name = sym->name();
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
        inst += "        range: " + mExportModel->range().at(sym->nr()) + "\n";
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
            name = sym->name() + dom;
            newName = sym->name() + PROJ_SUFFIX + dom;
            asParameter = true;
        }
        if (dom != domNew) {
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
            symViewState = mGdxViewer->state()->symbolViewState(sym->name());
        bool tableViewActive = symView && symView->isTableViewActive();
        bool hasTableViewState = symViewState && symViewState->tableViewActive();
        if (tableViewActive || hasTableViewState) {
            QVector<int> dimOrder;
            if (tableViewActive)
                dimOrder = symView->getTvModel()->tvDimOrder();
            else if (hasTableViewState)
                dimOrder = symViewState->tvDimOrder();
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
    save(connectFile);
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

bool ExportDialog::save(QString connectFile)
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
    else if (mExportModel->selectedSymbols().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("GDX Export");
        msgBox.setText("At least one symbol has to be selected for export");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return false;
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
    ui->leConnect->setEnabled(enabled);
    ui->leExcel->setEnabled(enabled);
    ui->pbBrowseConnect->setEnabled(enabled);
    ui->pbBrowseExcel->setEnabled(enabled);
    ui->comboBox->setEnabled(enabled);
    ui->tableView->setEnabled(enabled);
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
