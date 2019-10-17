#ifndef GAMS_STUDIO_GDXDIFFDIALOG_H
#define GAMS_STUDIO_GDXDIFFDIALOG_H

#include <QDialog>

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

private slots:

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::GdxDiffDialog *ui;
};

} // namespace gdxdiffdialog
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXDIFFDIALOG_H
