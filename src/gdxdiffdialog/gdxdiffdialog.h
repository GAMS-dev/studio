#ifndef GAMS_STUDIO_GDXDIFFDIALOG_H
#define GAMS_STUDIO_GDXDIFFDIALOG_H

#include <QDialog>
#include <mainwindow.h>
#include "gdxdiffprocess.h"

namespace gams {
namespace studio {
namespace gdxdiffdialog {

namespace Ui {
class GdxDiffDialog;
}

class GdxDiffDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GdxDiffDialog(QWidget *parent = nullptr);
    ~GdxDiffDialog() override;

    void setRecentPath(const QString &recentPath);
    void setInput1(QString filePath);
    void setInput2(QString filePath);
    void reset();
    QString lastDiffFile() const;
    QString lastInput1() const;
    QString lastInput2() const;
    QString input1() const;
    QString input2() const;
    void prepopulateDiff();

protected:
    void closeEvent(QCloseEvent *e) override;

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

    std::unique_ptr<GdxDiffProcess> mProc;
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
