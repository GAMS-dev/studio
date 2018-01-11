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
#include "resultsview.h"
#include "searchwidget.h"
#include "ui_resultsview.h"

namespace gams {
namespace studio {

ResultsView::ResultsView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResultsView)
{
    ui->setupUi(this);
}

ResultsView::~ResultsView()
{
    delete ui;
}

void ResultsView::addItem(Result r)
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(r.locFile()));
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(r.locLineNr())));
    ui->tableWidget->setItem(row, 2, new QTableWidgetItem(r.context()));
}

void ResultsView::resizeColumnsToContent()
{
    ui->tableWidget->resizeColumnsToContents();
}

}
}
