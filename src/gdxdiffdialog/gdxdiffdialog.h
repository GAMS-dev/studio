/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMS_STUDIO_GDXDIFFDIALOG_H
#define GAMS_STUDIO_GDXDIFFDIALOG_H

#include <QDialog>

namespace gams {
namespace studio {

class FileMeta;
namespace gdxviewer {
    class GdxViewer;
}

namespace gdxdiffdialog {

namespace Ui {
class GdxDiffDialog;
}

class GdxDiffProcess;

class GdxDiffDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GdxDiffDialog(QWidget *parent = nullptr);
    ~GdxDiffDialog() override;

    void setRecentPath(const QString &recentPath);
    void setInput1(const QString &filePath);
    void setInput2(const QString &filePath);
    void reset();
    QString lastDiffFile() const;
    QString lastInput1() const;
    QString lastInput2() const;
    QString input1() const;
    QString input2() const;
    void prepopulateDiff();

protected:
    void closeEvent(QCloseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private slots:
    void on_pbInput1_clicked();
    void on_pbInput2_clicked();
    void on_pbDiff_clicked();
    void on_pbCancel_clicked();
    void on_pbOK_clicked();
    void on_cbFieldOnly_toggled(bool checked);
    void on_cbDiffOnly_toggled(bool checked);
    void on_cbFieldToCompare_currentIndexChanged(int index);
    void on_pbClear_clicked();
    void diffDone();

private:
    const QString defaultDiffFile = "diff.gdx";
    void setControlsEnabled(bool enabled);
    QStringList gdxDiffParamters();

    Ui::GdxDiffDialog *ui;
    QString mRecentPath;
    QString mWorkingDir;
    QString mLastDiffFile;
    QString mLastInput1;
    QString mLastInput2;

    QSharedPointer<GdxDiffProcess> mProc;
    gdxviewer::GdxViewer* mDiffGdxViewer = nullptr;
    FileMeta* mDiffFm = nullptr;
    bool mWasCanceled = false;
    bool mPrepopulateDiff = true;

    void cancelProcess(int waitMSec=0);


};

} // namespace gdxdiffdialog
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXDIFFDIALOG_H
