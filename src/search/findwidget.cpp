#include "findwidget.h"
#include "ui_findwidget.h"

namespace gams {
namespace studio {
namespace find {

FindWidget::FindWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FindWidget)
{
    ui->setupUi(this);
}

FindWidget::~FindWidget()
{
    delete ui;
}

bool FindWidget::active() const
{
    return mActive;
}

void FindWidget::setActive(bool newActive)
{
    mActive = newActive;
}

void FindWidget::toggleActive()
{
    setActive(!mActive);
}

void FindWidget::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    ui->edFind->setFocus();
}

} // namespace find
} // namespace studio
} // namespace gams
