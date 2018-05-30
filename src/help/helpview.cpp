#include <QToolBar>

#include "helpview.h"
#include "ui_helpview.h"

namespace gams {
namespace studio {

HelpView::HelpView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HelpView)
{
    ui->setupUi(this);
}

HelpView::~HelpView()
{
    delete ui;
}

} // namespace studio
} // namespace gams
