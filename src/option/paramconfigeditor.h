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
#include <QMenu>

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

    bool isInFocus(QWidget* focusWidget) const;

signals:
    void modificationChanged(bool modifiedState);

public slots:
    void showParameterContextMenu(const QPoint &pos);
    void showDefinitionContextMenu(const QPoint &pos);

    void parameterItemCommitted(QWidget *editor);
    void on_selectRow(int logicalIndex);
    void deSelectOptions();

private slots:
    void addActions();
    void init();

    void findAndSelectionParameterFromDefinition();
    void selectAnOption();
    void insertOption();
    void deleteOption();
    void moveOptionUp();
    void moveOptionDown();
    void selectAllOptions();
    void resizeColumnsToContents();

    void addParameterFromDefinition(const QModelIndex &index);
    void showOptionDefinition(bool selectRow = true);
    void showOptionRecurrence();
    void copyDefinitionToClipboard(int column);

    void on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void on_newTableRowDropped(const QModelIndex &index);

    QList<int> getRecurrentOption(const QModelIndex &index);
    QString getParameterTableEntry(int row);

    void deleteParameter();

    bool isThereARow() const;
    bool isThereARowSelection() const;
    bool isEverySelectionARow() const;

private:
    Ui::ParamConfigEditor *ui;
    QMenu mContextMenu;

    OptionTokenizer* mOptionTokenizer;
    ConfigParamTableModel * mParameterTableModel;
    OptionCompleterDelegate* mOptionCompleter;
};

}
}
}
#endif // PARAMCONFIGEDITOR_H
