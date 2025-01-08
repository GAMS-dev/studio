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
#include "sysloglocator.h"
#include "defaultsystemlogger.h"

namespace gams {
namespace studio {

AbstractSystemLogger* SysLogLocator::mSysLog = nullptr;
AbstractSystemLogger* SysLogLocator::mNullLog = new DefaultSystemLogger;

AbstractSystemLogger* SysLogLocator::systemLog()
{
    if (!mSysLog) return mNullLog;
    return mSysLog;
}

void SysLogLocator::provide(AbstractSystemLogger *syslogEdit)
{
    mSysLog = syslogEdit;
}

}
}
