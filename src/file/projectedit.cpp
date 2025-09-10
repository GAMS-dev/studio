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
#include "file/projectrepo.h"
#include "theme.h"

#include <QDir>
#include <QPushButton>
#include <QFont>
#include <QFileDialog>
#include <QComboBox>

namespace gams {
namespace studio {
namespace project {

const QString cOn("1");
const QString cOff("0");
const QString cNone("-none-");
const QString cName("Name: the name of the project");
const QString cProFile("Project file: this file contains all project information."
                       "\n The filename is always the project name."
                       "\n The folder can be changed using 'Move Project File ...' in the Project Explorer.");

ProjectData::ProjectData(PExProjectNode *project)
{
    mProject = project;
    QString projectFile = mProject->type() == PExProjectNode::tSmall ? "[internal]"
                                                                     : QDir::toNativeSeparators(mProject->fileName());
    mDataOri.insert(file, projectFile);
    mDataOri.insert(hasGsp, mProject->type() == PExProjectNode::tSmall ? cOff : cOn);
    mDataOri.insert(dynMain, mProject->dynamicMainFile() ? cOn : cOff);
    mDataOri.insert(ownBase, mProject->ownBaseDir() ? cOn : cOff);
    mDataOri.insert(name, mProject->name());
    mDataOri.insert(nameExt, mProject->nameExt());
    QString path = mProject->workDir();
    if (path.endsWith("/")) path = path.left(path.length() - 1);
    mDataOri.insert(workDir, QDir::toNativeSeparators(mProject->workDir()));
    path = mProject->location();
    if (path.endsWith("/")) path = path.left(path.length() - 1);
    mDataOri.insert(baseDir, QDir::toNativeSeparators(mProject->location()));
    if (mProject->mainFile())
        mDataOri.insert(mainFile, QDir::toNativeSeparators(mProject->mainFile()->location()));
    else
        mDataOri.insert(mainFile, cNone);
    if (mProject->parameterFile())
        mDataOri.insert(pfFile, QDir::toNativeSeparators(mProject->parameterFile()->location()));
    else
        mDataOri.insert(pfFile, cNone);
    connect(mProject, &PExProjectNode::changed, this, &ProjectData::projectChanged);
}

void ProjectData::setFieldData(Field field, const QString& value)
{
    bool change = (mData.contains(field) ? mData.value(field) : mDataOri.value(field)) != value;
    if (value == mDataOri.value(field))
        mData.remove(field);
    else
        mData.insert(field, value);
    if (change) emit changed(field);
}

QString ProjectData::fieldData(Field field)
{
    if (mData.contains(field))
        return mData.value(field);
    return mDataOri.value(field);
}

bool ProjectData::isValidName(const QString &name)
{
    bool res = true;
    QFileInfo proFile(mProject->fileName());
    QString newProName = proFile.path() +'/'+ name +'.'+ proFile.suffix();
    for (const PExProjectNode *project : mProject->projectRepo()->projects()) {
        if (project == mProject) continue;
        if (project->fileName().compare(newProName, FileType::fsCaseSense()) == 0) {
            res = false;
            break;
        }
    }
    return res;
}

bool ProjectData::save()
{
    if (mData.isEmpty()) return true;
    if (mData.contains(baseDir)) {
        QString path = QDir::fromNativeSeparators(mData.value(baseDir)).trimmed();
        bool adapt = path.endsWith("/");
        if (adapt) path = path.left(path.length() - 1);
        if (path.compare(mProject->location(), FileType::fsCaseSense())) {
            mProject->setLocation(path);
            mDataOri.insert(baseDir, QDir::toNativeSeparators(path));
            mData.remove(baseDir);
            if (adapt) emit changed(baseDir);
        }
    }
    if (mData.contains(workDir)) {
        QString path = QDir::fromNativeSeparators(mData.value(workDir)).trimmed();
        bool adapt = path.endsWith("/");
        if (adapt) path = path.left(path.length() - 1);
        if (path.compare(mProject->workDir(), FileType::fsCaseSense())) {
            mProject->setWorkDir(path);
            mDataOri.insert(workDir, QDir::toNativeSeparators(path));
            mData.remove(workDir);
            if (adapt) emit changed(workDir);
        }
    }
    if (mData.contains(name)) {
        QFileInfo proFile(mProject->fileName());
        if (proFile.completeBaseName().compare(mData.value(name), FileType::fsCaseSense()) != 0) {
            QString newProFile = proFile.path() +"/"+ mData.value(name) +"."+ proFile.suffix();
            ProjectMoveOption opt = mProject->type() == PExProjectNode::tCommon ? pmMove : pmRename;
            mProject->projectRepo()->moveProject(mProject, newProFile, opt);
            mDataOri.insert(name, mData.value(name));
        }
    }
    if (mData.contains(mainFile)) {
        updateFile(FileKind::Gms, QDir::fromNativeSeparators(mData.value(mainFile)).trimmed());
        mDataOri.insert(mainFile, mData.value(mainFile));
    }
    if (mData.contains(dynMain)) {
        if (mProject->dynamicMainFile() != (mData.value(dynMain) == cOn)) {
            mProject->setDynamicMainFile(mData.value(dynMain) == cOn);
            mDataOri.insert(dynMain, mData.value(dynMain));
        }
    }
    if (mData.contains(ownBase)) {
        if (mProject->ownBaseDir() != (mData.value(ownBase) == cOn)) {
            mProject->setOwnBaseDir(mData.value(ownBase) == cOn);
            mDataOri.insert(ownBase, mData.value(ownBase));
        }
    }
    if (mData.contains(pfFile)) {
        updateFile(FileKind::Pf, QDir::fromNativeSeparators(mData.value(pfFile)).trimmed());
        mDataOri.insert(pfFile, mData.value(pfFile));
    }
    mData.clear();
    mProject->setNeedSave();
    return mProject->needSave();
}

void ProjectData::updateFile(FileKind kind, const QString &path)
{
    if (!mProject) return;
    FileMeta *meta = kind == FileKind::Gms ? mProject->mainFile() : mProject->parameterFile();
    QString curPath = meta ? QDir::fromNativeSeparators(meta->location()) : QString();
    if (curPath.compare(path, FileType::fsCaseSense()) != 0) {
        PExFileNode *file = mProject->findFile(path);
        meta = file ? file->file() : nullptr;
        if (kind == FileKind::Gms)
            mProject->setMainFile(meta);
        else
            mProject->setParameterFile(meta);
    }
}

void ProjectData::projectChanged(const NodeId &id)
{
    // Changes from original project (outside of the ProjectEdit)
    if (mProject->id() != id) return;
    QString projectFile = mProject->type() == PExProjectNode::tSmall ? "[internal]"
                                                                     : QDir::toNativeSeparators(mProject->fileName());
    if (mDataOri.value(ProjectData::file) != projectFile) {
        mDataOri.insert(ProjectData::file, projectFile);
        emit changed(ProjectData::file);
    }
    if (mDataOri.value(ProjectData::hasGsp) != (mProject->type() == PExProjectNode::tSmall ? cOff : cOn)) {
        mDataOri.insert(ProjectData::hasGsp, (mProject->type() == PExProjectNode::tSmall ? cOff : cOn));
        emit changed(ProjectData::hasGsp);
    }
    if (mDataOri.value(ProjectData::dynMain) != (mProject->dynamicMainFile() ? cOn : cOff)) {
        mDataOri.insert(ProjectData::dynMain, (mProject->dynamicMainFile() ? cOn : cOff));
        emit changed(ProjectData::dynMain);
    }
    if (mDataOri.value(ProjectData::ownBase) != (mProject->ownBaseDir() ? cOn : cOff)) {
        mDataOri.insert(ProjectData::ownBase, (mProject->ownBaseDir() ? cOn : cOff));
        emit changed(ProjectData::ownBase);
    }
    if (mDataOri.value(ProjectData::name) != mProject->name()) {
        mDataOri.insert(ProjectData::name, mProject->name());
        emit changed(ProjectData::name);
    }
    if (mDataOri.value(ProjectData::nameExt) != mProject->nameExt()) {
        mDataOri.insert(ProjectData::nameExt, mProject->nameExt());
        emit changed(ProjectData::nameExt);
    }
    QString mainFile = mProject->mainFile() ? QDir::toNativeSeparators(mProject->mainFile()->location()) : "";
    if (mDataOri.value(ProjectData::mainFile) != mainFile) {
        mDataOri.insert(ProjectData::mainFile, mainFile);
        emit changed(ProjectData::mainFile);
    }
    QString pfFile = mProject->parameterFile() ? QDir::toNativeSeparators(mProject->parameterFile()->location()) : "";
    if (mDataOri.value(ProjectData::pfFile) != pfFile) {
        mDataOri.insert(ProjectData::pfFile, pfFile);
        emit changed(ProjectData::pfFile);
    }
    const auto keys = mData.keys();
    for (auto key : keys) {
        if (mData.value(key) == mDataOri.value(key))
            mData.remove(key);
    }
    emit projectFilesChanged();
}


ProjectEdit::ProjectEdit(ProjectData *sharedData,  QWidget *parent) :
    AbstractView(parent),
    ui(new Ui::ProjectEdit)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    mBlockUpdate = true;
    mSharedData = sharedData;

    ui->edProjectFile->setEnabled(false);
    ui->edProjectFile->setToolTip(cProFile);
    ui->laProjectFile->setToolTip(ui->edProjectFile->toolTip());
    ui->edName->setToolTip(cName);
    ui->laName->setToolTip(ui->edName->toolTip());
    ui->edFullName->setEnabled(false);
    ui->edFullName->setToolTip("Full name: the full name of the project including extension to avoid name conflicts");

    ui->edWorkDir->setToolTip("Working directory: used as working directory to run GAMS");
    ui->laWorkDir->setToolTip(ui->edWorkDir->toolTip());
    ui->bWorkDir->setIcon(Theme::icon(":/%1/folder-open-bw"));
    ui->bWorkDir->setToolTip("Browse for working directory");

    ui->edBaseDir->setToolTip("Base directory: used as base folder to represent the files/n"
                              "Equals the Working directory by default. Check to assign a different path.");
    ui->cbOwnBaseDir->setToolTip(ui->edBaseDir->toolTip());
    ui->bBaseDir->setIcon(Theme::icon(":/%1/folder-open-bw"));
    ui->bBaseDir->setToolTip("Browse for base directory");
    ui->edBaseDir->setEnabled(sharedData->fieldData(ProjectData::ownBase) == cOn);
    ui->bBaseDir->setEnabled(sharedData->fieldData(ProjectData::ownBase) == cOn);

    ui->cbMainFile->setToolTip("Main file: this file will be excuted with GAMS");
    ui->laMainGms->setToolTip(ui->cbMainFile->toolTip());
    ui->cbPfFile->setToolTip("Parameter file: this file contains the default parameters");
    ui->laPfFile->setToolTip(ui->cbPfFile->toolTip());
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

void ProjectEdit::showEvent(QShowEvent *event)
{
    AbstractView::showEvent(event);
    if (mSharedData->project()->isEmpty() && mSharedData->project()->name().compare("newProject") == 0) {
        ui->edName->selectAll();
        QTimer::singleShot(0, this, [this]() {
            ui->edName->setFocus();
        });
    }
}

void ProjectEdit::on_edName_textChanged(const QString &text)
{
    if (mBlockUpdate) return;

    bool valid = isValidName(text.trimmed());
    if (!valid) {
        QPalette pal = qApp->palette();
        pal.setColor(QPalette::Text, Theme::color(Theme::Mark_errorFg));
        ui->edName->setPalette(pal);
    } else {
        ui->edName->setPalette(qApp->palette());
        if (text != mSharedData->fieldData(ProjectData::name))
            mSharedData->setFieldData(ProjectData::name, text);
    }
    updateState();
}

bool ProjectEdit::isValidName(const QString &name)
{
    return mSharedData->isValidName(name);
}

void ProjectEdit::on_edWorkDir_textChanged(const QString &text)
{
    if (mBlockUpdate) return;
    updateEditColor(ui->edWorkDir, text);
    if (text != mSharedData->fieldData(ProjectData::workDir))
        mSharedData->setFieldData(ProjectData::workDir, text);
    if (mSharedData->fieldData(ProjectData::ownBase) == cOff)
        ui->edBaseDir->setText(text);
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
    bool isModified = false;
    if (ui->edName->text().trimmed().compare(pro->name()))
        isModified = true;
    QString edPath = QDir::fromNativeSeparators(ui->edBaseDir->text()).trimmed();
    if (edPath.endsWith("/")) edPath = edPath.left(edPath.length()-1);
    if (edPath.compare(pro->location(), FileType::fsCaseSense()))
        isModified = true;
    edPath = QDir::fromNativeSeparators(ui->edWorkDir->text()).trimmed();
    if (edPath.endsWith("/")) edPath = edPath.left(edPath.length()-1);
    if (edPath.compare(pro->workDir(), FileType::fsCaseSense()))
        isModified = true;
    QString proPath = pro->mainFile() ? pro->mainFile()->location() : cNone;
    edPath = QDir::fromNativeSeparators(ui->cbMainFile->currentText());
    if (edPath.compare(proPath))
        isModified = true;
    if (ui->cbDynamicMainFile->isChecked() != pro->dynamicMainFile())
        isModified = true;
    if (ui->cbOwnBaseDir->isChecked() != pro->ownBaseDir())
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
    if (field & (ProjectData::name | ProjectData::nameExt)) {
        mSharedData->project()->refreshProjectTabName();
        QString fullName = mSharedData->fieldData(ProjectData::name) + mSharedData->fieldData(ProjectData::nameExt);
        if (ui->edFullName->text() != fullName)
            ui->edFullName->setText(fullName);
    }
    if ((field & ProjectData::file) && ui->edProjectFile->text() != mSharedData->fieldData(ProjectData::file))
        ui->edProjectFile->setText(mSharedData->fieldData(ProjectData::file));
    if ((field & ProjectData::name) && ui->edName->text() != mSharedData->fieldData(ProjectData::name))
        ui->edName->setText(mSharedData->fieldData(ProjectData::name));
    if ((field & ProjectData::dynMain) && ui->cbDynamicMainFile->isChecked() != (mSharedData->fieldData(ProjectData::dynMain) == cOn))
        ui->cbDynamicMainFile->setChecked(mSharedData->fieldData(ProjectData::dynMain) == cOn);
    if ((field & ProjectData::ownBase) && ui->cbOwnBaseDir->isChecked() != (mSharedData->fieldData(ProjectData::ownBase) == cOn)) {
        ui->cbOwnBaseDir->setChecked(mSharedData->fieldData(ProjectData::ownBase) == cOn);
        ui->edBaseDir->setEnabled(ui->cbOwnBaseDir->isChecked());
        ui->bBaseDir->setEnabled(ui->cbOwnBaseDir->isChecked());
    }
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
    bool hasGsp = mSharedData->fieldData(ProjectData::hasGsp) == cOn;
    ui->bGspSwitch->setEnabled(hasGsp || mSharedData->project()->childCount());
    ui->bGspSwitch->setIcon(Theme::icon(QString(":/%1/") + (hasGsp ? "delete-all" : "new")));
    ui->bGspSwitch->setToolTip(hasGsp ? "Delete the project file (project data is stored in the Studio settings)"
                                      : "Create a project file (.gsp) for this project");
    ui->edName->setToolTip(cName + (hasGsp ? "\n Renaming changes the project file name."  : ""));
    ui->edProjectFile->setToolTip(hasGsp ? cProFile : "Project data is stored in the Studio settings");
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

void ProjectEdit::on_cbDynamicMainFile_toggled(bool checked)
{
    Q_UNUSED(checked)
    if (ui->cbDynamicMainFile->isChecked() != (mSharedData->fieldData(ProjectData::dynMain) == cOn))
        mSharedData->setFieldData(ProjectData::dynMain, ui->cbDynamicMainFile->isChecked() ? cOn : cOff);
    updateState();
}

void ProjectEdit::on_cbOwnBaseDir_toggled(bool checked)
{
    Q_UNUSED(checked)
    if (ui->cbOwnBaseDir->isChecked() != (mSharedData->fieldData(ProjectData::ownBase) == cOn)) {
        mSharedData->setFieldData(ProjectData::ownBase, ui->cbOwnBaseDir->isChecked() ? cOn : cOff);
        if (!checked) {
            ui->edBaseDir->setText(ui->edWorkDir->text());
        }
        ui->edBaseDir->setEnabled(ui->cbOwnBaseDir->isChecked());
        ui->bBaseDir->setEnabled(ui->cbOwnBaseDir->isChecked());
    }
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
