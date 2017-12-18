#include "searchwidget.h"
#include "ui_searchwidget.h"
#include <QDebug>

namespace gams {
namespace studio {


SearchWidget::SearchWidget(RecentData rec, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchWidget), mRecent(rec)
{
    ui->setupUi(this);
}

SearchWidget::~SearchWidget()
{
    delete ui;
}

void SearchWidget::on_buttonFind_clicked()
{
    qDebug() << "on_buttonFind_clicked in";
}

void SearchWidget::on_buttonReplace_clicked()
{
    qDebug() << "on_buttonReplace_clicked";
}

void SearchWidget::on_buttonFindAll_clicked()
{
    qDebug() << "on_buttonFindAll_clicked";
}

void SearchWidget::on_buttonReplaceAll_clicked()
{
    qDebug() << "on_buttonReplaceAll_clicked";
}

}
}

