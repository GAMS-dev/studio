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
#include "projectedit.h"
#include "ui_projectedit.h"
#include "file/pexgroupnode.h"
#include "file/pexfilenode.h"
#include "file/filemeta.h"
#include "theme.h"

#include <QDir>
#include <QPushButton>
#include <QFont>
#include <QFileDialog>
#include <QComboBox>

namespace gams {
namespace studio {
namespace project {

const QString cNone("-none-");

ProjectData::ProjectData(PExProjectNode *project)
{
    mProject = project;
    QString projectFile = project->type() == PExProjectNode::tSmall ? "[internal]"
                                                                    : QDir::toNativeSeparators(project->fileName());
    mData.insert(file, projectFile);
    mData.insert(hasGsp, project->type() == PExProjectNode::tSmall ? "0" : "1");
    mData.insert(name, project->name());
    mData.insert(nameExt, project->nameExt());
    mData.insert(workDir, QDir::toNativeSeparators(project->workDir()));
    mData.insert(baseDir, QDir::toNativeSeparators(project->location()));
    if (mProject->runnableGms())
        mData.insert(mainFile, QDir::toNativeSeparators(mProject->runnableGms()->location()));
    else
        mData.insert(mainFile, cNone);
    if (mProject->parameterFile())
        mData.insert(pfFile, QDir::toNativeSeparators(mProject->parameterFile()->location()));
    else
        mData.insert(pfFile, cNone);
    connect(project, &PExProjectNode::changed, this, &ProjectData::projectChanged);
}

void ProjectData::setFieldData(Field field, const QString& value)
{
    bool change = !mData.contains(field) || mData.value(field) != value;
    mData.insert(field, value);
    if (change) emit changed(field);
}

QString ProjectData::fieldData(Field field)
{
    return mData.value(field);
}

bool ProjectData::save()
{
    QString path = QDir::fromNativeSeparators(mData.value(baseDir)).trimmed();
    if (path.compare(mProject->location(), FileType::fsCaseSense()))
        mProject->setLocation(path);
    path = QDir::fromNativeSeparators(mData.value(workDir)).trimmed();
    if (path.compare(mProject->workDir(), FileType::fsCaseSense()))
        mProject->setWorkDir(path);
    updateFile(FileKind::Gms, QDir::fromNativeSeparators(mData.value(mainFile)).trimmed());
    updateFile(FileKind::Pf, QDir::fromNativeSeparators(mData.value(pfFile)).trimmed());
    mProject->setNeedSave();
    return mProject->needSave();
}

void ProjectData::updateFile(FileKind kind, const QString &path)
{
    if (!mProject) return;
    FileMeta *meta = kind == FileKind::Gms ? mProject->runnableGms() : mProject->parameterFile();
    QString curPath = meta ? QDir::fromNativeSeparators(meta->location()) : QString();
    if (curPath.compare(path, FileType::fsCaseSense()) != 0) {
        PExFileNode *file = mProject->findFile(path);
        meta = file ? file->file() : nullptr;
        if (kind == FileKind::Gms)
            mProject->setRunnableGms(meta);
        else
            mProject->setParameterFile(meta);
    }
}

void ProjectData::projectChanged(const NodeId &id)
{
    if (mProject->id() != id) return;
    QString projectFile = mProject->type() == PExProjectNode::tSmall ? "[internal]"
                                                                     : QDir::toNativeSeparators(mProject->fileName());
    if (fieldData(ProjectData::file) != projectFile)
        setFieldData(ProjectData::file, projectFile);
    if (fieldData(ProjectData::hasGsp) != (mProject->type() == PExProjectNode::tSmall ? "0" : "1"))
        setFieldData(ProjectData::hasGsp, (mProject->type() == PExProjectNode::tSmall ? "0" : "1"));
    if (fieldData(ProjectData::name) != mProject->name())
        setFieldData(ProjectData::name, mProject->name());
    if (fieldData(ProjectData::nameExt) != mProject->nameExt())
        setFieldData(ProjectData::nameExt, mProject->nameExt());
    QString mainFile = mProject->runnableGms() ? QDir::toNativeSeparators(mProject->runnableGms()->location()) : "";
    if (fieldData(ProjectData::mainFile) != mainFile)
        setFieldData(ProjectData::mainFile, mainFile);
    QString pfFile = mProject->parameterFile() ? QDir::toNativeSeparators(mProject->parameterFile()->location()) : "";
    if (fieldData(ProjectData::pfFile) != pfFile)
        setFieldData(ProjectData::pfFile, pfFile);
    emit projectFilesChanged();
}


ProjectEdit::ProjectEdit(ProjectData *sharedData,  QWidget *parent) :
    AbstractView(parent),
    ui(new Ui::ProjectEdit)
{
    ui->setupUi(this);
    mBlockUpdate = true;
    mSharedData = sharedData;
    ui->edName->setEnabled(false);
    ui->edName->setToolTip("Name: the name of the project, this is always the filename");
    ui->cbMainFile->setToolTip("Main file: this file will be excuted with GAMS");
    ui->cbPfFile->setToolTip("Parameter file: this file contains the default parameters");
    ui->edProjectFile->setEnabled(false);
    ui->edProjectFile->setToolTip("Project file: this file contains all project information");
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    ui->edBaseDir->setToolTip("Base directory: used as base folder to represent the files");
    ui->edWorkDir->setToolTip("Working directory: used as working directory to run GAMS");
    ui->laName->setToolTip(ui->edName->toolTip());
    ui->laProjectFile->setToolTip(ui->edProjectFile->toolTip());
    ui->laMainGms->setToolTip(ui->cbMainFile->toolTip());
    ui->laPfFile->setToolTip(ui->cbPfFile->toolTip());
    ui->laBaseDir->setToolTip(ui->edBaseDir->toolTip());
    ui->laWorkDir->setToolTip(ui->edWorkDir->toolTip());
    ui->bBaseDir->setIcon(Theme::icon(":/%1/folder-open-bw"));
    ui->bBaseDir->setToolTip("Browse for base directory");
    ui->bWorkDir->setIcon(Theme::icon(":/%1/folder-open-bw"));
    ui->bBaseDir->setToolTip("Browse for working directory");
    adjustSize();
    connect(sharedData, &ProjectData::changed, this, &ProjectEdit::updateData);
    connect(sharedData, &ProjectData::projectFilesChanged, this, &ProjectEdit::updateComboboxEntries);
    updateData(ProjectData::all);
    mBlockUpdate = false;
    updateState();
}

ProjectEdit::~ProjectEdit()
{
    if (mSharedData->project()) mSharedData->project()->unlinkProjectEditFileMeta();
    delete ui;
}

ProjectData *ProjectEdit::sharedData() const
{
    return mSharedData;
}

QString ProjectEdit::tabName(NameModifier mod)
{
    return '[' + mSharedData->project()->name() + mSharedData->project()->nameExt()
            + (mod == NameModifier::editState && isModified() ? "]*" : "]");
}

bool ProjectEdit::isModified() const
{
    return mModified;
}

bool ProjectEdit::save()
{
    bool res = mSharedData->save();
    emit saveProjects();
    updateState();
    return res;
}

void ProjectEdit::on_edWorkDir_textChanged(const QString &text)
{
    if (mBlockUpdate) return;
    updateEditColor(ui->edWorkDir, text);
    if (text != mSharedData->fieldData(ProjectData::workDir))
        mSharedData->setFieldData(ProjectData::workDir, text);
    updateState();
}

void ProjectEdit::on_edBaseDir_textChanged(const QString &text)
{
    if (mBlockUpdate) return;
    updateEditColor(ui->edBaseDir, text);
    if (text != mSharedData->fieldData(ProjectData::baseDir))
        mSharedData->setFieldData(ProjectData::baseDir, text);
    updateState();
}

void ProjectEdit::updateEditColor(QLineEdit *edit, const QString &text)
{
    QDir dir(text.trimmed());
    if (!dir.exists()) {
        QPalette pal = qApp->palette();
        pal.setColor(QPalette::Text, Theme::color(Theme::Mark_errorFg));
        edit->setPalette(pal);
    } else {
        edit->setPalette(qApp->palette());
    }
}

void ProjectEdit::updateState()
{
    PExProjectNode *pro = mSharedData->project();
    bool isModified = pro->needSave();
    QString edPath = QDir::fromNativeSeparators(ui->edBaseDir->text()).trimmed();
    if (edPath.endsWith("/")) edPath = edPath.left(edPath.length()-1);
    if (edPath.compare(pro->location(), FileType::fsCaseSense()))
        isModified = true;
    edPath = QDir::fromNativeSeparators(ui->edWorkDir->text()).trimmed();
    if (edPath.endsWith("/")) edPath = edPath.left(edPath.length()-1);
    if (edPath.compare(pro->workDir(), FileType::fsCaseSense()))
        isModified = true;
    QString proPath = pro->runnableGms() ? pro->runnableGms()->location() : cNone;
    edPath = QDir::fromNativeSeparators(ui->cbMainFile->currentText());
    if (edPath.compare(proPath))
        isModified = true;
    proPath = pro->parameterFile() ? pro->parameterFile()->location() : cNone;
    edPath = QDir::fromNativeSeparators(ui->cbPfFile->currentText());
    if (edPath.compare(proPath))
        isModified = true;
    if (isModified != mModified) {
        mModified = isModified;
    }
    emit modificationChanged(mModified);
}

void ProjectEdit::on_bGspSwitch_clicked()
{
    if (mSharedData->project()->type() == PExProjectNode::tCommon) {
        mSharedData->project()->setHasGspFile(false);
        mSharedData->project()->setNeedSave(false);
        if (QFile::exists(mSharedData->project()->fileName()))
            QFile(mSharedData->project()->fileName()).remove();
    } else if (mSharedData->project()->type() == PExProjectNode::tSmall) {
        mSharedData->project()->setHasGspFile(true);
        mSharedData->project()->setNeedSave(true);
    }
    emit modificationChanged(true);
    emit saveProjects();
}

void ProjectEdit::on_bWorkDir_clicked()
{
    showDirDialog("Select Working Directory", ui->edWorkDir, mSharedData->project()->workDir());
}

void ProjectEdit::on_bBaseDir_clicked()
{
    showDirDialog("Select Base Directory", ui->edBaseDir, mSharedData->project()->location());
}

void ProjectEdit::updateData(gams::studio::project::ProjectData::Field field)
{
    if (field & (ProjectData::name | ProjectData::nameExt))
        mSharedData->project()->refreshProjectTabName();
    if ((field & ProjectData::file) && ui->edProjectFile->text() != mSharedData->fieldData(ProjectData::file))
        ui->edProjectFile->setText(mSharedData->fieldData(ProjectData::file));
    QString fullName = mSharedData->fieldData(ProjectData::name) + mSharedData->fieldData(ProjectData::nameExt);
    if (ui->edName->text() != fullName)
        ui->edName->setText(fullName);
    if ((field & ProjectData::workDir) && ui->edWorkDir->text() != mSharedData->fieldData(ProjectData::workDir))
        ui->edWorkDir->setText(mSharedData->fieldData(ProjectData::workDir));
    if ((field & ProjectData::baseDir) && ui->edBaseDir->text() != mSharedData->fieldData(ProjectData::baseDir))
        ui->edBaseDir->setText(mSharedData->fieldData(ProjectData::baseDir));
    updateComboboxEntries();
    if ((field & ProjectData::mainFile) && ui->cbMainFile->currentText() != mSharedData->fieldData(ProjectData::mainFile))
        ui->cbMainFile->setCurrentIndex(qMax(0, ui->cbMainFile->findText(mSharedData->fieldData(ProjectData::mainFile))));
    if ((field & ProjectData::pfFile) && ui->cbPfFile->currentText() != mSharedData->fieldData(ProjectData::pfFile))
        ui->cbPfFile->setCurrentIndex(qMax(0, ui->cbPfFile->findText(mSharedData->fieldData(ProjectData::pfFile))));
}

void ProjectEdit::updateComboboxEntries()
{
    bool hasGsp = mSharedData->fieldData(ProjectData::hasGsp) != "0";
    ui->bGspSwitch->setEnabled(hasGsp || mSharedData->project()->childCount());
    ui->bGspSwitch->setIcon(Theme::icon(QString(":/%1/") + (hasGsp ? "delete-all" : "new")));
    ui->bGspSwitch->setToolTip(hasGsp ? "Delete the project file (project data is stored in the Studio settings)"
                                      : "Create a project file (.gsp) for this project");
    // update combobox of main-file and pf-file
    QStringList mainFiles = files(FileKind::Gms);
    if (mainFiles.isEmpty())
        mainFiles.prepend(cNone);
    updateChanged(ui->cbMainFile, mainFiles);
    QStringList pfFiles =  files(FileKind::Pf);
    pfFiles.prepend(cNone);
    updateChanged(ui->cbPfFile, pfFiles);
}

QStringList ProjectEdit::files(FileKind kind)
{
    QStringList res;
    PExProjectNode *project = mSharedData->project();
    if (!project) return res;

    QVector<PExFileNode*> nodes = mSharedData->project()->listFiles();
    for (const PExFileNode *node : nodes) {
        if (node->file()->kind() == kind)
            res.append(QDir::toNativeSeparators(node->location()));
    }
    return res;
}

void ProjectEdit::updateChanged(QComboBox *comboBox, const QStringList &data)
{
    bool changed = comboBox->count() != data.count();
    if (!changed) {
        for (int i = 0; i < data.count(); ++i) {
            if (comboBox->itemText(i) != data.at(i)) {
                changed = true;
                break;
            }
        }
    }
    if (changed) {
        mBlockUpdate = true;
        QString current = comboBox->currentText();
        comboBox->clear();
        comboBox->addItems(data);
        comboBox->setCurrentIndex(qMax(0, comboBox->findText(current)));
        mBlockUpdate = false;
    }
}

void ProjectEdit::showDirDialog(const QString &title, QLineEdit *lineEdit, const QString &defaultDir)
{
    QString path = QDir::fromNativeSeparators(ui->edBaseDir->text()).trimmed();
    QDir dir(path);
    if (!dir.exists()) path = defaultDir;
    QFileDialog *dialog = new QFileDialog(this, title, path);
    dialog->setFileMode(QFileDialog::Directory);
    connect(dialog, &QFileDialog::accepted, this, [lineEdit, dialog]() {
        if (dialog->selectedFiles().count() == 1) {
            QDir dir(dialog->selectedFiles().at(0).trimmed());
            if (dir.exists()) lineEdit->setText(QDir::toNativeSeparators(dir.path()));
        }
    });
    connect(dialog, &QFileDialog::finished, this, [dialog]() { dialog->deleteLater(); });
    dialog->setModal(true);
    dialog->open();
}

void ProjectEdit::on_cbMainFile_currentIndexChanged(int index)
{
    if (mBlockUpdate) return;
    QString text = index ? ui->cbMainFile->currentText() : cNone;
    if (text != mSharedData->fieldData(ProjectData::mainFile))
        mSharedData->setFieldData(ProjectData::mainFile, text);
    updateState();
}


void ProjectEdit::on_cbPfFile_currentIndexChanged(int index)
{
    if (mBlockUpdate) return;
    QString text = index ? ui->cbPfFile->currentText() : cNone;
    if (text != mSharedData->fieldData(ProjectData::pfFile))
        mSharedData->setFieldData(ProjectData::pfFile, text);
    updateState();
}


} // namespace project
} // namespace studio
} // namespace gams
