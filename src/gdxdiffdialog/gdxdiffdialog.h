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
    ~GdxDiffDialog();

    void setRecentPath(const QString &recentPath);
    void setInput1(QString filePath);
    void setInput2(QString filePath);

    void clear();

    QString lastDiffFile() const;

    QString lastInput1() const;

    QString lastInput2() const;

private slots:

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pbCancel_clicked();

    void on_pbOK_clicked();

    void on_cbFieldOnly_toggled(bool checked);

    void on_cbDiffOnly_toggled(bool checked);

    void on_cbFieldToCompare_currentIndexChanged(int index);

    void on_pbClear_clicked();

private:
    Ui::GdxDiffDialog *ui;
    QString mRecentPath;
    QString mLastDiffFile;
    QString mLastInput1;
    QString mLastInput2;

};

} // namespace gdxdiffdialog
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXDIFFDIALOG_H
