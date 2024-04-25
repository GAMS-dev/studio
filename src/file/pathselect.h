#ifndef GAMS_STUDIO_PATHSELECT_H
#define GAMS_STUDIO_PATHSELECT_H

#include <QDialog>
#include <QLineEdit>

namespace gams {
namespace studio {
namespace pathselect {

namespace Ui {
class PathSelect;
}

class PathSelect : public QDialog
{
    Q_OBJECT

public:
    explicit PathSelect(QWidget *parent = nullptr);
    ~PathSelect() override;

signals:
    void workDirSelected(const QString &workDir);
    void canceled();

private slots:
    void on_edWorkDir_textChanged(const QString &text);
    void on_bWorkDir_clicked();
    void on_pbOk_clicked();

    void on_pbCancel_clicked();

private:
    void showDirDialog(const QString &title, QLineEdit *lineEdit, const QString &defaultDir);

private:
    Ui::PathSelect *ui;
};

} // namespace pathselect
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_PATHSELECT_H
