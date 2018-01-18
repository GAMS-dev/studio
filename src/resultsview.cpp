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
#include "exception.h"

namespace gams {
namespace studio {

ResultsView::ResultsView(SearchResultList resultList, MainWindow *parent) :
    QWidget(parent), ui(new Ui::ResultsView), mMain(parent), mResultList(resultList)
{
    ui->setupUi(this);
    foreach (Result item, resultList.resultList()) {
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(item.locFile()));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(item.locLineNr())));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(item.context()));
    }
}

ResultsView::~ResultsView()
{
    delete ui;
}

void ResultsView::resizeColumnsToContent()
{
    ui->tableWidget->resizeColumnsToContents();
}

void ResultsView::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    FileSystemContext *fsc = mMain->fileRepository()->findContext(item->data(0).toString());
    FileContext *jmpFc = nullptr;
    if (!fsc) EXCEPT() << "File not found:" << item->data(0).toString();

    if (fsc->type() == FileSystemContext::File)
        jmpFc = static_cast<FileContext*>(fsc);

    if (!jmpFc) EXCEPT() << "Not a file:" << item->data(0).toString();

    mMain->openFile(item->data(0).toString());

//    QTextCursor tc(jmpFc->document()->findBlockByLineNumber(item->data(1).toInt()));
//    qDebug() << "data 0" << item->data(0);
//    qDebug() << "data 1" << item->data(1);
//    qDebug() << "data 2" << item->data(2);
//    jmpFc->jumpTo(tc, false);
}

}
}
