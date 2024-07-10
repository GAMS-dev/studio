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
#include "filterlineedit.h"
#include "theme.h"
#include "logger.h"
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

const QRegularExpression &FilterLineEdit::regExp() const
{
    return mRegExp;
}

void FilterLineEdit::setOptionState(FilterLineEditFlag option, int state)
{
    nextButtonState(button(option), state);
}

void FilterLineEdit::setKeyColumn(int column)
{
    if (mKeyColumn != column) {
        mKeyColumn = column;
        mAllColButton->setVisible(true);
        emit columnScopeChanged();
    }
}

void FilterLineEdit::hideOptions(FilterLineEditFlags options)
{
    if (options.testFlag(foClear)) mCanClear = false;
    if (options.testFlag(foExact)) mExactButton->setVisible(false);
    if (options.testFlag(foRegEx)) mRegExButton->setVisible(false);
    if (options.testFlag(foColumn)) mAllColButton->setVisible(false);
    updateRegExp();
}

int FilterLineEdit::effectiveKeyColumn()
{
    return buttonState(mAllColButton) ? -1 : mKeyColumn;
}

bool FilterLineEdit::exactMatch()
{
    return buttonState(mExactButton);
}

bool FilterLineEdit::isRegEx()
{
    return buttonState(mRegExButton);
}

void FilterLineEdit::resizeEvent(QResizeEvent *event)
{
    QLineEdit::resizeEvent(event);
    updateTextMargins();
}

void FilterLineEdit::init()
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::MinimumExpanding));
    lay->setContentsMargins(0,0,3,0);
    lay->setSpacing(0);

    mClearButton = createButton(QStringList() << ":/img/tremove", QStringList() << "Clear");
    connect(mClearButton, &QAbstractButton::clicked, this, [this](){ clear(); });
    lay->addWidget(mClearButton);
    mClearButton->setVisible(false);

    mExactButton = createButton(QStringList() << ":/img/tpart" << ":/img/twhole",
                                QStringList() << "Allow substring matches" << "Only allow exact matches");
    connect(mExactButton, &QAbstractButton::clicked, this, [this](){ nextButtonState(mExactButton); });
    lay->addWidget(mExactButton);

    mRegExButton = createButton(QStringList() << ":/img/trex-off" << ":/img/trex-on",
                                QStringList() << "Wildcard matching" << "Regular expression matching");
    connect(mRegExButton, &QAbstractButton::clicked, this, [this](){ nextButtonState(mRegExButton); });
    lay->addWidget(mRegExButton);

    mAllColButton = createButton(QStringList() << ":/img/tcol-one" << ":/img/tcol-all",
                                 QStringList() << "Search in key column only" << "Search in all columns");
    connect(mAllColButton, &QAbstractButton::clicked, this, [this]() {
        nextButtonState(mAllColButton);
        emit columnScopeChanged();
    });
    lay->addWidget(mAllColButton);
    mAllColButton->setVisible(false);

    setLayout(lay);
    connect(this, &FilterLineEdit::textChanged, this, [this](){ updateRegExp(); });
    updateRegExp();
    updateTextMargins();
}

void FilterLineEdit::updateRegExp()
{
    if (mClearButton->isVisible() == text().isEmpty()) {
        mClearButton->setVisible(!text().isEmpty());
        updateTextMargins();
    }

    QString filter;
    if (!text().isEmpty()) {
        if (buttonState(mRegExButton))
            filter = buttonState(mExactButton) ? "^"+text()+"$" : text();
        else
            filter = QRegularExpression::wildcardToRegularExpression(buttonState(mExactButton) ? text()
                                                                                               : "*"+text()+"*");
    }
    mRegExp = QRegularExpression(filter, QRegularExpression::CaseInsensitiveOption);
    if (mRegExp.isValid())
        emit regExpChanged(mRegExp);
}

QAbstractButton *FilterLineEdit::createButton(const QStringList &iconPaths, const QStringList &toolTips)
{
    if (iconPaths.isEmpty()) {
        DEB() << "At least one icon needed";
        return nullptr;
    }
    if (iconPaths.size() != toolTips.size()) {
        DEB() << "Count of icons and tool tips differ";
        return nullptr;
    }
    MiniButton *button = new MiniButton(this);
    button->setIconSize(QSize(height()/2,height()/2));
    button->setContentsMargins(0,0,0,0);
    button->setFlat(true);
    button->setCursor(Qt::PointingHandCursor);
    button->setIcon(Theme::instance()->icon(iconPaths.at(0)));
    button->setToolTip(toolTips.at(0));
    button->setProperty("icons", iconPaths);
    button->setProperty("tips", toolTips);
    return button;
}

int FilterLineEdit::nextButtonState(QAbstractButton *button, int forceState)
{
    if (!button) return -1;
    bool ok;
    int state = button->property("state").toInt(&ok);
    if (!ok) state = 0;
    QStringList icons = button->property("icons").toStringList();
    QStringList tips = button->property("tips").toStringList();
    if (icons.isEmpty()) return -1;
    if (forceState < 0)
        state = (state+1) % icons.size();
    else
        state = forceState % icons.size();
    button->setIcon(Theme::instance()->icon(icons.at(state)));
    button->setToolTip("<p style=\"white-space: nowrap;\">"+tips.at(state)+"</p>");
    if (icons.size() > 1) button->setProperty("state", state);
    updateRegExp();
    return state;
}

int FilterLineEdit::buttonState(QAbstractButton *button)
{
    bool ok;
    int state = button->property("state").toInt(&ok);
    if (!ok) state = 0;
    return state;
}

void FilterLineEdit::updateTextMargins()
{
    int visiButtons = 2 + (mClearButton->isVisible() ? 1 : 0) + (mAllColButton->isVisible() ? 1 : 0);
    int rightMargin = mExactButton->sizeHint().width() * visiButtons;
    if (textMargins().right() != rightMargin)
        setTextMargins(0, 0, mExactButton->sizeHint().width() * visiButtons, 0);
}

QAbstractButton *FilterLineEdit::button(FilterLineEditFlag option)
{
    switch (option) {
    case foClear: return mClearButton;
    case foExact: return mExactButton;
    case foRegEx: return mRegExButton;
    case foColumn: return mAllColButton;
    default: return nullptr;
    }
}

QSize MiniButton::sizeHint() const
{
    return iconSize() + QSize(2,3);
}

//void MiniButton::paintEvent(QPaintEvent *event)
//{
//    QAbstractButton::paintEvent(event);
//}

} // namespace studio
} // namespace gams
