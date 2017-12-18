#include "searchwidget.h"
#include "ui_searchwidget.h"
#include <QDebug>

namespace gams {
namespace studio {


SearchWidget::SearchWidget(RecentData &rec, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SearchWidget), mRecent(rec)
{
    ui->setupUi(this);
}

SearchWidget::~SearchWidget()
{
    delete ui;
}

}
}

