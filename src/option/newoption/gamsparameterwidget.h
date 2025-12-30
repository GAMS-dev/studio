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
#ifndef GAMSPARAMETERWIDGET_H
#define GAMSPARAMETERWIDGET_H

#include <QDockWidget>
#include <QWidget>

#include "option/optiontokenizer.h"
#include "option/newoption/gamsparameditor.h"

namespace gams {
namespace studio {
namespace option {
namespace newoption {

class GamsParamEditor;
namespace Ui {
class GamsParameterWidget;
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

class GamsParameterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GamsParameterWidget(QAction *aRun, QAction *aCompile, QAction *aRunWith, QAction *aCompileWith,
                             QAction* aRunDebug, QAction* aStepDebug, QList<QAction*> aActionFlags,
                             QAction* aRunNeos, QAction *aRunEngine, QAction* aInterrupt, QAction* aStop,
                             MainWindow *parent = nullptr);

    ~GamsParameterWidget();

    QString on_runAction(RunActionState state);
    void on_interruptAction();
    void on_stopAction();
    GamsParamEditor *dockChild();

    OptionTokenizer *getOptionTokenizer() const;
    bool isAParameterEditorFocused(QWidget* focusWidget) const;

    QString getSelectedParameterName(QWidget* widget) const;

    QString getCurrentCommandLineData() const;
    void focus();

    void runDefaultAction();

    void setEditorExtended(bool extended);
    bool isEditorExtended();
    QDockWidget* extendedEditor() const;

signals:
//    void parameterLoaded(const QString &location);
    void ParameterTableModelChanged(const QString &commandLineStr);
    void commandLineChanged(QLineEdit* lineEdit, const QList<gams::studio::option::OptionItem*> &optionItems);
    void optionsChanged(const QString &commandLineStr);

public slots:
    void updateParameterTableModel(QLineEdit* lineEdit, const QString &commandLineStr);
    void updateCommandLineStr(const QList<gams::studio::option::OptionItem*> &optionItems);

    void updateRunState(bool isRunnable, bool isRunning);
    void loadCommandLine(const QStringList &history);

    void selectSearchField();
    void deSelectParameters();

private:
    void setRunsActionGroup();
    void setInterruptActionGroup();
    void setRunActionsEnabled(bool enable);
    void setInterruptActionsEnabled(bool enable);

    void on_parameterTableModelChanged(const QString &commandLineStr);

    Ui::GamsParameterWidget *ui;

    QDockWidget *mExtendedEditor = nullptr;
    GamsParamEditor *mDockChild = nullptr;
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

    MainWindow* main;
};

} // namepsace newoption
} // namepsace option
} // namespace studio
} // namespace gams

#endif // GAMSPARAMETERWIDGET_H
