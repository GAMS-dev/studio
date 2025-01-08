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
#include <QKeyEvent>
#include <QLineEdit>
#include "commandline.h"
#include "filterlineedit.h"

namespace gams {
namespace studio {
namespace option {

CommandLine::CommandLine(QWidget* parent) :
    QComboBox(parent), mParameterString(""), mCurrentIndex(-1)
{
    setDisabled(true);
    setEditable(true);
    setCurrentIndex(-1);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setInsertPolicy(QComboBox::InsertAtTop);
//    lineEdit()->setClearButtonEnabled(true);
    FilterLineEdit *ed = new FilterLineEdit(this);
    setLineEdit(ed);
    ed->hideOptions(FilterLineEdit::FilterLineEditFlags(FilterLineEdit::foExact | FilterLineEdit::foRegEx));
}

CommandLine::~CommandLine()
{
}

void CommandLine::validateChangedParameter(const QString &text)
{
    mParameterString = text.simplified();
    this->lineEdit()->setToolTip("");
    emit commandLineChanged(this->lineEdit(), text);
}

QString CommandLine::getParameterString() const
{
    return mParameterString;
}

void CommandLine::keyPressEvent(QKeyEvent *event)
{
    QComboBox::keyPressEvent(event);
    if ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return)) {
        emit parameterRunChanged();
    } else if (event->key() == Qt::Key_Escape) {
              emit parameterEditCancelled();
    }
}

void CommandLine::resetCurrentValue()
{
    mParameterString = "";
}

} // namespace option
} // namespace studio
} // namespace gams
