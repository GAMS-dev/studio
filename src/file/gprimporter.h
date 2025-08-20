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

#include <QDialog>
#include <QStringList>
#include "projectrepo.h"
#include "pexgroupnode.h"

namespace gams {
namespace studio {
namespace gpr {

namespace Ui {
class GprImporter;
}

class GprImporter : public QDialog
{
    Q_OBJECT
public:
    GprImporter(QWidget *parent, ProjectRepo &repo);
    bool import(const QString &gprFile);

signals:
    void warning(const QString &message);
    void openFilePath(const QString &file, PExProjectNode *project);

private slots:
    void relocatePaths(const QString &oriPath, QString newPath);
    void okClicked();
    void on_edRelocatedPath_textEdited(const QString &text);

private:
    bool readFile(const QString &gprFile);
    int needRelocatePaths(const QString &projectPath, QString &oriPath, QString &newPath);
    int checkPaths(const QString &oriPath, QString newPath);
    const QStringList tailPaths(const QString &path);
    const QStringList parentPaths(const QString str);
    void createProject();

private:
    Ui::GprImporter *ui;
    ProjectRepo &mProjectRepo;
    QFileInfo mProjectInfo;
    QString mMissText;
    QString mCoverText;
    QStringList mAddFiles;
    QStringList mOpenFiles;
    QHash<QString, FileId> mFileIds;
    QHash<QString, QStringList> mAllRPs;

};

} // namespace gpr
} // namespace studio
} // namespace gams

#endif // GPRIMPORTER_H
