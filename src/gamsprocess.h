/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMSPROCESS_H
#define GAMSPROCESS_H

#include "abstractprocess.h"

namespace gams {
namespace studio {

class FileGroupContext;

class GAMSProcess
        : public AbstractProcess
{
    Q_OBJECT

public:
    GAMSProcess(QObject *parent = Q_NULLPTR);

    QString app() override;
    QString nativeAppPath() override;

    void setWorkingDir(const QString &workingDir);
    QString workingDir() const;

    void setContext(FileGroupContext *context);
    FileGroupContext* context() const;

    void execute() override;

    static QString aboutGAMS();

private:
    static const QString App;
    QString mWorkingDir;
    FileGroupContext *mContext = nullptr;
};

} // namespace studio
} // namespace gams

#endif // GAMSPROCESS_H
