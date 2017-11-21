/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#include "ui_welcomepage.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

namespace gams {
namespace studio {

WelcomePage::WelcomePage(HistoryData *history, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WelcomePage)
{
    ui->setupUi(this);
    connect(ui->label_documentation, &QLabel::linkActivated, this, &WelcomePage::labelLinkActivated);
    connect(ui->label_gamsworld, &QLabel::linkActivated, this, &WelcomePage::labelLinkActivated);
    connect(ui->label_gamsyoutube, &QLabel::linkActivated, this, &WelcomePage::labelLinkActivated);
    connect(ui->label_stackoverflow, &QLabel::linkActivated, this, &WelcomePage::labelLinkActivated);

    historyChanged(history);
}

void WelcomePage::historyChanged(HistoryData *history)
{
    int size = ui->layout_lastFiles->rowCount();
    qDebug() << "size" << size;
    for (int i = 0; i < size; i++) {
        ui->layout_lastFiles->removeRow(0);
    }

    QLabel *tmpLabel;
    qDebug() << "history->lastOpenedFiles.size()" << history->lastOpenedFiles.size();
    for (int i = 0; i < history->lastOpenedFiles.size(); i++) {
        QFileInfo file(history->lastOpenedFiles.at(i));
        tmpLabel = new QLabel("<a href='" + file.filePath() + "'>" + file.fileName() + "</a>");
        tmpLabel->setToolTip(file.filePath());

        connect(tmpLabel, &QLabel::linkActivated, this, &WelcomePage::linkActivated);
        ui->layout_lastFiles->addWidget(tmpLabel);
    }
}

WelcomePage::~WelcomePage()
{
    delete ui;
}

void WelcomePage::labelLinkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link, QUrl::TolerantMode));
}

}
}
