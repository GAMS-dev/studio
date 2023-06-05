#include "debugwidget.h"
#include "ui_debugwidget.h"
#include "server.h"

namespace gams {
namespace studio {
namespace debugger {

DebugWidget::DebugWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebugWidget)
{
    ui->setupUi(this);
}

DebugWidget::~DebugWidget()
{
    delete ui;
}

void DebugWidget::setText(const QString &text)
{
    ui->laInfo->setText(text);
}

void DebugWidget::setDebugServer(Server *server)
{
    if (mServer) {
        disconnect(this, &DebugWidget::sendRun, mServer, &Server::sendRun);
        disconnect(this, &DebugWidget::sendStepLine, mServer, &Server::sendStepLine);
        disconnect(this, &DebugWidget::sendInterrupt, mServer, &Server::sendInterrupt);
    }
    mServer = server;
    if (mServer) {
        connect(this, &DebugWidget::sendRun, mServer, &Server::sendRun);
        connect(this, &DebugWidget::sendStepLine, mServer, &Server::sendStepLine);
        connect(this, &DebugWidget::sendInterrupt, mServer, &Server::sendInterrupt);
    }
}


void DebugWidget::on_tbRun_clicked()
{
    emit sendRun();
}


void DebugWidget::on_tbStep_clicked()
{
    emit sendStepLine();
}


void DebugWidget::on_tbStop_clicked()
{
    emit sendInterrupt();
}

} // namespace debugger
} // namespace studio
} // namespace gams
