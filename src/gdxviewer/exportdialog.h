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

private slots:
    void exportDone();
    void on_pbSelectAll_clicked();
    void on_pbDeselectAll_clicked();

private:
    bool save(QString connectFile, bool fileExistsWarning=true);
    void execute(QString connectFile);
    void cancelProcess(int waitMSec=0);
    void setControlsEnabled(bool enabled);

    const QString PROJ_SUFFIX = "_proj_";
    const QString FILTER_SUFFIX = "_filter_";
    QString generateInstructions();
    QString generateGdxReader();
    QString generatePDExcelWriter(QString excelFile);
    QString generateProjections();
    QString generateFilters();

    QString generateDomains(GdxSymbol *sym);
    QString generateDomainsNew(GdxSymbol *sym);

    bool hasActiveLabelFilter(GdxSymbol *sym);
    bool hasActiveValueFilter(GdxSymbol *sym);
    bool hasActiveFilter(GdxSymbol *sym);

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
