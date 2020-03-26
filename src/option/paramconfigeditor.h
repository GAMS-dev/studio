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
#ifndef PARAMCONFIGEDITOR_H
#define PARAMCONFIGEDITOR_H

#include "optiontokenizer.h"
#include "configparamtablemodel.h"
#include "optioncompleterdelegate.h"

#include <QWidget>

namespace gams {
namespace studio {

class MainWindow;

namespace option {

namespace Ui {
class ParamConfigEditor;
}

class OptionTokenizer;

class ParamConfigEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ParamConfigEditor(QWidget *parent = nullptr);
    ~ParamConfigEditor();

public slots:
    void parameterItemCommitted(QWidget *editor);

private slots:
    void findAndSelectionParameterFromDefinition();

//    void on_newTableRowDropped(const QModelIndex &index);
//    void on_parameterTableNameChanged(const QString &from, const QString &to);
//    void on_parameterValueChanged(const QModelIndex &index);
//    void on_parameterTableModelChanged(const QString &commandLineStr);

private:
    Ui::ParamConfigEditor *ui;

    OptionTokenizer* mOptionTokenizer;
    ConfigParamTableModel * mParameterTableModel;
    OptionCompleterDelegate* mOptionCompleter;
};

}
}
}
#endif // PARAMCONFIGEDITOR_H
