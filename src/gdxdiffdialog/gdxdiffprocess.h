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
#ifndef GDXDIFFPROCESS_H
#define GDXDIFFPROCESS_H

#include "process/abstractprocess.h"

namespace gams {
namespace studio {
namespace gdxdiffdialog {

class GdxDiffProcess final
        : public AbstractGamsProcess
{
    Q_OBJECT

public:
    GdxDiffProcess(QObject *parent = nullptr);

    void execute() override;

    QString diffFile() const;
    void stop(int waitMSec=0);

private slots:
    void appendSystemLog(const QString &text);

private:
    QString mDiffFile;
};

} // namespace gdxdiffdialog
} // namespace studio
} // namespace gams

#endif // GDXDIFFPROCESS_H
