/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#ifndef OPTIONWIDGET_H
#define OPTIONWIDGET_H

#include <QWidget>

#include "option.h"
#include "commandlineoption.h"
#include "optiontokenizer.h"

namespace gams {
namespace studio {

class MainWindow;

namespace option {

namespace Ui {
class OptionWidget;
}

enum class RunActionState {
    Run,
    RunWithGDXCreation,
    Compile,
    CompileWithGDXCreation
};

class OptionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OptionWidget(QAction* aRun, QAction* aRunGDX, QAction* aCompile, QAction* aCompileGDX,
                          QAction* aInterrupt, QAction* aStop,
                          MainWindow *parent = nullptr);
    ~OptionWidget();

    QString on_runAction(RunActionState state);
    void on_interruptAction();
    void on_stopAction();

    void checkOptionDefinition(bool checked);
    bool isOptionDefinitionChecked();

    OptionTokenizer *getOptionTokenizer() const;
    bool isAnOptionWidgetFocused(QWidget* focusWidget);
    QString getSelectedOptionName(QWidget* widget) const;

    QString getCurrentCommandLineData() const;
    void focus();

signals:
    void optionEditorDisabled();
    void optionLoaded(const QString &location);
    void optionTableModelChanged(const QString &commandLineStr);
    void commandLineOptionChanged(QLineEdit* lineEdit, const QString &commandLineStr);
    void commandLineOptionChanged(QLineEdit* lineEdit, const QList<OptionItem> &optionItems);

public slots:
    void updateOptionTableModel(QLineEdit* lineEdit, const QString &commandLineStr);
    void updateCommandLineStr(const QString &commandLineStr);
    void updateCommandLineStr(const QList<OptionItem> &optionItems);
    void showOptionContextMenu(const QPoint &pos);
    void updateRunState(bool isRunnable, bool isRunning);
    void addOptionFromDefinition(const QModelIndex &index);
//    void loadCommandLineOption(const QString &location);
    void loadCommandLineOption(const QStringList &history);
    void disableOptionEditor();

private slots:
    void toggleOptionDefinition(bool checked);

private:
    void setRunsActionGroup(QAction *aRun, QAction *aRunGDX, QAction *aCompile, QAction *aCompileGDX);
    void setInterruptActionGroup(QAction* aInterrupt, QAction* aStop);
    void setRunActionsEnabled(bool enable);
    void setInterruptActionsEnabled(bool enable);

    Ui::OptionWidget *ui;

    QAction* actionRun;
    QAction* actionRun_with_GDX_Creation;
    QAction* actionCompile;
    QAction* actionCompile_with_GDX_Creation;

    QAction* actionInterrupt;
    QAction* actionStop;

    MainWindow* main;

//    CommandLineHistory* mCommandLineHistory;
    OptionTokenizer* mOptionTokenizer;
};

}
}
}

#endif // OPTIONWIDGET_H
