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
#include "checkmenu.h"
#include <QKeyEvent>

namespace gams {
namespace studio {

CheckMenu::CheckMenu(QWidget *parent): QMenu(parent)
{}

CheckMenu::~CheckMenu()
{}

void CheckMenu::addSubMenu(int actionDataValue, CheckMenu *subMenu)
{
    mSubMenus.insert(actionDataValue, subMenu);
    subMenu->mParentMenu = this;
}

void CheckMenu::showEvent(QShowEvent *event)
{
    QMenu::showEvent(event);
    const auto acts = actions();
    for (QAction *act : acts) {
        if (act->shortcut().toString().endsWith("F9"))
            act->setShortcut(QKeySequence("F9,Shift+F9"));
        if (act->shortcut().toString().endsWith("F10"))
            act->setShortcut(QKeySequence("F10,Shift+F10"));
    }
}

void CheckMenu::hideEvent(QHideEvent *event)
{
    QMenu::hideEvent(event);
    const auto acts = actions();
    for (QAction *act : acts) {
        if (act->shortcut().toString().endsWith("F9"))
            act->setShortcut(QKeySequence("F9"));
        if (act->shortcut().toString().endsWith("F10"))
            act->setShortcut(QKeySequence("F10"));
    }
}

void CheckMenu::mousePressEvent(QMouseEvent *event)
{
    if (mParentMenu && mParentMenu->contains(event->globalPosition())) {
        QPointF gloPos = event->globalPosition();
        QPointF parPos = mParentMenu->mapFromGlobal(gloPos);
        QMouseEvent *parEvent =  new QMouseEvent(event->type(), parPos, gloPos, event->button(), event->buttons(),
                                                event->modifiers(), event->pointingDevice());
        mParentMenu->mousePressEvent(parEvent);
    } else {
        QMenu::mousePressEvent(event);
    }
}

void CheckMenu::mouseReleaseEvent(QMouseEvent *event) {
    QAction *action = activeAction();
    if (action && action->isCheckable()) {
        action->setEnabled(false);
        QMenu::mouseReleaseEvent(event);
        action->setEnabled(true);
        action->trigger();
    } else if (mParentMenu && mParentMenu->contains(event->globalPosition())) {
        QPointF gloPos = event->globalPosition();
        QPointF parPos = mParentMenu->mapFromGlobal(gloPos);
        QMouseEvent *parEvent =  new QMouseEvent(event->type(), parPos, gloPos, event->button(), event->buttons(),
                                                event->modifiers(), event->pointingDevice());
        mParentMenu->mouseReleaseEvent(parEvent);
        hide();
        mParentMenu->hide();
    } else {
        QMenu::mouseReleaseEvent(event);
    }
}

void CheckMenu::keyPressEvent(QKeyEvent *event) {
    QAction *action = activeAction();
    QChar c = event->text().isNull() ? '\0' : event->text().at(0).toUpper();
    QList<int> keepOpenKeys;
    const auto acts = actions();
    for (QAction *act : acts) {
        QKeySequence sequence = QKeySequence::mnemonic(act->text());
        int key = sequence[0].toCombined() & 0xffff; // suspicious
        if (act->isCheckable()) keepOpenKeys << key;
        if (key == c.unicode()) {
            action = act;
            break;
        }
    }
    keepOpenKeys << Qt::Key_Return << Qt::Key_Enter;
    if (action && action->isCheckable() && keepOpenKeys.contains(event->key()))
        action->trigger();
    else
        QMenu::keyPressEvent(event);
}

void CheckMenu::mouseMoveEvent(QMouseEvent *event)
{
    QAction *act = actionAt(event->position().toPoint());
    if (act) {
        handleAction(act);
    } else if (mVisibleSub) {
        if (!mVisibleSub->contains(event->globalPosition()))
            handleAction(act);
    }
    if (mParentMenu && mParentMenu->contains(event->globalPosition())) {
        QPointF gloPos = event->globalPosition();
        QPointF parPos = mParentMenu->mapFromGlobal(gloPos);
        QMouseEvent *parEvent =  new QMouseEvent(event->type(), parPos, gloPos, event->button(), event->buttons(),
                                                event->modifiers(), event->pointingDevice());
        mParentMenu->mouseMoveEvent(parEvent);
    }
    QMenu::mouseMoveEvent(event);
}

void CheckMenu::handleAction(QAction *action)
{
    CheckMenu *newSub = nullptr;
    if (action && action->data().isValid()) {
        CheckMenu *sub = mSubMenus.value(action->data().toInt());
        if (sub) newSub = sub;
    }
    if (newSub != mVisibleSub) {
        if (mVisibleSub) mVisibleSub->hide();
        if (newSub) {
            QRect rect = actionGeometry(action);
            newSub->popup(pos() + rect.topRight());
        }
        mVisibleSub = newSub;
    }
}

bool CheckMenu::contains(QPointF globalPos)
{
    return geometry().contains(globalPos.toPoint());
}



} // namespace studio
} // namespace gams
