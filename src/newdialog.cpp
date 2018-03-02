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
#include "newdialog.h"

#include <QFileDialog>

namespace gams {
namespace studio {

NewDialog::NewDialog(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.toolButton, &QToolButton::pressed, this, &NewDialog::directory);
}

QString NewDialog::fileName() const
{
    return ui.nameEdit->text();
}

QString NewDialog::location() const
{
    return ui.locationEdit->text();
}

void NewDialog::directory()
{
    auto dir = QFileDialog::getExistingDirectory(this, "Choose a directory", ".");
    ui.locationEdit->setText(dir);
}

}
}
