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
#include <QLineEdit>
#include "commandlineoption.h"

namespace gams {
namespace studio {
namespace option {

CommandLineOption::CommandLineOption(QWidget* parent) :
    QComboBox(parent)
{
    setDisabled(true);
    setEditable(true);
    setCurrentIndex(-1);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setInsertPolicy(QComboBox::InsertAtTop);
    lineEdit()->setClearButtonEnabled(true);
    mOptionString = "";
    mCurrentIndex = -1;
}

CommandLineOption::~CommandLineOption()
{
}

void CommandLineOption::validateChangedOption(const QString &text)
{
    mOptionString = text.simplified();
    this->lineEdit()->setToolTip("");
    emit commandLineOptionChanged(this->lineEdit(), text);
}

QString CommandLineOption::getOptionString() const
{
    return mOptionString;
}

void CommandLineOption::keyPressEvent(QKeyEvent *event)
{
    QComboBox::keyPressEvent(event);
    if ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return)) {
        emit optionRunChanged();
    } else if (event->key() == Qt::Key_Escape) {
              emit optionEditCancelled();
    }
}

void CommandLineOption::resetCurrentValue()
{
    mOptionString = "";
}

} // namespace option
} // namespace studio
} // namespace gams
