/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include <QCompleter>
#include "lineeditcompleteevent.h"

namespace gams {
namespace studio {
namespace option {

LineEditCompleteEvent::LineEditCompleteEvent(QLineEdit *lineEdit) :
    QEvent(LineEditCompleteEvent::type()), mLineEdit(lineEdit)
{
}

void LineEditCompleteEvent::complete()
{
    mLineEdit->completer()->complete();
}

QEvent::Type LineEditCompleteEvent::type()
{
   if (LineEditCompleteEventType == QEvent::None) {
       const int generatedType = QEvent::registerEventType();
       LineEditCompleteEventType = static_cast<QEvent::Type>(generatedType);
   }
   return LineEditCompleteEventType;
}

QEvent::Type LineEditCompleteEvent::LineEditCompleteEventType = QEvent::None;

} // namespace option
} // namespace studio
} // namespace gams
