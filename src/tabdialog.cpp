#include "tabdialog.h"
#include "ui_tabdialog.h"


namespace gams {
namespace studio {
namespace tabdialog {

TabDialog::TabDialog(QTabWidget *tabs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TabDialog)
{
    ui->setupUi(this);

//    ui
}

TabDialog::~TabDialog()
{
    delete ui;
}

} // namespace studio
} // namespace gams
}
