#include <QKeyEvent>
#include "navigatordialog.h"

#include <QTime>
#include <QDebug>

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

void NavigatorDialog::keyPressEvent(QKeyEvent *e)
{
     QDialog::keyPressEvent(e);
}

}
}
