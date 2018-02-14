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
#include "wplabel.h"
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
    historyChanged(history);
}

void WelcomePage::historyChanged(HistoryData *history)
{
    QLayoutItem* item;
    while ((item = ui->layout_lastFiles->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    QLabel *tmpLabel;
    for (int i = 0; i < history->lastOpenedFiles.size(); i++) {
        QFileInfo file(history->lastOpenedFiles.at(i));
        if (history->lastOpenedFiles.at(i) == "") continue;
        if (file.exists()) {
            tmpLabel = new WpLabel("<a href='" + file.filePath() + "'>" + file.fileName() + "</a><br/>"
                                  + "<small>" + file.filePath() + "</small>", file.filePath());
            tmpLabel->setToolTip(file.filePath());
            tmpLabel->setFrameShape(QFrame::StyledPanel);
            tmpLabel->setMargin(8);
            connect(tmpLabel, &QLabel::linkActivated, this, &WelcomePage::linkActivated);
        } else {
            tmpLabel = new QLabel(file.fileName() + " (File missing!)<br/><small>" + file.canonicalPath() + "</small>");
        }
        ui->layout_lastFiles->addWidget(tmpLabel);
    }
}

WelcomePage::~WelcomePage()
{
    delete ui;
}

}
}
