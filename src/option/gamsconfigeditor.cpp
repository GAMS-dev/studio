/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "gamsconfigeditor.h"
#include "ui_gamsconfigeditor.h"

#include <QPushButton>
#include <QDebug>

namespace gams {
namespace studio {
namespace option {

GamsConfigEditor::GamsConfigEditor(QString fileName, QString optionFilePath,
                                   FileId id, QTextCodec*codec, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GamsConfigEditor),
    mFileId(id),
    mLocation(optionFilePath),
    mFileName(fileName),
    mCodec(codec),
    mModified(false)
{
    ui->setupUi(this);

    mGuc = new GamsUserConfig(mLocation);
    mParamConfigEditor = new ParamConfigEditor(mGuc->readCommandLineParameters(), this);
    ui->GamsCfgTabWidget->addTab( mParamConfigEditor, ConfigEditorName.at(int(ConfigEditorType::commandLineParameter)) );

    mEnvVarConfigEditor = new EnvVarConfigEditor(mGuc->readEnvironmentVariables(), this);
    ui->GamsCfgTabWidget->addTab( mEnvVarConfigEditor, ConfigEditorName.at(int(ConfigEditorType::environmentVariable)) );

    connect(mParamConfigEditor, &ParamConfigEditor::modificationChanged, this, &GamsConfigEditor::setModified, Qt::UniqueConnection);
    connect(mEnvVarConfigEditor, &EnvVarConfigEditor::modificationChanged, this, &GamsConfigEditor::setModified, Qt::UniqueConnection);

    ui->GamsCfgTabWidget->setCurrentIndex(int(ConfigEditorType::commandLineParameter));
    setFocusProxy(ui->GamsCfgTabWidget);
}

GamsConfigEditor::~GamsConfigEditor()
{
    delete ui;
    if (mGuc)  delete  mGuc;
}

FileId GamsConfigEditor::fileId() const
{
    return mFileId;
}

bool GamsConfigEditor::isModified() const
{
    return mModified;
}

void GamsConfigEditor::setModified(bool modified)
{
    mModified = modified;
    emit modificationChanged( mModified );

    ui->GamsCfgTabWidget->setTabText(int(ConfigEditorType::commandLineParameter),
                                     ConfigEditorName.at(int(ConfigEditorType::commandLineParameter)) + (mParamConfigEditor->isModified() ? "*" : ""));
    ui->GamsCfgTabWidget->setTabText(int(ConfigEditorType::environmentVariable),
                                     ConfigEditorName.at(int(ConfigEditorType::environmentVariable)) + (mEnvVarConfigEditor->isModified() ? "*" : ""));
}

bool GamsConfigEditor::saveConfigFile(const QString &location)
{
    return saveAs(location);
}

void GamsConfigEditor::deSelectAll()
{
    if (ui->GamsCfgTabWidget->currentIndex()==int(ConfigEditorType::commandLineParameter))
        mParamConfigEditor->deSelectOptions();
    else if (ui->GamsCfgTabWidget->currentIndex()==int(ConfigEditorType::environmentVariable))
            mEnvVarConfigEditor->deSelectOptions();
}

bool GamsConfigEditor::saveAs(const QString &location)
{
    if (mParamConfigEditor->isModified()) {
       mGuc->updateCommandLineParameters( mParamConfigEditor->parameterConfigItems() );
       mParamConfigEditor->setModified(false);
    }
    if (mEnvVarConfigEditor->isModified()) {
       mGuc->updateEnvironmentVariables( mEnvVarConfigEditor->envVarConfigItems());
       mParamConfigEditor->setModified(false);
    }
    mGuc->writeGamsUserConfigFile(location);
    setModified(false);

    return false;
}

}
}
}
