#ifndef GAMS_STUDIO_GDXVIEWER_EXPORTDIALOG_H
#define GAMS_STUDIO_GDXVIEWER_EXPORTDIALOG_H

#include "exportmodel.h"
#include <QDialog>
#include <QSortFilterProxyModel>
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
    void on_pbBrowseExcel_clicked();
    void on_pbBrowseConnect_clicked();
    void save();
    void saveAndExecute();

protected:
    void closeEvent(QCloseEvent *e) override;

private:
    void save(QString connectFile);
    void execute(QString connectFile);
    void cancelProcess(int waitMSec=0);
    void setControlsEnabled(bool enabled);

    const QString PROJ_SUFFIX = "_proj_";
    QString generateInstructions();
    QString generateGdxReader();
    QString generatePDExcelWriter(QString excelFile);
    QString generateProjections();

    QString generateDomains(GdxSymbol *sym);
    QString generateDomainsNew(GdxSymbol *sym);

    GdxViewer *mGdxViewer = nullptr;
    GdxSymbolTableModel *mSymbolTableModel = nullptr;
    std::unique_ptr<ConnectProcess> mProc = nullptr;
    Ui::ExportDialog *ui;
    ExportModel *mExportModel = nullptr;
    QSortFilterProxyModel* mProxyModel = nullptr;
    QString mRecentPath;
    QString mGdxFile;

    QAction *mExportAction = nullptr;
    QAction *mSaveAction = nullptr;
};


} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_EXPORTDIALOG_H
