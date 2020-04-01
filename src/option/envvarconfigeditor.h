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
#ifndef ENVVARCONFIGEDITOR_H
#define ENVVARCONFIGEDITOR_H

#include "gamsuserconfig.h"

#include <QWidget>

namespace gams {
namespace studio {
namespace option {

namespace Ui {
class EnvVarConfigEditor;
}

class EnvVarConfigEditor : public QWidget
{
    Q_OBJECT

public:
    explicit EnvVarConfigEditor(const QList<EnvVarConfigItem *> &initParams, QWidget *parent = nullptr);
    ~EnvVarConfigEditor();

signals:
    void modificationChanged(bool modifiedState);

public slots:
    void setModified(bool modified);
    bool isModified() const;

private slots:
    void init(const QList<EnvVarConfigItem *> &initParams);

private:
    Ui::EnvVarConfigEditor *ui;
    bool mModified;
};

}
}
}
#endif // ENVVARCONFIGEDITOR_H
