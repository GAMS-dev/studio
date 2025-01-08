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
#ifndef GAMS_STUDIO_GDXVIEWER_EXPORTDRIVER_H
#define GAMS_STUDIO_GDXVIEWER_EXPORTDRIVER_H

#include "exportmodel.h"

#include <QString>
#include <process/connectprocess.h>


namespace gams {
namespace studio {
namespace gdxviewer {

class ExportDriver : public QObject
{
    Q_OBJECT

public:
    explicit ExportDriver(GdxViewer *gdxViewer, ExportModel *exportModel, QObject *parent = nullptr);
    ~ExportDriver();
    bool save(const QString& connectFile, const QString &output, bool applyFilters, const QString &eps, const QString &posInf, const QString &negInf, const QString &undef, const QString &na);
    void execute(const QString &connectFile, const QString &workingDirectory);
    void saveAndExecute(const QString &connectFile, const QString &output, const QString &workingDirectory, bool applyFilters, const QString &eps, const QString &posInf, const QString &negInf, const QString &undef, const QString &na);
    void cancelProcess(int waitMSec=0);

signals:
    void exportDone();

private:
    const QString PROJ_SUFFIX = "_proj_";
    const QString FILTER_SUFFIX = "_filter_";

    QScopedPointer<ConnectProcess> mProc;
    GdxViewer *mGdxViewer = nullptr;
    ExportModel *mExportModel = nullptr;
    QString generateInstructions(const QString &gdxFile, const QString &output, bool applyFilters, const QString &eps, const QString &posInf, const QString &negInf, const QString &undef, const QString &na);
    QString generateGdxReader(const QString &gdxFile);
    QString generateExcelWriter(const QString &excelFile, bool applyFilters, const QString &eps, const QString &posInf, const QString &negInf, const QString &undef, const QString &na);
    QString generateProjections(bool applyFilters);
    QString generateFilters();

    QString generateDomains(GdxSymbol *sym);
    QString generateDomainsNew(GdxSymbol *sym);

    bool hasActiveLabelFilter(GdxSymbol *sym);
    bool hasActiveValueFilter(GdxSymbol *sym);
    bool hasActiveFilter(GdxSymbol *sym);

};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_EXPORTDRIVER_H
