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
#include "gotodialog.h"
#include "ui_gotodialog.h"
#include "mainwindow.h"

namespace gams {
namespace studio {


GoToDialog::GoToDialog(MainWindow *parent) :
    QDialog(parent), ui(new Ui::GoToDialog), mMain(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    ui->lineEdit->setValidator(new QIntValidator(0, 1000000, this) );

    connect(ui->lineEdit, &QLineEdit::editingFinished, this, &GoToDialog::on_goToButton_clicked);
}

GoToDialog::~GoToDialog()
{
    delete ui;
}

void GoToDialog::on_goToButton_clicked()
{
    int altLine =(ui->lineEdit->text().toInt())-1;
    QTextCursor cursor;
    ProjectFileNode* fc = mMain->projectRepo()->fileNode(mMain->recent()->editor());
    if (!fc) return;
    fc->jumpTo(cursor, true, altLine, 0);
    ui->lineEdit->clear();
    close();
}

void GoToDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    }
}

}
}
