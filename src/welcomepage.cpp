/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include "welcomepage.h"
#include "studiosettings.h"
#include "ui_welcomepage.h"
#include "mainwindow.h"
#include "wplabel.h"

namespace gams {
namespace studio {

WelcomePage::WelcomePage(HistoryData *history, MainWindow *parent)
    : QWidget(parent), ui(new Ui::WelcomePage), mMain(parent)
{
    ui->setupUi(this);
    historyChanged(history);
    mOutputVisible = mMain->outputViewVisibility();

    connect(this, &WelcomePage::relayActionWp, parent, &MainWindow::receiveAction);
    connect(this, &WelcomePage::relayModLibLoad, parent, &MainWindow::receiveModLibLoad);
    connect(this, &WelcomePage::relayDocOpen, parent, &MainWindow::receiveOpenDoc);
}

void WelcomePage::historyChanged(HistoryData *history)
{
    QLayoutItem* item;
    while ((item = ui->layout_lastFiles->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    WpLabel *tmpLabel;
    int j = 0;
    for (int i = 0; i < history->lastOpenedFiles.size(); i++) {
        QFileInfo file(history->lastOpenedFiles.at(i));
        if (history->lastOpenedFiles.at(i) == "") continue;

        if (file.exists()) {
            tmpLabel = new WpLabel("<b>" + file.fileName() + "</b><br/>"
                                  + "<small>" + file.filePath() + "</small>", file.filePath(), this);
            tmpLabel->setToolTip(file.filePath());
            connect(tmpLabel, &QLabel::linkActivated, this, &WelcomePage::linkActivated);
            ui->layout_lastFiles->addWidget(tmpLabel);
            j++;
        }
    }
    if (j == 0) {
        tmpLabel = new WpLabel(QString("<b>No recent files.</b><br/>"
                                       "<small>Start using GAMS Studio to populate this list.</small>"), "", this);
        ui->layout_lastFiles->addWidget(tmpLabel);
    }
}

WelcomePage::~WelcomePage()
{
    delete ui;
}

void WelcomePage::on_relayAction(QString action)
{
    emit relayActionWp(action);
}

void WelcomePage::on_relayModLibLoad(QString lib)
{
    emit relayModLibLoad(lib);
}

void WelcomePage::on_relayOpenDoc(QString doc, QString anchor)
{
    emit relayDocOpen(doc, anchor);
}

void WelcomePage::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    mOutputVisible = mMain->outputViewVisibility();
    mMain->setOutputViewVisibility(false);
}

void WelcomePage::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    mMain->setOutputViewVisibility(mOutputVisible);
}

}
}
