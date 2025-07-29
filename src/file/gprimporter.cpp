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
#include "gprimporter.h"

namespace gams {
namespace studio {

GprImporter::GprImporter()
{ }

void GprImporter::import(const QString &gprFile, ProjectRepo &repo)
{
    QFile gpr(gprFile);
    if (!gpr.exists()) {
        emit warning("GPR Import: Can't find file. " + gprFile);
        return;
    }
    if (!gpr.open(QIODeviceBase::ReadOnly)) {
        emit warning("GPR Import: Can't open file. " + gpr.errorString());
        return;
    }

    QStringList addFiles;
    QStringList openFiles;
    QHash<QString, FileId> fileIds;
    QHash<QString, QStringList> allRPs;

    QFileInfo gprFI(gprFile);
    PExProjectNode *project = nullptr;
    int lineNr = -1;
    QString group;
    QString rpGroup;
    QTextStream stream(&gpr);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        ++lineNr;

        // changed group and rpGroup
        if (line.startsWith("[")) {
            int ind = line.indexOf("]");
            if (ind < 0) {
                emit warning("GPR Import: Tag not closed in line " + QString::number(lineNr));
                return;
            }
            group = line.mid(1, ind - 1);
            rpGroup = group.startsWith("RP:") ? group.mid(3) : "";
            DEB() << "group " << group << "  rpGroup " << rpGroup;
            continue;
        }

        // process depending on group
        if (group.startsWith("MRUFILES")) {
            int ind = line.indexOf("=") + 1;
            if (ind > 0)
                addFiles << QDir::fromNativeSeparators(line.mid(ind));

        } else if (group.startsWith("OPENWINDOW_")) {
            if (!line.startsWith("FILE"))
                continue;
            int ind = line.indexOf("=") + 1;
            if (ind > 0) {
                QString file = QDir::fromNativeSeparators(line.mid(ind));
                openFiles.prepend(file); // prepend to open the first at last and make it the current
            }

        } else if (group.startsWith("RP:")) {
            int ind = line.indexOf("=") + 1;
            if (ind > 0) {
                if (!allRPs.contains(rpGroup))
                    allRPs.insert(rpGroup, QStringList());
                if (!allRPs.value(rpGroup).contains(line.mid(ind)))
                    allRPs[rpGroup].prepend(line.mid(ind));
            }
        }
    }
    gpr.close();

    // add opened before non-opened files. First file first, to ensure the mainfile will be the topmost
    for (const QString &file : openFiles)
        addFiles.prepend(file);

    // create project and add al files
    for (const QString &file : std::as_const(addFiles)) {
        QFileInfo fi(file);
        if (!project)
            project = repo.createProject(gprFI.completeBaseName() + "-import", gprFI.path(), "", onExist_AddNr);
        PExFileNode *node = repo.findOrCreateFileNode(fi.filePath(), project);
        if (node->file()->kind() == FileKind::Gms)
            fileIds.insert(fi.completeBaseName().toUpper(), node->file()->id());
    }

    // add command line parameters
    for (auto it = allRPs.constBegin(); it != allRPs.constEnd(); ++it) {
        FileId fId = fileIds.value(it.key());
        if (fId.isValid())
            project->setRunFileParameterHistory(fId, it.value());
        else
            emit warning("Import GPR: Couldn't add run parameters for " + it.key());
    }
    for (const QString &file : openFiles) {
        emit openFilePath(file, project /*, ogNone, true*/);
    }

}

} // namespace studio
} // namespace gams
