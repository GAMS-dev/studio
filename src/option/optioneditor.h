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
#ifndef OPTIONEDITOR_H
#define OPTIONEDITOR_H

#include <QtWidgets>

#include "commandlineoption.h"
#include "commandlinetokenizer.h"
#include "option.h"
#include "optionsortfilterproxymodel.h"
#include "optionparametermodel.h"

namespace gams {
namespace studio {

class OptionEditor : public QWidget
{
    Q_OBJECT
public:
    explicit OptionEditor(CommandLineOption* option, CommandLineTokenizer* tokenizer, QWidget *parent = nullptr);
    ~OptionEditor();

    void setupUi(QWidget* parent);

    QList<OptionItem> getCurrentListOfOptionItems();

signals:
    void optionRunWithParameterChanged(const QString &fileLocation, const QString &parameter);
    void optionTableModelChanged(const QString &commandLineStr);
    void commandLineOptionChanged(QLineEdit* lineEdit, const QString &commandLineStr);
    void commandLineOptionChanged(QLineEdit* lineEdit, const QList<OptionItem> &opionItems);

public slots:
//    void toggleOptionDefinition(bool checked);
    void updateTableModel(QLineEdit* lineEdit, const QString &commandLineStr);
    void updateCommandLineStr(const QString &commandLineStr);
    void updateCommandLineStr(const QList<OptionItem> &opionItems);
    void showOptionContextMenu(const QPoint &pos);
    void addOptionFromDefinition(const QModelIndex &index);

private:
    CommandLineOption* mCommandLineOption;
    CommandLineTokenizer* mTokenizer;

    QVBoxLayout *verticalLayout;
    QSplitter *splitter;
    QTableView *commandLineTableView;
    OptionParameterModel* optionParamModel;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *optionDefinition_VLayout;
    QLineEdit *searchLineEdit;
    QTreeView *optionDefinitionTreeView;
    QHBoxLayout *button_HLayout;

};

} // namespace studio
} // namespace gams

#endif // OPTIONEDITOR_H
