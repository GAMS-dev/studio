/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#ifndef SOLVERCONFIGINFO_H
#define SOLVERCONFIGINFO_H

#include <QMap>
#include <QString>

struct cfgRec;
typedef struct cfgRec *cfgHandle_t;

namespace gams {
namespace studio {
namespace support {

class SolverConfigInfo
{
public:
    SolverConfigInfo();
    ~SolverConfigInfo();

    int solvers() const;
    int modelTypes() const;

    int solverId(const QString &name) const;

    QString solverName(int id) const;
    QMap<int, QString> solverNames();

    bool isSolverHidden(const QString &solverName);

    QString solverOptDefFileName(const QString &solverName) const;
    QMap<QString, QString> solverOptDefFileNames();

    QMap<int, int> solverIndices();

    QString modelTypeName(int modelTypeId) const;
    QString defaultSolverFormodelTypeName(int modelTypeId) const;
    QMap<int, QString> modelTypeNames();

    bool solverCapability(int solver, int modelType) const;
    QStringList solversForModelType(int modelType) const;

    QString solverCodes(int solverId) const;

private:
    static int errorCallback(int count, const char *message);

private:
    cfgHandle_t mCFG;
};

}
}
}

#endif // SOLVERCONFIGINFO_H
