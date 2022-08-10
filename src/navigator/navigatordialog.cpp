#include "navigatordialog.h"

namespace gams {
namespace studio {

NavigatorDialog::NavigatorDialog(QWidget *parent) : QDialog(parent), ui(new Ui::Navigator)
{
    ui->setupUi(this);
    // todo
}

NavigatorDialog::~NavigatorDialog()
{
    delete ui;
}

}
}
