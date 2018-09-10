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
#include "sysloglocator.h"
#include <QDebug>

namespace gams {
namespace studio {

SystemLogEdit* SysLogLocator::mSysLog = nullptr;

void SysLogLocator::write(const QString &msg, LogMsgType type)
{
    if (!mSysLog) {
        qDebug() << "Warning: No systemLogEdit set in SysLogLocator";
        return;
    }
    mSysLog->appendLog(msg, type);
}

void SysLogLocator::provide(SystemLogEdit *syslogEdit)
{
    mSysLog = syslogEdit;
}



}
}
