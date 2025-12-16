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
#ifndef GAMSPARAMEDITOR_H
#define GAMSPARAMEDITOR_H

#include <QDockWidget>
#include <QWidget>
#include <QMenu>
#include <QToolBar>

#include "abstractview.h"
#include "ui_gamsparameterwidget.h"

namespace gams {
namespace studio {

class MainWindow;

namespace option {
namespace newoption {

namespace Ui {
class ParameterEditor;
class OptionWidget;
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
    AbstractView *dockChild();

private:
    void setRunsActionGroup();
    void setInterruptActionGroup();
    void setRunActionsEnabled(bool enable);
    void setInterruptActionsEnabled(bool enable);

    Ui::GamsParamEditor *ui;
    QToolBar* mToolBar;

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

    MainWindow* main;

};

} // namepsace newoption
} // namepsace option
} // namespace studio
} // namespace gams

#endif // GAMSPARAMEDITOR_H
