#include <QtWidgets>
#include "lineeditcompleteevent.h"

namespace gams {
namespace studio {

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
       int generatedType = QEvent::registerEventType();
       LineEditCompleteEventType = static_cast<QEvent::Type>(generatedType);
   }
   return LineEditCompleteEventType;
}

QEvent::Type LineEditCompleteEvent::LineEditCompleteEventType = QEvent::None;

} // namespace studio
} // namespace gams
