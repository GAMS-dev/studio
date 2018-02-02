#ifndef LINEEDITCOMPLETEEVENT_H
#define LINEEDITCOMPLETEEVENT_H

#include <QtWidgets>

namespace gams {
namespace studio {

class LineEditCompleteEvent : public QEvent
{
public:
    LineEditCompleteEvent(QLineEdit *lineEdit);

    void complete();

    static QEvent::Type type();

private:
    QLineEdit* mLineEdit;

    static QEvent::Type LineEditCompleteEventType;
};

} // namespace studio
} // namespace gams

#endif // LINEEDITCOMPLETEEVENT_H
