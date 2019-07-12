/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include "textviewedit.h"
#include "search/searchdialog.h"
#include "locators/searchlocator.h"
#include "keys.h"
#include "logger.h"
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>

namespace gams {
namespace studio {

TextViewEdit::TextViewEdit(TextMapper &mapper, QWidget *parent)
    : CodeEdit(parent), mMapper(mapper), mSettings(SettingsLocator::settings())
{
    setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
    setAllowBlockEdit(false);
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    disconnect(&wordDelayTimer(), &QTimer::timeout, this, &CodeEdit::updateExtraSelections);
}

void TextViewEdit::protectWordUnderCursor(bool protect)
{
    mKeepWordUnderCursor = protect;
}

bool TextViewEdit::hasSelection() const
{
    return mMapper.hasSelection();
}

void TextViewEdit::copySelection()
{
    int selSize = mMapper.selectionSize();
    if (selSize < 0) {
        QMessageBox::information(this, "Large selection", "Your selection is too large for the clipboard");
        return;
    }
    if (selSize > 5*1024*1024) {
        QStringList list;
        list << "KB" << "MB" << "GB";
        QStringListIterator i(list);
        QString unit("bytes");
        while (selSize >= 1024.0 && i.hasNext()) {
            unit = i.next();
            selSize /= 1024.0;
        }
        QString text = QString("Your selection is very large (%1 %2). Do you want to proceed?").arg(selSize,'f',2).arg(unit);
        QMessageBox::StandardButton choice = QMessageBox::question(this, "Large selection", text);
        if (choice != QMessageBox::Yes) return;
    }
    mMapper.copyToClipboard();
}

void TextViewEdit::selectAllText()
{
    mMapper.selectAll();
    emit updatePosAndAnchor();
}

void TextViewEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown
            || event->key() == Qt::Key_Up || event->key() == Qt::Key_Down
            || event->key() == Qt::Key_Home || event->key() == Qt::Key_End ) {
        emit keyPressed(event);
        if (!event->isAccepted())
            CodeEdit::keyPressEvent(event);
    } else if (event->key() == Qt::Key_A && event->modifiers().testFlag(Qt::ControlModifier)) {
        selectAllText();
    } else if (event->key() == Qt::Key_C && event->modifiers().testFlag(Qt::ControlModifier)) {
        copySelection();
    } else {
        CodeEdit::keyPressEvent(event);
    }
}

void TextViewEdit::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *menu = createStandardContextMenu();
    QAction *lastAct = nullptr;
    for (int i = menu->actions().count()-1; i >= 0; --i) {
        QAction *act = menu->actions().at(i);
        if (act->objectName() == "select-all") {
            if (blockEdit()) act->setEnabled(false);
            menu->removeAction(act);
            act->disconnect();
            connect(act, &QAction::triggered, this, &CodeEdit::selectAllText);
            menu->insertAction(lastAct, act);
        } else if (act->objectName() == "edit-paste" && act->isEnabled()) {
            menu->removeAction(act);
            act->disconnect();
        } else if (act->objectName() == "edit-cut") {
            menu->removeAction(act);
            act->disconnect();
        } else if (act->objectName() == "edit-copy") {
            menu->removeAction(act);
            act->disconnect();
            act->setEnabled(mMapper.hasSelection());
            connect(act, &QAction::triggered, this, &TextViewEdit::copySelection);
            menu->insertAction(lastAct, act);
        } else if (act->objectName() == "edit-delete") {
            menu->removeAction(act);
            act->disconnect();
        }
        lastAct = act;
    }
    menu->exec(e->globalPos());
    delete menu;
}

void TextViewEdit::recalcWordUnderCursor()
{
    if (!mKeepWordUnderCursor) {
        CodeEdit::recalcWordUnderCursor();
    }
}

int TextViewEdit::effectiveBlockNr(const int &localBlockNr) const
{
    int res = mMapper.absTopLine();
    if (res < 0) {
        res -= localBlockNr;
    } else {
        res += localBlockNr;
    }
    return res;
}

void TextViewEdit::extraSelCurrentLine(QList<QTextEdit::ExtraSelection> &selections)
{
    if (!mSettings->highlightCurrentLine()) return;

    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(mSettings->colorScheme().value("Edit.currentLineBg", QColor(255, 250, 170)));
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = QTextCursor(document()->findBlockByNumber(mMapper.position(true).y()));
    selections.append(selection);
}

int TextViewEdit::topVisibleLine()
{
    return mMapper.absTopLine() + mMapper.visibleOffset();
}

} // namespace studio
} // namespace gams
