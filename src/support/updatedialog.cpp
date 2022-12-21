/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#include "updatedialog.h"
#include "ui_updatedialog.h"
#include "updatechecker.h"

#include <QAbstractButton>
#include <QDesktopServices>

namespace gams {
namespace studio {
namespace support {

UpdateDialog::UpdateDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
    , ui(new Ui::UpdateDialog)
    , mUpdateChecker(new UpdateChecker(this))
{
    ui->setupUi(this);
    ui->updateInfo->setText(tr("Checking for updates..."));
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    connect(ui->updateInfo, &QTextBrowser::anchorClicked,
            this, &UpdateDialog::anchorClicked);
    connect(mUpdateChecker, &UpdateChecker::messageAvailable,
            ui->updateInfo, &QTextBrowser::setText);
    connect(this, &QDialog::rejected,
            this, &UpdateDialog::cancelupdateCheck);
    connect(ui->buttonBox, &QDialogButtonBox::clicked,
            this, &UpdateDialog::cancelupdateCheck);
    mUpdateChecker->start();
}

void UpdateDialog::anchorClicked(const QUrl &link)
{
    QDesktopServices::openUrl(QUrl("https://"+link.toString()));
}

void UpdateDialog::cancelupdateCheck()
{
    if (!mUpdateChecker->isRunning())
        return;
    mUpdateChecker->quit();
    mUpdateChecker->wait();
}

}
}
}
