/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "exportdriver.h"
#include <QDialog>
#include <QSortFilterProxyModel>

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
    explicit ExportDialog(GdxViewer *gdxViewer, QWidget *parent = nullptr);
    ~ExportDialog();

private slots:
    void on_pbCancel_clicked();
    void on_pbBrowseExcel_clicked();
    void on_pbBrowseConnect_clicked();

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void on_pbSelectAll_clicked();
    void on_pbDeselectAll_clicked();

private:
    bool save(bool fileExistsWarning=true);
    void saveAndExecute();
    void execute(QString connectFile);
    void setControlsEnabled(bool enabled);

    GdxViewer *mGdxViewer = nullptr;
    GdxSymbolTableModel *mSymbolTableModel = nullptr;

    Ui::ExportDialog *ui;
    ExportModel *mExportModel = nullptr;
    ExportDriver *mExportDriver = nullptr;
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
