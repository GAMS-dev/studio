/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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

#include "gamsuserconfig.h"
#include "optiontokenizer.h"
#include "configparamtablemodel.h"
#include "optioncompleterdelegate.h"

#include <QWidget>
#include <QMenu>
#include <QToolBar>
#include <QHeaderView>

namespace gams {
namespace studio {
namespace option {

namespace Ui {
class ParamConfigEditor;
}

class OptionTokenizer;

class ParamConfigEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ParamConfigEditor(const QList<ConfigItem *> &initParams, QWidget *parent = nullptr);
    ~ParamConfigEditor() override;

    bool isInFocus(QWidget* focusWidget) const;
    QList<QHeaderView*> headers();
    QString getSelectedParameterName(QWidget* widget) const;
    bool isModified() const;

    void selectSearchField() const;

signals:
    void modificationChanged(bool modifiedState);

public slots:
    void parameterItemCommitted(QWidget *editor);
    void on_reloadGamsUserConfigFile(const QList<gams::studio::option::ConfigItem *> &initParams);

    void selectAll();
    void deSelect();

    void setModified(bool modified);

    QList<gams::studio::option::ConfigItem *> parameterConfigItems();

private slots:
    void init(const QList<gams::studio::option::ConfigItem *> &initParams);

    void initActions();
    void updateActionsState(const QModelIndex &index);
    void updateActionsState();

    void currentTableSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void showParameterContextMenu(const QPoint &pos);
    void showDefinitionContextMenu(const QPoint &pos);

    void updateDefinitionActionsState(const QModelIndex &index);
    void findAndSelectionParameterFromDefinition();

    void selectAnOption();
    void deleteOption();

    void addParameterFromDefinition(const QModelIndex &index);
    void showOptionDefinition(bool selectRow = true);
    void copyDefinitionToClipboard(int column);

    void on_selectRow(int logicalIndex);
    void on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void on_newTableRowDropped(const QModelIndex &index);

    QList<int> getRecurrentOption(const QModelIndex &index);
    QString getParameterTableEntry(int row);

//    void deleteParameter();

    void on_actionInsert_triggered();
    void on_actionDelete_triggered();
    void on_actionMoveUp_triggered();
    void on_actionMoveDown_triggered();

    void on_actionSelect_Current_Row_triggered();
    void on_actionSelectAll_triggered();
    void on_actionShowRecurrence_triggered();
    void on_actionResize_Columns_To_Contents_triggered();
    void on_actionShow_Option_Definition_triggered();

    void on_actionAdd_This_Parameter_triggered();
    void on_actionRemove_This_Parameter_triggered();

private:
    bool isThereARow() const;
    bool isThereAnIndexSelection() const;
    bool isThereARowSelection() const;
    bool isEverySelectionARow() const;

private:
    friend class GamsConfigEditor;

    Ui::ParamConfigEditor *ui;
    QMenu mContextMenu;
    QToolBar* mToolBar;

    OptionTokenizer* mOptionTokenizer;
    ConfigParamTableModel* mParameterTableModel;
    OptionCompleterDelegate* mOptionCompleter;

    bool mModified;
};

}
}
}
#endif // PARAMCONFIGEDITOR_H
