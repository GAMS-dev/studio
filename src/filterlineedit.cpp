#include "filterlineedit.h"
#include "theme.h"
#include <QAction>
#include <QToolButton>
#include <QPainter>

namespace gams {
namespace studio {

FilterLineEdit::FilterLineEdit(QWidget *parent): QLineEdit(parent)
{
    init();
}

FilterLineEdit::FilterLineEdit(const QString &contents, QWidget *parent): QLineEdit(contents, parent)
{
    init();
}

const QRegExp &FilterLineEdit::regExp() const
{
    return mRegExp;
}

void FilterLineEdit::setExactMatch(bool exact)
{
    mExactAction->setVisible(exact);
    mLooseAction->setVisible(!exact);
    updateRegExp();
}

void FilterLineEdit::init()
{
    QToolButton *button;

    mRegExp.setCaseSensitivity(Qt::CaseInsensitive);
    mExactAction = addAction(Theme::instance()->icon(":/img/tlock", false), QLineEdit::TrailingPosition);
    mExactAction->setToolTip("<p style=\"white-space: nowrap;\"><b>Exact match</b> - accepts only entries that match exactly the term<br><i>Click to switch to <b>Contain</b></i></p>");
    button = qobject_cast<QToolButton *>(mExactAction->associatedWidgets().at(1));
    if (button) button->setCursor(Qt::PointingHandCursor);
    connect(mExactAction, &QAction::triggered, this, [this](){ setExactMatch(false); });

    mLooseAction = addAction(Theme::instance()->icon(":/img/tlock-open", false), QLineEdit::TrailingPosition);
    mLooseAction->setToolTip("<p style=\"white-space: nowrap;\"><b>Contain</b> - accepts all entries that contain the term<br><i>Click to switch to <b>Exact match</b></i></p>");
    button = qobject_cast<QToolButton *>(mLooseAction->associatedWidgets().at(1));
    if (button) button->setCursor(Qt::PointingHandCursor);
    connect(mLooseAction, &QAction::triggered, this, [this](){ setExactMatch(true); });

    setClearButtonEnabled(false);
    mClearAction = addAction(Theme::instance()->icon(":/img/tremove", false), QLineEdit::TrailingPosition);
    mClearAction->setVisible(false);
    button = qobject_cast<QToolButton *>(mClearAction->associatedWidgets().at(1));
    if (button) button->setCursor(Qt::PointingHandCursor);
    connect(mClearAction, &QAction::triggered, this, [this](){ clear(); });

    connect(this, &FilterLineEdit::textChanged, this, [this](){ updateRegExp(); });
    setExactMatch(false);
}

void FilterLineEdit::updateRegExp()
{
    mClearAction->setVisible(!text().isEmpty());
    QString filter = text().isEmpty() ? QString() : mExactAction->isVisible() ? '^'+QRegExp::escape(text())+'$'
                                                                              : QRegExp::escape(text());
    filter.replace("\\*", ".*");
    filter.replace("\\?", ".");
    mRegExp = QRegExp(filter);
    emit regExpChanged(mRegExp);
}

} // namespace studio
} // namespace gams
