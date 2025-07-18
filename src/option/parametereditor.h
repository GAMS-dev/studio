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
#ifndef PARAMETEREDITOR_H
#define PARAMETEREDITOR_H

#include <QDockWidget>
#include <QMenu>

#include "abstractview.h"
#include "option.h"
#include "optiontokenizer.h"
#include "gamsparametertablemodel.h"
#include "optioncompleterdelegate.h"

namespace gams {
namespace studio {

class MainWindow;

namespace option {

namespace Ui {
class ParameterEditor;
}

enum class RunActionState {
    Run,
    RunWithSelected,
    RunDebug,
    StepDebug,
    Compile,
    CompileWithSelected,
    RunNeos,
    RunEngine
};

class ParameterEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ParameterEditor(QAction *aRun, QAction *aCompile, QAction *aRunWith, QAction *aCompileWith,
                             QAction* aRunDebug, QAction* aStepDebug, QList<QAction*> aActionFlags,
                             QAction* aRunNeos, QAction *aRunEngine, QAction* aInterrupt, QAction* aStop,
                             MainWindow *parent = nullptr);
    ~ParameterEditor();

    QString on_runAction(RunActionState state);
    void on_interruptAction();
    void on_stopAction();
    AbstractView *dockChild();

    OptionTokenizer *getOptionTokenizer() const;
    bool isAParameterEditorFocused(QWidget* focusWidget) const;
    bool isAParameterTableFocused(QWidget* focusWidget) const;

    QString getSelectedParameterName(QWidget* widget) const;

    QString getCurrentCommandLineData() const;
    void focus();

    void runDefaultAction();

    void setEditorExtended(bool extended);
    bool isEditorExtended();
    QDockWidget* extendedEditor() const;

signals:
    void parameterLoaded(const QString &location);
    void ParameterTableModelChanged(const QString &commandLineStr);
    void commandLineChanged(QLineEdit* lineEdit, const QList<gams::studio::option::OptionItem> &optionItems);
    void optionsChanged(const QString &commandLineStr);

public slots:
    void updateParameterTableModel(QLineEdit* lineEdit, const QString &commandLineStr);
    void updateCommandLineStr(const QList<gams::studio::option::OptionItem> &optionItems);

    void showParameterContextMenu(const QPoint &pos);
    void showDefinitionContextMenu(const QPoint &pos);

    void updateRunState(bool isRunnable, bool isRunning);
    void addParameterFromDefinition(const QModelIndex &index);
    void loadCommandLine(const QStringList &history);

    void selectSearchField();
    void parameterItemCommitted(const QModelIndex &index);

    void deSelectParameters();

private slots:
    void findAndSelectionParameterFromDefinition();

    void showParameterDefinition(bool selectRow = true);
    void showParameterRecurrence();
    void deleteParameter();
    void deleteAllParameters();
    void insertParameter();

    void moveParameterUp();
    void moveParameterDown();

    void on_newTableRowDropped(const QModelIndex &index);
    void on_parameterTableNameChanged(const QString &from, const QString &to);
    void on_parameterValueChanged(const QModelIndex &index);
    void on_parameterTableModelChanged(const QString &commandLineStr);

    void resizeColumnsToContents();

private:
    void setRunsActionGroup();
    void setInterruptActionGroup();
    void setRunActionsEnabled(bool enable);
    void setInterruptActionsEnabled(bool enable);

    void addActions();

    QList<int> getRecurrentParameter(const QModelIndex &index);
    QString getParameterTableEntry(int row);

    Ui::ParameterEditor *ui;
    QDockWidget *mExtendedEditor = nullptr;
    AbstractView *mDockChild = nullptr;
    bool mHasSSL = false;

    QAction* actionRun;
    QAction* actionCompile;
    QAction* actionRunDebug;
    QAction* actionStepDebug;
    QAction *actionRunWithSelected;
    QAction *actionCompileWithSelected;
    QList<QAction*> actionFlags;
    QAction* actionRunNeos;
    QAction* actionRunEngine;

    QAction* actionInterrupt;
    QAction* actionStop;

    QMenu mContextMenu;
    OptionCompleterDelegate* mOptionCompleter;

    MainWindow* main;

    OptionTokenizer* mOptionTokenizer;
    GamsParameterTableModel* mParameterTableModel;
};

} // namespace option
} // namespace studio
} // namespace gams

#endif // PARAMETEREDITOR_H
