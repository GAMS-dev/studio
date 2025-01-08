/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "descriptiondialog.h"
#include "ui_descriptiondialog.h"

namespace gams {
namespace studio {
namespace modeldialog {

DescriptionDialog::DescriptionDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DescriptionDialog)
{
    ui->setupUi(this);
    mDefaultFont = ui->textEdit->document()->defaultFont();
    connect(ui->okButton, &QPushButton::clicked, this, &QDialog::accept);
}

DescriptionDialog::~DescriptionDialog()
{
    delete ui;
}

void DescriptionDialog::setText(const QString &text)
{
    ui->textEdit->setPlainText(text);
}

void DescriptionDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier ||
        (event->modifiers() ^ Qt::ControlModifier && event->modifiers() ^ Qt::KeypadModifier)) {
        if (event->key() == Qt::Key_Plus) {
            ui->textEdit->zoomIn();
        } else if (event->key() == Qt::Key_Minus) {
            ui->textEdit->zoomOut();
        } else if (event->key() == Qt::Key_0) {
            ui->textEdit->setFont(mDefaultFont);
        }
    }
}

}
}
}
