/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
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
        disconnect(this, &DebugWidget::sendStop, mServer, &Server::signalStop);
        disconnect(mServer, &Server::stateChanged, this, &DebugWidget::stateChanged);
    }
    mServer = server;
    if (mServer) {
        connect(this, &DebugWidget::sendRun, mServer, &Server::sendRun);
        connect(this, &DebugWidget::sendStepLine, mServer, &Server::sendStepLine);
        connect(this, &DebugWidget::sendPause, mServer, &Server::sendPause);
        connect(this, &DebugWidget::sendStop, mServer, &Server::signalStop);
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


void DebugWidget::on_tbStop_clicked()
{
    emit sendStop();
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
