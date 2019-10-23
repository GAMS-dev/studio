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
    explicit GdxDiffDialog(QString recentPath, QWidget *parent = nullptr);
    ~GdxDiffDialog();
    QString diffFile();

private slots:

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pbCancel_clicked();

    void on_pbOK_clicked();

private:
    Ui::GdxDiffDialog *ui;
    QString mRecentPath;
    QString mDiffFile;
};

} // namespace gdxdiffdialog
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXDIFFDIALOG_H
