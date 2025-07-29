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
#ifndef GPRIMPORTER_H
#define GPRIMPORTER_H

#include <QStringList>
#include "projectrepo.h"
#include "pexgroupnode.h"

namespace gams {
namespace studio {

class GprImporter : public QObject
{
    Q_OBJECT
public:
    GprImporter();
    void import(const QString &gprFile, ProjectRepo &repo);

signals:
    void warning(const QString &message);
    void openFilePath(const QString &file, PExProjectNode *project);
};

} // namespace studio
} // namespace gams

#endif // GPRIMPORTER_H
