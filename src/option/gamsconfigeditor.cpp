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
#include "editors/abstractsystemlogger.h"
#include "editors/sysloglocator.h"

#include <QPushButton>
#include <QDebug>

namespace gams {
namespace studio {
namespace option {

GamsConfigEditor::GamsConfigEditor(QString fileName, QString optionFilePath,
                                   FileId id, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GamsConfigEditor),
    mFileId(id),
    mLocation(optionFilePath),
    mFileName(fileName),
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

void GamsConfigEditor::on_reloadGamsUserConfigFile(QTextCodec *codec)
{
    if (QTextCodec::codecForName("UTF-8") != codec)
        SysLogLocator::systemLog()->append(QString("Gams User Confiugration Editor supports only %1 encoding").arg(QString(codec->name())), LogMsgType::Info);
    else if (mFileHasChangedExtern)
             SysLogLocator::systemLog()->append(QString("Loading Gams User Configuration from %1").arg(mLocation), LogMsgType::Info);
    else
        return;

    if (mGuc->reloadGAMSUserConfigFile( mLocation )) {
        mParamConfigEditor->on_reloadGamsUserConfigFile( mGuc->readCommandLineParameters() );
        mEnvVarConfigEditor->on_reloadGamsUserConfigFile( mGuc->readEnvironmentVariables() );
    }

    setFileChangedExtern(false);
    setModified(false);
}

bool GamsConfigEditor::saveConfigFile(const QString &location)
{
    return saveAs(location);
}

void GamsConfigEditor::selectAll()
{
    if (ui->GamsCfgTabWidget->currentIndex()==int(ConfigEditorType::commandLineParameter))
        mParamConfigEditor->selectAll();
    else if (ui->GamsCfgTabWidget->currentIndex()==int(ConfigEditorType::environmentVariable))
             mEnvVarConfigEditor->selectAll();
}

void GamsConfigEditor::deSelectAll()
{
    if (ui->GamsCfgTabWidget->currentIndex()==int(ConfigEditorType::commandLineParameter))
        mParamConfigEditor->deSelect();
    else if (ui->GamsCfgTabWidget->currentIndex()==int(ConfigEditorType::environmentVariable))
        mEnvVarConfigEditor->deSelect();
}

void GamsConfigEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if ( (event->key() == Qt::Key_F1) &&
             (ui->GamsCfgTabWidget->currentIndex()==int(ConfigEditorType::commandLineParameter)) ) {
               mParamConfigEditor->on_actionShow_Option_Definition_triggered();
               event->accept(); return;
        } else if (event->key() == Qt::Key_R) {
                  if (ui->GamsCfgTabWidget->currentIndex()==int(ConfigEditorType::commandLineParameter)) {
                     mParamConfigEditor->on_actionResize_Columns_To_Contents_triggered();
                     event->accept(); return;
                  } else if (ui->GamsCfgTabWidget->currentIndex()==int(ConfigEditorType::environmentVariable)) {
                             mEnvVarConfigEditor->on_actionResize_Columns_To_Contents_triggered();
                             event->accept(); return;
                  }
        } else if (event->key() == Qt::Key_B) {
            if (ui->GamsCfgTabWidget->currentIndex()==int(ConfigEditorType::commandLineParameter)) {
               mParamConfigEditor->on_actionSelect_Current_Row_triggered();
               event->accept(); return;
            } else if (ui->GamsCfgTabWidget->currentIndex()==int(ConfigEditorType::environmentVariable)) {
                       mEnvVarConfigEditor->on_actionSelect_Current_Row_triggered();
                       event->accept(); return;
            }
        }
    } else if (event->modifiers() & Qt::ShiftModifier) {
              if ( (event->key() == Qt::Key_F1) &&
                   (ui->GamsCfgTabWidget->currentIndex()==int(ConfigEditorType::commandLineParameter)) ) {
                    mParamConfigEditor->on_actionShowRecurrence_triggered();
                    event->accept(); return;
              }
    }

    if (event->key() == Qt::Key_Return) {
        if (ui->GamsCfgTabWidget->currentIndex()==int(ConfigEditorType::commandLineParameter)) {
            mParamConfigEditor->on_actionAdd_This_Parameter_triggered();
            event->accept(); return;
        }
    }

    if (event->key() == Qt::Key_Delete) {
        if (ui->GamsCfgTabWidget->currentIndex()==int(ConfigEditorType::commandLineParameter)) {
            mParamConfigEditor->on_actionRemove_This_Parameter_triggered();
            event->accept(); return;
        }
    }

    QWidget::keyPressEvent(event);
}

void GamsConfigEditor::setFileChangedExtern(bool value)
{
    mFileHasChangedExtern = value;
}

bool GamsConfigEditor::saveAs(const QString &location)
{
    if (mParamConfigEditor->isModified()) {
       mGuc->updateCommandLineParameters( mParamConfigEditor->parameterConfigItems() );
       mParamConfigEditor->setModified(false);
    }
    if (mEnvVarConfigEditor->isModified()) {
       mGuc->updateEnvironmentVariables( mEnvVarConfigEditor->envVarConfigItems());
       mEnvVarConfigEditor->setModified(false);
    }
    mGuc->writeGamsUserConfigFile(location);
    setModified(false);

    return false;
}

}
}
}
