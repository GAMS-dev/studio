#ifndef GAMS_STUDIO_GDXVIEWER_EXPORTDIALOG_H
#define GAMS_STUDIO_GDXVIEWER_EXPORTDIALOG_H

#include "exportmodel.h"
#include <QDialog>
#include <process/connectprocess.h>

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxViewer;
class GdxSymbolTableModel;


namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(GdxViewer *gdxViewer, GdxSymbolTableModel *symbolTableModel, QWidget *parent = nullptr);
    ~ExportDialog();

private slots:
    void on_pbCancel_clicked();
    void on_pbExport_clicked();

    void on_pbBrows_clicked();

private:
    QString generateInstructions();
    QString generateGdxReader();
    QString generatePDExcelWriter(QString excelFile);
    QString generateProjections();

    QString generateDomains(GdxSymbol *sym);
    QString generateDomainsNew(GdxSymbol *sym);

    void setOutput(QString filePath);

    GdxViewer *mGdxViewer = nullptr;
    GdxSymbolTableModel *mSymbolTableModel = nullptr;
    Ui::ExportDialog *ui;
    ExportModel *mExportModel = nullptr;
    ConnectProcess *mProc = nullptr;
    QString mRecentPath;
    QString mGdxFile;
};


} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_EXPORTDIALOG_H
