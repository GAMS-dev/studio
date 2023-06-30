#include "debugwidget.h"
#include "ui_debugwidget.h"

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
    DebugState state = None;
    if (mServer) {
        disconnect(this, &DebugWidget::sendRun, mServer, &Server::sendRun);
        disconnect(this, &DebugWidget::sendStepLine, mServer, &Server::sendStepLine);
        disconnect(this, &DebugWidget::sendPause, mServer, &Server::sendPause);
        disconnect(mServer, &Server::stateChanged, this, &DebugWidget::stateChanged);
    }
    mServer = server;
    if (mServer) {
        connect(this, &DebugWidget::sendRun, mServer, &Server::sendRun);
        connect(this, &DebugWidget::sendStepLine, mServer, &Server::sendStepLine);
        connect(this, &DebugWidget::sendPause, mServer, &Server::sendPause);
        connect(mServer, &Server::stateChanged, this, &DebugWidget::stateChanged);
        state = mServer->state();
    }
    stateChanged(state);
}


void DebugWidget::on_tbRun_clicked()
{
    emit sendRun();
}


void DebugWidget::on_tbStep_clicked()
{
    emit sendStepLine();
}


void DebugWidget::on_tbPause_clicked()
{
    emit sendPause();
}

void DebugWidget::stateChanged(DebugState state)
{
    bool canPause = (state == Prepare || state == Running);
    ui->tbRun->setEnabled(!canPause);
    ui->tbStep->setEnabled(!canPause);
    ui->tbPause->setEnabled(canPause);
}

} // namespace debugger
} // namespace studio
} // namespace gams
