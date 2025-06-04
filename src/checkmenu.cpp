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
#include "logger.h"
#include <QKeyEvent>

namespace gams {
namespace studio {


CheckMenu::CheckMenu(QWidget *parent) : QMenu(parent)
{
    mParentMenu = qobject_cast<CheckParentMenu*>(parent);
    if (!mParentMenu) DEB() << "CheckMenu Warning: Parent must be of class CheckParentMenu for proper function.";
}

void CheckMenu::mousePressEvent(QMouseEvent *event)
{
    if (mParentMenu && mParentMenu->contains(event->globalPosition())) {
        QPointF gloPos = event->globalPosition();
        QPointF parPos = mParentMenu->mapFromGlobal(gloPos);
        QMouseEvent *parEvent = new QMouseEvent(event->type(), parPos, gloPos, event->button(),
                                                event->buttons(), event->modifiers(), event->pointingDevice());
        mParentMenu->mousePressEvent(parEvent);
    } else if (mParentMenu && !geometry().contains(event->globalPosition().toPoint())) {
        mParentMenu->hide();
    } else {
        QMenu::mousePressEvent(event);
    }
}

void CheckMenu::mouseMoveEvent(QMouseEvent *event)
{
    if (mParentMenu && mParentMenu->contains(event->globalPosition())) {
        QPointF gloPos = event->globalPosition();
        QPointF parPos = mParentMenu->mapFromGlobal(gloPos);
        QMouseEvent *parEvent = new QMouseEvent(event->type(), parPos, gloPos, event->button(),
                                                event->buttons(), event->modifiers(), event->pointingDevice());
        mParentMenu->mouseMoveEvent(parEvent);
    } else {
        QMenu::mouseMoveEvent(event);
    }
}

void CheckMenu::mouseReleaseEvent(QMouseEvent *event)
{
    QAction *action = activeAction();
    if (action && action->isCheckable()) {
        action->setEnabled(false);
        QMenu::mouseReleaseEvent(event);
        action->setEnabled(true);
        action->trigger();
    } else if (mParentMenu && mParentMenu->contains(event->globalPosition())) {
        QPointF gloPos = event->globalPosition();
        QPointF parPos = mParentMenu->mapFromGlobal(gloPos);
        QMouseEvent *parEvent = new QMouseEvent(event->type(), parPos, gloPos, event->button(),
                                                event->buttons(), event->modifiers(), event->pointingDevice());
        mParentMenu->mouseReleaseEvent(parEvent);
        hide();
        mParentMenu->hide();
    } else {
        QMenu::mouseReleaseEvent(event);
    }

}

void CheckMenu::keyPressEvent(QKeyEvent *event)
{
    // The CheckMenu shown to the right of the linked action of the CheckParentMenu claims the keyboard focus.
    // To tell which menu should process the keyboard mParentFocus is introduced.

    if (!mParentMenu)
        QMenu::keyPressEvent(event);

    if (event->key() == Qt::Key_Escape) {
        mParentMenu->hide();
    } else {
        QAction *action = activeAction();
        QChar c = event->text().isNull() ? '\0' : event->text().at(0).toUpper();
        QList<int> keepOpenKeys;
        const auto acts = actions();
        for (QAction *act : acts) {
            QKeySequence sequence = QKeySequence::mnemonic(act->text());
            int key = sequence[0].toCombined() & 0xffff;
            if (act->isCheckable())
                keepOpenKeys << key;
            if (key == c.unicode()) {
                action = act;
                break;
            }
        }
        if (mParentFocus) {
            QList<int> keysForParent;
            keysForParent << Qt::Key_Up  << Qt::Key_Down;
            if (event->key() == Qt::Key_Right) {
                mParentFocus = false;
                if (!activeAction() && actions().size())
                    setActiveAction(actions().at(0));
            } else if (keysForParent.contains(event->key())) {
                mParentMenu->keyPressEvent(event);
            } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
                if (mParentMenu->activeAction())
                    mParentMenu->activeAction()->trigger();
                mParentMenu->hide();
            } else if (keepOpenKeys.contains(event->key())) {
                action->trigger();
            }
        } else {
            keepOpenKeys << Qt::Key_Return << Qt::Key_Enter << Qt::Key_Space;
            if (event->key() == Qt::Key_Left) {
                mParentFocus = true;
                setActiveAction(nullptr);
            } else if (action && action->isCheckable() && keepOpenKeys.contains(event->key())) {
                action->trigger();
            } else {
                QMenu::keyPressEvent(event);
            }
        }
    }
}


// -------------------------------------------------------


CheckParentMenu::CheckParentMenu(QWidget *parent): QMenu(parent)
{
    connect(this, &CheckParentMenu::hovered, this, &CheckParentMenu::onHovered);
}

void CheckParentMenu::addCheckActions(int actionDataValue, QList<QAction*> checkActions)
{
    mCheckActions.insert(actionDataValue, checkActions);
}

void CheckParentMenu::hideEvent(QHideEvent *event)
{
    closeSub();
    QMenu::hideEvent(event);
}

void CheckParentMenu::mouseMoveEvent(QMouseEvent *event)
{
    if (mCheckMenu && !mCheckMenu->geometry().contains(event->globalPosition().toPoint())) {
        QAction *action = actionAt(event->position().toPoint());
        if (action != activeAction()) {
            closeSub();
            setActiveAction(action);
        }
    }
    QMenu::mouseMoveEvent(event);
}

void CheckParentMenu::onHovered(QAction *action)
{
    handleAction(action);
}

void CheckParentMenu::closeSub()
{
    if (mCheckMenu) {
        mCheckMenu->deleteLater();
        mCheckMenu = nullptr;
    }
}

void CheckParentMenu::handleAction(QAction *action)
{
    if (!isVisible()) return;
    CheckMenu *newSub = nullptr;
    if (action && action->data().isValid()) {
        if (mActionKind != action->data().toInt() && mCheckActions.contains(action->data().toInt())) {
            newSub = new CheckMenu(this);
            newSub->addActions(mCheckActions.value(action->data().toInt()));
        }
    }
    int kind = action ? action->data().toInt() : 0;
    if (mActionKind != kind) {
        closeSub();
        if (newSub) {
            QRect rect = actionGeometry(action);
            newSub->popup(pos() + rect.topRight());
        }
        mCheckMenu = newSub;
    }
    mActionKind = action ? action->data().toInt() : 0;
}

bool CheckParentMenu::contains(QPointF globalPos)
{
    return geometry().contains(globalPos.toPoint());
}


} // namespace studio
} // namespace gams
