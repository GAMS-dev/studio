/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMSCONFIGEDITOR_H
#define GAMSCONFIGEDITOR_H

#include <QWidget>
#include <QtCore5Compat/QTextCodec>

#include "abstractview.h"
#include "common.h"
#include "gamsuserconfig.h"
#include "envvarconfigeditor.h"
#include "paramconfigeditor.h"

namespace gams {
namespace studio {

class MainWindow;

namespace option {

namespace Ui {
class GamsConfigEditor;
}

class OptionTokenizer;

enum  class ConfigEditorType {
    commandLineParameter = 0,
    environmentVariable = 1,
    solverConfiguration = 2,
    undefined = 3
};

static const QList<QString> ConfigEditorName = {
    "Command Line Parameters",
    "Environment Variables",
    "Solver Configuration",
    "Undefined",
};

class GamsConfigEditor : public AbstractView
{
    Q_OBJECT

public:
    explicit GamsConfigEditor(QString fileName, QString optionFilePath,
                               FileId id, QWidget *parent = nullptr);
    ~GamsConfigEditor() override;

    FileId fileId() const;

    bool saveAs(const QString &location);
    void setFileChangedExtern(bool value);

    void on_reloadGamsUserConfigFile();
    QString getSelectedParameterName(QWidget* widget) const;

    void zoomIn(int range = 1);
    void zoomOut(int range = 1);
    bool isModified() const;

signals:
    void modificationChanged(bool modifiedState);

public slots:
    void setModified(bool modified);

    bool saveConfigFile(const QString &location);

    void selectAll();
    void deSelectAll();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::GamsConfigEditor *ui;

    FileId mFileId;
    QString mLocation;
    QString mFileName;
    bool mModified;

    bool mFileHasChangedExtern = false;

    GamsUserConfig* mGuc;
    ParamConfigEditor* mParamConfigEditor;
    EnvVarConfigEditor* mEnvVarConfigEditor;

private:
    void zoomInF(qreal range);

};


}
}
}
#endif // GAMSCONFIGEDITOR_H
