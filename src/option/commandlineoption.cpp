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
#include <QKeyEvent>
#include "commandlineoption.h"

namespace gams {
namespace studio {

CommandLineOption::CommandLineOption(QWidget *parent) :
    CommandLineOption(true, parent)
{
}

CommandLineOption::CommandLineOption(bool validateFlag, QWidget* parent) :
    QComboBox(parent), mValidated(validateFlag)
{
    this->setDisabled(true);
    this->setEditable(true);
    this->setCurrentIndex(-1);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    this->setInsertPolicy(QComboBox::InsertAtTop);
    this->lineEdit()->setClearButtonEnabled(true);
    this->mCurrentContext = "";
    this->mCurrentOption = "";
    this->mCurrentIndex = -1;
}

CommandLineOption::~CommandLineOption()
{
}

void CommandLineOption::validateChangedOption(const QString &text)
{
    mCurrentOption = text.simplified();

    this->lineEdit()->setToolTip("");
//  also allow empty option to be validated
//    if (mCurrentOption.isEmpty())
//        return;

    if (mValidated)
       emit commandLineOptionChanged(this->lineEdit(), text);
}

QString CommandLineOption::getCurrentOption() const
{
    return mCurrentOption;
}

void CommandLineOption::keyPressEvent(QKeyEvent *event)
{
    QComboBox::keyPressEvent(event);
    if ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return)) {
        emit optionRunChanged();
    }
}

QString CommandLineOption::getCurrentContext() const
{
    return mCurrentContext;
}

void CommandLineOption::setCurrentContext(const QString &currentContext)
{
    mCurrentContext = currentContext;
}

void CommandLineOption::resetCurrentValue()
{
    mCurrentContext = "";
    mCurrentOption = "";
}

bool CommandLineOption::isValidated() const
{
    return mValidated;
}

void CommandLineOption::validated(bool value)
{
    mValidated = value;
}

} // namespace studio
} // namespace gams
