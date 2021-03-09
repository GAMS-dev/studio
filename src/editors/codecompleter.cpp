#include "codecompleter.h"
#include "ui_codecompleter.h"
#include "editors/codeedit.h"
#include "logger.h"

namespace gams {
namespace studio {

CodeCompleter::CodeCompleter(CodeEdit *parent) :
    QListWidget(nullptr),
    mEdit(parent)
{
    setWindowFlag(Qt::FramelessWindowHint);
}

CodeCompleter::~CodeCompleter()
{
}

bool CodeCompleter::event(QEvent *event)
{
    if (event->type() == QEvent::ActivationChange) {
        if (!this->isActiveWindow())
            hide();
    }
    return QListWidget::event(event);
}

void CodeCompleter::showEvent(QShowEvent *event)
{
    QListWidget::showEvent(event);
}

void CodeCompleter::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    hide();
}

void CodeCompleter::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        hide();
    else
        mEdit->keyPressEvent(event);
}

void CodeCompleter::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        ;
    else
        mEdit->keyPressEvent(event);
}

void CodeCompleter::focusOutEvent(QFocusEvent *event)
{
    QListView::focusOutEvent(event);
    hide();
}

} // namespace studio
} // namespace gams
