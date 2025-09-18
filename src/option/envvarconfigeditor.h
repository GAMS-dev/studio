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
#ifndef ENVVARCONFIGEDITOR_H
#define ENVVARCONFIGEDITOR_H

#include "envvartablemodel.h"
#include "envvarcfgcompleterdelegate.h"
#include "gamsuserconfig.h"

#include <QWidget>
#include <QMenu>
#include <QToolBar>
#include <QHeaderView>

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
    explicit EnvVarConfigEditor(const QList<EnvVarConfigItem *> &initItems, QWidget *parent = nullptr);
    ~EnvVarConfigEditor() override;
    QList<QHeaderView*> headers();
    bool isModified() const;
    bool isInFocus(QWidget* focusWidget) const;

signals:
    void modificationChanged(bool modifiedState);

public slots:
    void parameterItemCommitted(QWidget *editor);
    void on_reloadGamsUserConfigFile(const QList<gams::studio::option::EnvVarConfigItem *> &initItems);

    void selectAll();
    void deSelect();

    void setModified(bool modified);

    QList<gams::studio::option::EnvVarConfigItem *> envVarConfigItems();

private slots:
    void init(const QList<gams::studio::option::EnvVarConfigItem *> &initItems);

    void initActions();
    void updateActionsState(const QModelIndex &index);
    void updateActionsState();

    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void showContextMenu(const QPoint &pos);

    void on_selectRow(int logicalIndex);
    void on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

    void on_actionInsert_triggered();
    void on_actionDelete_triggered();
    void on_actionMoveUp_triggered();
    void on_actionMoveDown_triggered();
    void on_actionSelect_Current_Row_triggered();
    void on_actionSelectAll_triggered();
    void on_actionResize_Columns_To_Contents_triggered();

private:
    bool isThereARow() const;
    bool isThereAnIndexSelection() const;
    bool isThereARowSelection() const;
    bool isEverySelectionARow() const;

private:
    friend class GamsConfigEditor;

    Ui::EnvVarConfigEditor *ui;
    EnvVarTableModel* mEnvVarTableModel;
    EnvVarCfgCompleterDelegate* mCompleter;
    bool mModified;
    int mPrevFontHeight;

    QMenu mContextMenu;
    QToolBar* mToolBar;
};

}
}
}
#endif // ENVVARCONFIGEDITOR_H
