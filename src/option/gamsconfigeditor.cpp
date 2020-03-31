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
    ui->GamsCfgTabWidget->setCurrentIndex(0);
    setFocusProxy(ui->GamsCfgTabWidget);

    ParamConfigEditor* configEditor =  static_cast<ParamConfigEditor*>(ui->GamsCfgTabWidget->widget(int(ConfigEditorType::commandLineParameter)));
    initEditorType(configEditor);

    connect(configEditor, &ParamConfigEditor::modificationChanged, this, &GamsConfigEditor::setModified, Qt::UniqueConnection);
}

GamsConfigEditor::~GamsConfigEditor()
{
    delete ui;
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
}

bool GamsConfigEditor::saveConfigFile(const QString &location)
{
    return saveAs(location);
}

void GamsConfigEditor::deSelectAll()
{
    // TODO (JP)
    QWidget* w = ui->GamsCfgTabWidget->currentWidget();
    if (toParamConfigEditor(w)) {
        toParamConfigEditor(w)->deSelectOptions();
    }
}

bool GamsConfigEditor::saveAs(const QString &location)
{
    setModified(false);

    // TODO (JP)
    return false;
}

}
}
}
