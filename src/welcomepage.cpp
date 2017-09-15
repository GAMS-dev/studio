#include "welcomepage.h"
#include "ui_welcomepage.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

namespace gams {
namespace ide {

WelcomePage::WelcomePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WelcomePage)
{
    ui->setupUi(this);
    connect(ui->label_documentation, &QLabel::linkActivated, this, &WelcomePage::labelLinkActivated);
    connect(ui->label_gamsworld, &QLabel::linkActivated, this, &WelcomePage::labelLinkActivated);
    connect(ui->label_gamsyoutube, &QLabel::linkActivated, this, &WelcomePage::labelLinkActivated);
    connect(ui->label_stackoverflow, &QLabel::linkActivated, this, &WelcomePage::labelLinkActivated);
}

WelcomePage::~WelcomePage()
{
    delete ui;
}

void WelcomePage::labelLinkActivated(const QString &link)
{
    qDebug() << "click";
    QDesktopServices::openUrl(QUrl(link, QUrl::TolerantMode));
}

}
}
