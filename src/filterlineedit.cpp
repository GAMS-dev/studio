#include "filterlineedit.h"
#include "theme.h"
#include "logger.h"
#include <QAction>
#include <QToolButton>
#include <QPainter>
#include <QHBoxLayout>
#include <QSpacerItem>

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
    mExact = exact;
    if (mExactButton)
        mExactButton->setIcon(mExact ? Theme::instance()->icon(":/img/tlock")
                                     : Theme::instance()->icon(":/img/tlock-open"));
    updateRegExp();
}

void FilterLineEdit::init()
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::MinimumExpanding));
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(0);

    mClearButton = new QToolButton(this);
    mClearButton->setIconSize(QSize(height()/2,height()/2));
    mClearButton->setContentsMargins(0,0,0,0);
    mClearButton->setStyleSheet("border:none;background:yellow;");
    mClearButton->setIcon(Theme::instance()->icon(":/img/tremove"));
    mClearButton->setCursor(Qt::PointingHandCursor);
    connect(mClearButton, &QToolButton::clicked, this, [this](){ clear(); });
    lay->addWidget(mClearButton);

    mExactButton = new QToolButton(this);
    mClearButton->setIconSize(QSize(height()/2,height()/2));
    mExactButton->setContentsMargins(0,0,0,0);
    mExactButton->setStyleSheet("border:none;background:cyan;");
    mExactButton->setIcon(Theme::instance()->icon(":/img/tlock-open"));
    mExactButton->setCursor(Qt::PointingHandCursor);
    connect(mExactButton, &QToolButton::clicked, this, [this](){ setExactMatch(!mExact); });
    lay->addWidget(mExactButton);

    setLayout(lay);

    connect(this, &FilterLineEdit::textChanged, this, [this](){ updateRegExp(); });
    setExactMatch(false);
}

void FilterLineEdit::updateRegExp()
{
    mClearButton->setVisible(!text().isEmpty());
    QString filter = text().isEmpty() ? QString() : mExact ? '^'+QRegExp::escape(text())+'$'
                                                           : QRegExp::escape(text());
    filter.replace("\\*", ".*");
    filter.replace("\\?", ".");
    mRegExp = QRegExp(filter);
    mRegExp.setCaseSensitivity(Qt::CaseInsensitive);
    emit regExpChanged(mRegExp);
}

} // namespace studio
} // namespace gams
