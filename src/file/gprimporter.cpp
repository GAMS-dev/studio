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
#include "ui_gprimporter.h"
#include <QPushButton>

namespace gams {
namespace studio {
namespace gpr {


GprImporter::GprImporter(QWidget *parent, ProjectRepo &repo) :
    QDialog(parent),
    ui(new Ui::GprImporter),
    mProjectRepo(repo)
{
    ui->setupUi(this);
    mMissText = ui->laMissCount->text();
    mCoverText = ui->laNewCoverCount->text();
    connect(ui->btOk, &QPushButton::clicked, this, &GprImporter::okClicked);
    connect(ui->btCancel, &QPushButton::clicked, this, &GprImporter::close);
}

bool GprImporter::import(const QString &gprFile)
{
    if (!readFile(gprFile)) return false;
    mProjectInfo = QFileInfo(gprFile);
    QString projectPath = mProjectInfo.absolutePath();
    if (projectPath.isEmpty() || !mAddFiles.size())
        return false;

    QString oriPath;
    QString newPath;
    if (int miss = needRelocatePaths(projectPath, oriPath, newPath)) {
        ui->laMissCount->setText(mMissText.arg(miss).arg(mAddFiles.count()));
        ui->edCurrentPath->setText(oriPath);
        ui->edRelocatedPath->setText(newPath);
        int missRelocate = checkPaths(oriPath, newPath);
        ui->laNewCoverCount->setText(mCoverText.arg(mAddFiles.count() - missRelocate).arg(mAddFiles.count()));
        show();
    } else {
        createProject();
    }
    return true;
}

void GprImporter::relocatePaths(const QString &oriPath, QString newPath)
{
    while (newPath.endsWith("/"))
        newPath = newPath.left(newPath.length() - 1);
    DEB() << "Relocate files to '" << newPath << "'";
    for (int i = 0; i < mAddFiles.size(); ++i) {
        if (mAddFiles.at(i).startsWith(oriPath)) {
            QString newFilename = newPath + mAddFiles.at(i).mid(oriPath.length());
            mAddFiles.replace(i, newFilename);
            DEB() << "  - " << mAddFiles.at(i);
        } else {
            DEB() << "  - Outside of path, file kept: " << mAddFiles.at(i);
        }
    }
    for (int i = 0; i < mOpenFiles.size(); ++i) {
        if (mOpenFiles.at(i).startsWith(oriPath))
            mOpenFiles.replace(i, newPath + mOpenFiles.at(i).mid(oriPath.length()));
    }

}

bool GprImporter::readFile(const QString &gprFile)
{
    QFile gpr(gprFile);
    if (!gpr.exists()) {
        emit warning("GPR Import: Can't find file. " + gprFile);
        return false;
    }
    if (!gpr.open(QIODeviceBase::ReadOnly)) {
        emit warning("GPR Import: Can't open file. " + gpr.errorString());
        return false;
    }

    mAddFiles.clear();
    mOpenFiles.clear();
    mFileIds.clear();
    mAllRPs.clear();

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
                return false;
            }
            group = line.mid(1, ind - 1);
            rpGroup = group.startsWith("RP:") ? group.mid(3) : "";
            continue;
        }

        // process depending on group
        if (group.startsWith("MRUFILES")) {
            int ind = line.indexOf("=") + 1;
            if (ind > 0)
                mAddFiles << line.mid(ind).replace('\\', '/');

        } else if (group.startsWith("OPENWINDOW_")) {
            if (!line.startsWith("FILE"))
                continue;
            int ind = line.indexOf("=") + 1;
            if (ind > 0) {
                QString file = line.mid(ind).replace('\\', '/');
                mOpenFiles.prepend(file); // prepend to open the first at last and make it the current
            }

        } else if (group.startsWith("RP:")) {
            int ind = line.indexOf("=") + 1;
            if (ind > 0) {
                if (!mAllRPs.contains(rpGroup)) {
                    mAllRPs.insert(rpGroup, QStringList());
                    DEB() << " Parameters for " << rpGroup;
                }
                if (!mAllRPs.value(rpGroup).contains(line.mid(ind))) {
                    mAllRPs[rpGroup].prepend(line.mid(ind));
                    DEB() << "   : '" << line.mid(ind) << "'";
                }
            }
        }
    }
    gpr.close();

    // add opened before non-opened files. First file first, to ensure the mainfile will be the topmost
    for (const QString &file : mOpenFiles)
        mAddFiles.prepend(file);

    return true;
}

int GprImporter::needRelocatePaths(const QString &projectPath, QString &oriPath, QString &newPath)
{
    oriPath = QString();
    newPath = QString();

    // Keep paths unchanged if first file exists in that place
    int miss = 0;
    for (const QString &file : mAddFiles)
        if (!QFile::exists(file)) ++miss;
    if (!miss) return miss;

    // Find the commonPath (also using recentPaths) that matches an existing file
    for (int i = 1; i < mAddFiles.size(); ++i) {
        QString file = mAddFiles.at(i);
        for (const QString &path : parentPaths(projectPath)) {
            for (const QString &tail : tailPaths(file)) {
                if (QFile::exists(path + tail)) {
                    oriPath = file.left(file.size() - tail.size());
                    newPath = path;
                    break;
                }
            }
            if (oriPath.size()) break;
        }
        if (oriPath.size()) break;
    }
    // shorten while folders are equal
    while (true) {
        QString tail = oriPath.mid(oriPath.lastIndexOf('/'));
        if (tail.length() > 1 && newPath.endsWith(tail, FileType::fsCaseSense())) {
            oriPath = oriPath.left(oriPath.length() - tail.length());
            newPath = newPath.left(newPath.length() - tail.length());
        } else {
            break;
        }
    }
    return miss;
}

int GprImporter::checkPaths(const QString &oriPath, QString newPath)
{
    while (newPath.endsWith("/"))
        newPath = newPath.left(newPath.length() - 1);
    if (!QFileInfo::exists(newPath)) return -1;
    int miss = 0;
    for (int i = 0; i < mAddFiles.size(); ++i) {
        if (mAddFiles.at(i).startsWith(oriPath)) {
            QString newFilename = newPath + mAddFiles.at(i).mid(oriPath.length());
            if (!QFile::exists(newFilename)) ++miss;
        }
    }
    return miss;
}


const QStringList GprImporter::tailPaths(const QString& str)
{
    QStringList res;
    QString path = QFileInfo(QDir::cleanPath(str)).filePath();
    QString prev = path;
    while((path = QFileInfo(path).path()).length() < prev.length()) {
        res << (str.mid(path.length()));
        prev = path;
    }
    return res;
}

const QStringList GprImporter::parentPaths(const QString str)
{
    QStringList res;
    QString path = QFileInfo(QDir::cleanPath(str)).filePath();
    res << path;
    while((path = QFileInfo(path).path()).length() < res.last().length())
        res << path;
    return res;
}

void GprImporter::createProject()
{
    // create project and add all files
    PExProjectNode *project = nullptr;
    for (const QString &file : std::as_const(mAddFiles)) {
        QFileInfo fi(file);
        if (!project)
            project = mProjectRepo.createProject(mProjectInfo.completeBaseName() + "-import"
                                                 , mProjectInfo.path(), "", onExist_AddNr);
        PExFileNode *node = mProjectRepo.findOrCreateFileNode(fi.filePath(), project);
        if (node->file()->kind() == FileKind::Gms)
            mFileIds.insert(fi.completeBaseName().toUpper(), node->file()->id());
    }

    // add command line parameters
    for (auto it = mAllRPs.constBegin(); it != mAllRPs.constEnd(); ++it) {
        FileId fId = mFileIds.value(it.key());
        if (fId.isValid() && project)
            project->setMainFileParameterHistory(fId, it.value());
        else
            emit warning("Import GPR: Couldn't add run parameters for " + it.key());
    }
    for (const QString &file : mOpenFiles) {
        if (QFile::exists(file))
            emit openFilePath(file, project /*, ogNone, true*/);
        else
            emit warning("File not found. Couldn't import " + file);
    }
}

void GprImporter::okClicked()
{
    relocatePaths(ui->edCurrentPath->text(), ui->edRelocatedPath->text());
    createProject();
    close();
}

void GprImporter::on_edRelocatedPath_textEdited(const QString &text)
{
    Q_UNUSED(text)
    int miss = checkPaths(ui->edCurrentPath->text(), ui->edRelocatedPath->text());
    ui->edRelocatedPath->setStyleSheet(miss < 0 ? "color:"+toColor(Theme::Normal_Red).name()+";" : "");
    if (miss < 0) miss = mAddFiles.count();
    ui->laNewCoverCount->setText(mCoverText.arg(mAddFiles.count() - miss).arg(mAddFiles.count()));
    ui->btOk->setEnabled(miss < mAddFiles.count());
}






} // namespace gpr
} // namespace studio
} // namespace gams
