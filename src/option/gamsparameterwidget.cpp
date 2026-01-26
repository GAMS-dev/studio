/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "checkmenu.h"
#include "mainwindow.h"
#include "settings.h"

#include "option/gamsparameterwidget.h"
#include "ui_gamsparameterwidget.h"
#include "option/gamsparameditor.h"

namespace gams {
namespace studio {
namespace option {

GamsParameterWidget::GamsParameterWidget(QAction *aRun, QAction *aCompile, QAction *aRunWith, QAction *aCompileWith,
                                         QAction *aRunDebug, QAction *aStepDebug, QList<QAction *> aActionFlags,
                                         QAction *aRunNeos, QAction *aRunEngine, QAction *aInterrupt, QAction *aStop,
                                         MainWindow *parent):
    QWidget(parent), ui(new Ui::GamsParameterWidget), actionRun(aRun), actionCompile(aCompile),
    actionRunDebug(aRunDebug), actionStepDebug(aStepDebug), actionRunWithSelected(aRunWith),
    actionCompileWithSelected(aCompileWith), actionFlags(aActionFlags), actionRunNeos(aRunNeos),
    actionRunEngine(aRunEngine), actionInterrupt(aInterrupt), actionStop(aStop), main(parent)
{
    ui->setupUi(this);

    mOptionTokenizer = new OptionTokenizer(GamsOptDefFile);

    setRunsActionGroup();
    setInterruptActionGroup();
    setFocusPolicy(Qt::StrongFocus);

    mExtendedEditor = new QDockWidget("GAMS Parameters", this);
    mExtendedEditor->setObjectName("gamsArguments");

    mDockChild = new GamsParamEditor(FileKind::None, ui->gamsParameterCommandLine->lineEdit()->text(), mOptionTokenizer, mExtendedEditor);
    mExtendedEditor->setWidget(mDockChild);
    mDockChild->layout()->setContentsMargins(0,0,0,0);

    mExtendedEditor->setFeatures(QDockWidget::NoDockWidgetFeatures);
    mExtendedEditor->setTitleBarWidget(new QWidget(this));
    main->addDockWidget(Qt::TopDockWidgetArea, mExtendedEditor);
    connect(mExtendedEditor, &QDockWidget::visibilityChanged, main, &MainWindow::setExtendedEditorVisibility, Qt::UniqueConnection);
    mExtendedEditor->setVisible(false);

#ifdef __APPLE__
    ui->verticalLayout->setContentsMargins(2,2,2,0);
#else
    ui->verticalLayout->setContentsMargins(2,0,2,2);
#endif

    connect(ui->gamsParameterCommandLine, &CommandLine::parameterRunChanged, main, &MainWindow::parameterRunChanged, Qt::UniqueConnection);
    connect(ui->gamsParameterCommandLine, &QComboBox::editTextChanged, ui->gamsParameterCommandLine, &CommandLine::validateChangedParameter, Qt::UniqueConnection);
    connect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged, mOptionTokenizer, &OptionTokenizer::formatTextLineEdit, Qt::UniqueConnection);
    connect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged, this, &GamsParameterWidget::updateParameterTableModel, Qt::UniqueConnection );
    connect(ui->gamsParameterCommandLine, &CommandLine::parameterEditCancelled, this, &CommandLine::clearFocus, Qt::UniqueConnection);

    connect(mDockChild->optionModel(), &GamsParamTableModel::optionModelChanged, this, &GamsParameterWidget::updateCommandLineStr,  Qt::UniqueConnection);
    connect(this, &GamsParameterWidget::commandLineChanged, mOptionTokenizer, &OptionTokenizer::formatItemLineEdit, Qt::UniqueConnection);
    connect(this, &GamsParameterWidget::ParameterTableModelChanged, this, &GamsParameterWidget::on_parameterTableModelChanged, Qt::UniqueConnection);

}

GamsParameterWidget::~GamsParameterWidget()
{
    delete ui;
    if (mOptionTokenizer)
        delete mOptionTokenizer;
    if (mDockChild)
        delete mDockChild;
    if (mExtendedEditor)
        delete mExtendedEditor;
}

QString GamsParameterWidget::on_runAction(RunActionState state)
{
    Q_ASSERT( mOptionTokenizer );

    Settings::settings()->setInt(skLastRun, static_cast<int>(state));
    QString commandLineStr =  ui->gamsParameterCommandLine->getParameterString();

    if (!commandLineStr.endsWith(" "))
        commandLineStr.append(" ");

    bool gdxParam = false;
    bool actParam = false;
    bool refParam = false;
    bool profParam = false;
    const auto items = mOptionTokenizer->tokenize(commandLineStr);
    for (const option::OptionItem* item : items) {
        if (QString::compare(item->key, "gdx", Qt::CaseInsensitive) == 0)
            gdxParam = true;
        if (QString::compare(item->key, "rf", Qt::CaseInsensitive) == 0)
            refParam = true;
        if ((QString::compare(item->key, "action", Qt::CaseInsensitive) == 0) ||
            (QString::compare(item->key, "a", Qt::CaseInsensitive) == 0))
            actParam = true;
        if ((QString::compare(item->key, "profile", Qt::CaseInsensitive) == 0))
            profParam = true;
    }

    if (state == RunActionState::RunWithSelected) {
        if (!gdxParam && actionFlags.size() && actionFlags.at(0)->isChecked()) commandLineStr.prepend("GDX=default ");
        if (!refParam && actionFlags.size() > 1 && actionFlags.at(1)->isChecked()) commandLineStr.prepend("RF=default ");
        if (!profParam && actionFlags.size() > 2 && actionFlags.at(2)->isChecked()) commandLineStr.prepend("Profile=300 ");
        ui->gamsRunToolButton->setDefaultAction( actionRunWithSelected );

    } else if (state == RunActionState::RunDebug) {
        ui->gamsRunToolButton->setDefaultAction( actionRunDebug );

    } else if (state == RunActionState::StepDebug) {
        ui->gamsRunToolButton->setDefaultAction( actionStepDebug );

    } else if (state == RunActionState::Compile) {
        if (!actParam) commandLineStr.prepend("ACTION=C ");
        ui->gamsRunToolButton->setDefaultAction( actionCompile );

    } else if (state == RunActionState::CompileWithSelected) {
        if (!gdxParam && actionFlags.size() && actionFlags.at(0)->isChecked()) commandLineStr.prepend("GDX=default ");
        if (!refParam && actionFlags.size() > 1 && actionFlags.at(1)->isChecked()) commandLineStr.prepend("RF=default ");
        if (!actParam) commandLineStr.prepend("ACTION=C ");
        ui->gamsRunToolButton->setDefaultAction( actionCompileWithSelected );

    } else if (state == RunActionState::RunNeos) {
        ui->gamsRunToolButton->setDefaultAction( actionRunNeos );

    } else if (state == RunActionState::RunEngine) {
        ui->gamsRunToolButton->setDefaultAction( actionRunEngine );

    } else {
        ui->gamsRunToolButton->setDefaultAction( actionRun );
    }

    return commandLineStr.simplified();
}

void GamsParameterWidget::on_interruptAction()
{
    ui->gamsInterruptToolButton->setDefaultAction( actionInterrupt );
}

void GamsParameterWidget::on_stopAction()
{
   ui->gamsInterruptToolButton->setDefaultAction( actionStop );
}

GamsParamEditor *GamsParameterWidget::dockChild()
{
    return mDockChild;
}

OptionTokenizer *GamsParameterWidget::getOptionTokenizer() const
{
    return mOptionTokenizer;
}

bool GamsParameterWidget::isAParameterEditorFocused(QWidget *focusWidget) const
{
    return (focusWidget==ui->gamsParameterCommandLine || mDockChild->isInFocus(focusWidget) );
}

QString GamsParameterWidget::getSelectedParameterName(QWidget *widget) const
{
    return mDockChild->getSelectedParameterName(widget);
}

QString GamsParameterWidget::getCurrentCommandLineData() const
{
    return ui->gamsParameterCommandLine->getParameterString();
}

void GamsParameterWidget::focus()
{
    if (isEditorExtended())
        mDockChild->focus();
    else
        ui->gamsParameterCommandLine->setFocus(Qt::ShortcutFocusReason);
}

void GamsParameterWidget::runDefaultAction()
{
    ui->gamsRunToolButton->defaultAction()->trigger();
}

void GamsParameterWidget::setEditorExtended(bool extended)
{
    if (extended) {
        disconnect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged,
                   this, &GamsParameterWidget::updateParameterTableModel );

        mDockChild->clearDefintionSelection();
        mDockChild->on_actionResize_Columns_To_Contents_triggered();
        emit ParameterTableModelChanged(ui->gamsParameterCommandLine->currentText());
    } else  {
        connect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged,
                this, &GamsParameterWidget::updateParameterTableModel, Qt::UniqueConnection );
    }
    mExtendedEditor->setVisible(extended);
    mDockChild->setEditorExtended(extended);
    main->updateRunState();
    ui->gamsParameterCommandLine->setEnabled(!extended);
}

bool GamsParameterWidget::isEditorExtended()
{
    return mExtendedEditor->isVisible();
}

QDockWidget *GamsParameterWidget::extendedEditor() const
{
    return mExtendedEditor;
}

void GamsParameterWidget::updateParameterTableModel(QLineEdit *lineEdit, const QString &commandLineStr)
{
    Q_UNUSED(lineEdit)
    if (mExtendedEditor->isHidden()) return;

    emit ParameterTableModelChanged(commandLineStr);
}

void GamsParameterWidget::updateRunState(bool isRunnable, bool isRunning)
{
    const bool activate = isRunnable && !isRunning;
    setRunActionsEnabled(activate);
    setInterruptActionsEnabled(isRunnable && isRunning);

    mDockChild->setEnabled(activate);
    ui->gamsParameterCommandLine->setEnabled(activate && !isEditorExtended());
}

void GamsParameterWidget::loadCommandLine(const QStringList &history)
{
    // disconnect
    disconnect(ui->gamsParameterCommandLine, &QComboBox::editTextChanged,
               ui->gamsParameterCommandLine, &CommandLine::validateChangedParameter);
    disconnect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged,
               mOptionTokenizer, &OptionTokenizer::formatTextLineEdit);
    disconnect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged,
               this, &GamsParameterWidget::updateParameterTableModel );
    disconnect(mDockChild->optionModel(), &GamsParamTableModel::optionModelChanged,
               this, &GamsParameterWidget::updateCommandLineStr);
    disconnect(this, &GamsParameterWidget::commandLineChanged,
               mOptionTokenizer, &OptionTokenizer::formatItemLineEdit);
    disconnect(ui->gamsParameterCommandLine, &QComboBox::currentTextChanged,
               this, &GamsParameterWidget::optionsChanged);

    mDockChild->clearDefintionSelection();
    ui->gamsParameterCommandLine->clear();
    ui->gamsParameterCommandLine->resetCurrentValue();

    for (const QString &str: history) {
        ui->gamsParameterCommandLine->insertItem(0, str );
    }
    if (!history.isEmpty()) {
        ui->gamsParameterCommandLine->validateChangedParameter( history.last() );
    }

    connect(ui->gamsParameterCommandLine, &QComboBox::editTextChanged,
            ui->gamsParameterCommandLine, &CommandLine::validateChangedParameter, Qt::UniqueConnection);
    connect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged,
            mOptionTokenizer, &OptionTokenizer::formatTextLineEdit, Qt::UniqueConnection);
    connect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged,
            this, &GamsParameterWidget::updateParameterTableModel, Qt::UniqueConnection);
    connect(mDockChild->optionModel(), &GamsParamTableModel::optionModelChanged,
            this, &GamsParameterWidget::updateCommandLineStr,  Qt::UniqueConnection);
    connect(this, &GamsParameterWidget::commandLineChanged,
            mOptionTokenizer, &OptionTokenizer::formatItemLineEdit, Qt::UniqueConnection);
    connect(ui->gamsParameterCommandLine, &QComboBox::currentTextChanged, this, &GamsParameterWidget::optionsChanged);

    if (history.isEmpty()) {
        mDockChild->clearDefintionSelection();
    }
    ui->gamsParameterCommandLine->setCurrentIndex(0);
    emit ui->gamsParameterCommandLine->commandLineChanged(ui->gamsParameterCommandLine->lineEdit(), ui->gamsParameterCommandLine->currentText());
    emit optionsChanged(ui->gamsParameterCommandLine->currentText());
}

void GamsParameterWidget::selectSearchField()
{
    mDockChild->selectSearchField();
}

void GamsParameterWidget::deSelectParameters()
{
    if (mDockChild->hasFocus() && mDockChild->hasSelection()) {
        mDockChild->deSelectParameters();
    } else {
        this->focusNextChild();
    }
}

void GamsParameterWidget::updateCommandLineStr(const QList<OptionItem*> &optionItems)
{
    if (mDockChild->isHidden())
        return;

    emit commandLineChanged(ui->gamsParameterCommandLine->lineEdit(), optionItems);
}

void GamsParameterWidget::setRunsActionGroup()
{
    mHasSSL = QSslSocket::supportsSsl();

    CheckParentMenu* runMenu = new CheckParentMenu(this);
    runMenu->addCheckActions(1, actionFlags);
    QList<QAction*> actionFlags2 = actionFlags;
    actionFlags2.removeLast();
    runMenu->addCheckActions(2, actionFlags2);

    runMenu->addAction(actionRun);
    runMenu->addAction(actionCompile);
    runMenu->addAction(actionRunDebug);
    runMenu->addAction(actionStepDebug);
    runMenu->addSeparator();

    runMenu->addAction(actionRunWithSelected);
    actionRunWithSelected->setData(1);
    runMenu->addAction(actionCompileWithSelected);
    actionCompileWithSelected->setData(2);

    runMenu->addSeparator();
    runMenu->addAction(actionRunNeos);
    runMenu->addAction(actionRunEngine);

    actionRun->setShortcutVisibleInContextMenu(true);
    actionCompile->setShortcutVisibleInContextMenu(true);
    actionRunWithSelected->setShortcutVisibleInContextMenu(true);
    actionCompileWithSelected->setShortcutVisibleInContextMenu(true);
    actionRunDebug->setShortcutVisibleInContextMenu(true);
    actionStepDebug->setShortcutVisibleInContextMenu(true);
    actionRunNeos->setShortcutVisibleInContextMenu(true);
    actionRunEngine->setShortcutVisibleInContextMenu(true);

    ui->gamsRunToolButton->setMenu(runMenu);
    ui->gamsRunToolButton->setDefaultAction(actionRun);
    RunActionState state = RunActionState(Settings::settings()->toInt(skLastRun));
    on_runAction(state);

    ui->gamsRunToolButton->raise();
}

void GamsParameterWidget::setInterruptActionGroup()
{
    actionInterrupt->setShortcutVisibleInContextMenu(true);
    actionStop->setShortcutVisibleInContextMenu(true);

    QMenu* interruptMenu = new QMenu(this);
    interruptMenu->addAction(actionInterrupt);
    interruptMenu->addAction(actionStop);

    ui->gamsInterruptToolButton->setMenu(interruptMenu);
    ui->gamsInterruptToolButton->setDefaultAction(actionInterrupt);

    ui->gamsInterruptToolButton->setFocusPolicy(Qt::StrongFocus);
}

void GamsParameterWidget::setRunActionsEnabled(bool enable)
{
    actionRun->setEnabled(enable);
    actionCompile->setEnabled(enable);
    actionRunWithSelected->setEnabled(enable);
    actionCompileWithSelected->setEnabled(enable);
    actionRunDebug->setEnabled(enable);
    actionStepDebug->setEnabled(enable);
    actionRunNeos->setEnabled(enable && mHasSSL);
    actionRunEngine->setEnabled(enable);
    ui->gamsRunToolButton->menu()->setEnabled(enable);
}

void GamsParameterWidget::setInterruptActionsEnabled(bool enable)
{
    actionInterrupt->setEnabled(enable);
    actionStop->setEnabled(enable);
    ui->gamsInterruptToolButton->menu()->setEnabled(enable);
}

void GamsParameterWidget::on_parameterTableModelChanged(const QString &commandLineStr)
{
    disconnect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged, mOptionTokenizer, &OptionTokenizer::formatTextLineEdit);
    disconnect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged, this, &GamsParameterWidget::updateParameterTableModel);
    disconnect(mDockChild->optionModel(), &GamsParamTableModel::optionModelChanged, this, &GamsParameterWidget::updateCommandLineStr);

    mDockChild->on_ParameterTableModelChanged(commandLineStr);

    connect(mDockChild->optionModel(), &GamsParamTableModel::optionModelChanged, this, &GamsParameterWidget::updateCommandLineStr,  Qt::UniqueConnection);
    connect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged, mOptionTokenizer, &OptionTokenizer::formatTextLineEdit, Qt::UniqueConnection);
    connect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged, this, &GamsParameterWidget::updateParameterTableModel, Qt::UniqueConnection );
}

} // namepsace option
} // namespace studio
} // namespace gams
