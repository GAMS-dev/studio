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
#include "updatedialog.h"
#include "ui_updatedialog.h"
#include "gclgms.h"
#include "gamspaths.h"
#include "exception.h"

namespace gams {
namespace studio {

UpdateDialog::UpdateDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f),
      ui(new Ui::UpdateDialog)
{
    ui->setupUi(this);
    setUpdateInfo();
}

UpdateDialog::~UpdateDialog()
{
    c4uFree(&mCheckUpdate);
}

void UpdateDialog::setUpdateInfo()
{
    char message[GMS_SSSIZE];
    if (!c4uCreateD(&mCheckUpdate, GAMSPaths::systemDir().toLatin1(), message, GMS_SSSIZE)) {
        EXCEPT() << "Could not load c4u library: " << message;
    }

    c4uReadLice(mCheckUpdate, GAMSPaths::systemDir().toLatin1(), "gamslice.txt", false);
    if (!c4uIsValid(mCheckUpdate)) {
        c4uCreateMsg(mCheckUpdate);
    }

    QStringList messages;
    for (int i=0, c=c4uMsgCount(mCheckUpdate); i<c; ++i) {
        if (c4uGetMsg(mCheckUpdate, i, message))
            messages.append(message);
    }
    ui->updateInfo->setText(messages.join("\n"));
}

}
}
