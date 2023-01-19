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
#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QThread>

namespace gams {
namespace studio {
namespace support {

class UpdateChecker : public QThread
{
    Q_OBJECT

public:
    UpdateChecker(QObject *parent = nullptr);

    void run() override;

signals:
    void messageAvailable(const QString &message);
};

}
}
}

#endif // UPDATECHECKER_H
