#ifndef GAMS_STUDIO_TABDIALOG_H
#define GAMS_STUDIO_TABDIALOG_H

#include <QDialog>
#include <QTabWidget>

namespace gams {
namespace studio {
namespace tabdialog {

namespace Ui {
class TabDialog;
}

class TabDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TabDialog(QTabWidget *tabs, QWidget *parent = nullptr);
    ~TabDialog();

private:
    Ui::TabDialog *ui;

};


}
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_TABDIALOG_H
