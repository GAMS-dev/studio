#include "codecompleter.h"
#include "editors/codeedit.h"
#include "syntaxdata.h"
#include "logger.h"

#include <QSortFilterProxyModel>

namespace gams {
namespace studio {

CodeCompleter::CodeCompleter(CodeEdit *parent) :
    QListView(nullptr),
    mEdit(parent),
    mModel(new CodeCompleterModel(parent)),
    mFilter(new QSortFilterProxyModel(parent))
{
    mFilter->setSourceModel(mModel);
    setModel(mFilter);
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
    return QListView::event(event);
}

void CodeCompleter::showEvent(QShowEvent *event)
{
    QListView::showEvent(event);
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

CodeCompleterModel::CodeCompleterModel(QObject *parent): QAbstractListModel(parent)
{
    QList<QPair<QString, QString>> dat;
    QList<QPair<QString, QString>> src;

    src = syntax::SyntaxData::directives();
    QList<QPair<QString, QString>>::ConstIterator it = src.constBegin();
    while (it != src.constEnd()) {
        dat.append(QPair<QString,QString>('$'+it->first, it->second));
    }
}

int CodeCompleterModel::rowCount(const QModelIndex &parent) const
{
    return 0;
}

QVariant CodeCompleterModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

} // namespace studio
} // namespace gams
