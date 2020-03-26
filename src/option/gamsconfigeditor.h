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
#ifndef GAMSCONFIGEDITOR_H
#define GAMSCONFIGEDITOR_H

#include <QWidget>

#include "common.h"
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

class GamsConfigEditor : public QWidget
{
    Q_OBJECT

public:
    explicit GamsConfigEditor(QString fileName, QString optionFilePath,
                               FileId id, QTextCodec* mCodec, QWidget *parent = nullptr);
    ~GamsConfigEditor();

    FileId fileId() const;

    bool saveAs(const QString &location);

    inline static ParamConfigEditor* initEditorType(ParamConfigEditor * w) {
        if(w) w->setProperty("ConfigEditorType", int(ConfigEditorType::commandLineParameter));
        return w;
    }
    inline static ConfigEditorType configEditorType(QWidget* w) {
        QVariant v = w ? w->property("EditorType") : QVariant();
        return (v.isValid() ? static_cast<ConfigEditorType>(v.toInt()) : ConfigEditorType::undefined);
    }
    inline static ParamConfigEditor* toParamConfigEditor(QWidget* w) {
        return (configEditorType(w) == ConfigEditorType::commandLineParameter) ? static_cast<ParamConfigEditor*>(w) : nullptr;
    }

public slots:
    bool isModified() const;
    void setModified(bool modified);

    bool saveConfigFile(const QString &location);

    void deSelectAll();

private:
    Ui::GamsConfigEditor *ui;

    FileId mFileId;
    QString mLocation;
    QString mFileName;
    QTextCodec* mCodec;

    bool mModified;
};


}
}
}
#endif // GAMSCONFIGEDITOR_H
