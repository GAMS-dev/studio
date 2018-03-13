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
#include "version.h"

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
    c4uFree(&mC4UHandle);
}

void UpdateDialog::setUpdateInfo()
{
    char buffer[GMS_SSSIZE];
    if (!c4uCreateD(&mC4UHandle, GAMSPaths::systemDir().toLatin1(), buffer, GMS_SSSIZE)) {
        EXCEPT() << "Could not load c4u library: " << buffer;
    }

    c4uReadLice(mC4UHandle, GAMSPaths::systemDir().toLatin1(), "gamslice.txt", false);
    if (!c4uIsValid(mC4UHandle)) {
        c4uCreateMsg(mC4UHandle);
    }

    int messageIndex=0;
    mMessages << "GAMS Distribution";
    getMessages(messageIndex, buffer);

    mMessages << "\nGAMS Studio";
    c4uCheck4NewStudio(mC4UHandle, gams::studio::versionToNumber());
    getMessages(messageIndex, buffer);

    ui->updateInfo->setText(mMessages.join("\n"));
}

void UpdateDialog::getMessages(int &messageIndex, char *buffer)
{
    for (int c=c4uMsgCount(mC4UHandle); messageIndex<c; ++messageIndex) {
        if (c4uGetMsg(mC4UHandle, messageIndex, buffer))
            mMessages.append(buffer);
    }
}

}
}
