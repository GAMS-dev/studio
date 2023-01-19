/*
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
 */
#include "updatechecker.h"
#include "checkforupdatewrapper.h"

namespace gams {
namespace studio {
namespace support {

UpdateChecker::UpdateChecker(QObject *parent)
    : QThread(parent)
{

}

void UpdateChecker::run()
{
    QString message;
    CheckForUpdateWrapper c4uWrapper;
    if (c4uWrapper.isValid()) {
        message = c4uWrapper.checkForUpdate();
    } else {
        message = c4uWrapper.message();
    }
    emit messageAvailable(message);
}

}
}
}
